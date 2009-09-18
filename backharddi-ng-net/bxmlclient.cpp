/***************************************************************************
 *   Copyright (C) 2007 by Oscar Campos Ruiz-Adame                         *
 *   oscar.campos@edmanufacturer.es                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "XmlRpc.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>

#define BXMLCLIENT_VERSION 	0.1

using namespace XmlRpc;

void print_help(const char *appname);
XmlRpcValue parse_params(char *str);
void parse_params_from_file(char *fileName, XmlRpcClient *c);
std::string parseRequest(std::string const& xml, XmlRpcValue& params);

int main(int argc, char* argv[])
{	
	if(argc < 4) {
		print_help(argv[0]);
		return -1;
  	}
	
	char *host = argv[1];
	int port = atoi(argv[2]);	
	char *method = argv[3];
	XmlRpcValue params, result;	
	
	// Use introspection API to look up the supported methods
	XmlRpcClient c(host, port, "/", false);
	
	// Parse Method for special functions
	if(!strcmp(method, "-f") || !strcmp(method, "--file")) {
		parse_params_from_file(argv[4], &c);
		return 0;	
	}
	else {
		params = parse_params(argv[4]);
	}	
	
	//XmlRpc::setVerbosity(5);
  	
	// Check for available methods
	if(!strcmp(method, "listmethods")) {
		XmlRpcValue noArgs;
		if(c.execute("system.listMethods", noArgs, result)) {
			std::cout << "\nAvailable Methods:\n" << result << "\n\n";
		}
		else {
			std::cerr << "The server does not support methods listing.\n\n";
		}
		
		return 0;
	}
	
  	// Execute the user query	
	if(c.execute(method, params, result)) 
		std::cout << result << "\n\n";
  	else
	{
		std::cerr << "Error Calling " << method << "\n\n";	
		return 1;
	}

  	return 0;
}

void print_help(const char *appname)
{
	std::cerr << "Backharddi XML-RPC command line client v" << BXMLCLIENT_VERSION << "\n";
	std::cerr << "(C) Copyright 2008 Oscar Campos <oscar.campos@edmanufacturer.es>\n";
	std::cerr << "\n";
	std::cerr << "Syntax: " << appname << " <host> <port> <method> <params>\n";
	std::cerr << "Example: " << appname << " localhost 8080 sum \"i 10,i 20,i 45\"\n";	
	std::cerr << "Special methods:\n";
	std::cerr << appname << " <host> <port> listmethods for a list of available methods (maybe the server does not support this).\n";
	std::cerr << appname << " <host> <port> -f | --file <path to valid XML File> get method and params from an existent and valid XML File.\n\n";
	std::cerr << "For bug reports send backtraces to <oscar.campos@edmanufacturer.es>\n";
}

XmlRpcValue parse_params(char *str)
{
	int i = 0;
	char *pch;
	XmlRpcValue params;	
	std::vector<char *> strParams;
		
	pch = strtok(str, ",");		
	while(pch != NULL) {	
		strParams.push_back((char*)pch);
		i++;
		pch = strtok(NULL, ",");
	}	
	
	char *tok;
	int index = 0;
	for(std::vector<char*>::iterator it = strParams.begin(); it != strParams.end();it++) {
		tok = strtok(*it, " ");
		if(!strcmp(tok, "i")) {
			tok = strtok(NULL, " ");
			params[index] = atoi(tok);
		}		
		else if(!strcmp(tok, "d")) {
			tok = strtok(NULL, " ");
			params[index] = atof(tok);
		}
		else {
			tok = strtok(NULL, " ");
			params[index] = tok;
		}
					
		index++;
	}
	
	std::cerr << "Params: " << params << "\n";
		
	return params;
}

void parse_params_from_file(char *fileName, XmlRpcClient *c)
{
	std::ifstream infile(fileName);
	if (infile.fail()) {
		std::cerr << "Could not open file '" << fileName << "'.\n";
		return;
	}

  	infile.seekg(0L, std::ios::end);
	long nb = infile.tellg();
	infile.clear();
	infile.seekg(0L);
	char* b = new char[nb+1];
	infile.read(b, nb);
	b[nb] = 0;

	std::cerr << "Reading file...\n";
  
	std::string s(b);
	XmlRpcValue params;
	std::string name = parseRequest(s, params);

	if (name.empty()) {
		std::cerr << "Could not parse file %s!\nCheck the file to be sure it's a valid XML file.";
		return;
	}

	XmlRpcValue result;
	std::cerr << "Calling " << name << std::endl;
	if (c->execute(name.c_str(), params, result))
		std::cout << result << "\n\n";
	else
		std::cerr << "Error calling '" << name << "'\n\n";
}

std::string parseRequest(std::string const& xml, XmlRpcValue& params)
{
	const char METHODNAME_TAG[] = "<methodName>";
	const char PARAMS_TAG[] = "<params>";
	const char PARAMS_ETAG[] = "</params>";
	const char PARAM_TAG[] = "<param>";
	const char PARAM_ETAG[] = "</param>";

	int offset = 0;   // Number of chars parsed from the request

	std::string methodName = XmlRpcUtil::parseTag(METHODNAME_TAG, xml, &offset);
	XmlRpcUtil::log(3, "XmlRpcServerConnection::parseRequest: parsed methodName %s.", methodName.c_str()); 

	if (! methodName.empty() && XmlRpcUtil::findTag(PARAMS_TAG, xml, &offset))
	{
		int nArgs = 0;
		while (XmlRpcUtil::nextTagIs(PARAM_TAG, xml, &offset)) {
			std::cerr << "Parsing arg " << nArgs+1 << std::endl;
			XmlRpcValue arg(xml, &offset);
			if ( ! arg.valid()) {
				std::cerr << "Invalid argument\n";
				return std::string();
			}
			std::cerr << "Adding arg " << nArgs+1 << " to params array." << std::endl;
			params[nArgs++] = arg;
			(void) XmlRpcUtil::nextTagIs(PARAM_ETAG, xml, &offset);
		}

		XmlRpcUtil::log(3, "XmlRpcServerConnection::parseRequest: parsed %d params.", nArgs); 

		(void) XmlRpcUtil::nextTagIs(PARAMS_ETAG, xml, &offset);
	}

	return methodName;
}
