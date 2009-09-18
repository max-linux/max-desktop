#!/usr/bin/python

import socket
import sys
import os
import posix
import threading
import datetime
import stat
import os.path
import glob
import Queue
import subprocess

LOGFILE = '/var/log/backharddi-ng-server.log'
PORT = 4700 
ROOT = '/tmp/backharddi-ng/'
UDPSENDER = '/usr/bin/udp-sender'
BUFFERS = 16
BUFFER = 256*1024
STEP = 10
MINCLIENTS = 15

class log:
	def __init__( self, f ):
		self.f = f
	def write( self, s ):
		self.f.write( s )
		self.f.flush()

class reader( threading.Thread ):
	def __init__( self, writer ):
		threading.Thread.__init__( self )
		self.writer = writer
	def run( self ):
		for file in self.writer.input:
			self.input = open( file )
			try:
				r = self.input.read( BUFFER )
			except:
				self.input.close()
				return
			while r:
				self.writer.server.queue.put( r )
				try:
					r = self.input.read( BUFFER )
				except:
					self.input.close()
					return
			self.input.close()
		self.writer.server.queue.put( False )
	def stop( self ):
		self.input.close()
		self.writer.server.queue.get()
				
class progress_pipe( threading.Thread ):
	def __init__( self, server ):
		threading.Thread.__init__( self )
		self.server = server
		self.output = server.input
		self.input = glob.glob( server.filename + ".??" )
		self.input.sort()
		self.inputsize = sum( os.stat( file ).st_size for file in self.input )
		self.reader = reader( self )
		self.reader.start()
	def run( self ):	
		b = 0
		p_old = 0
		self.server.transfer.wait()	
		while self.server.participants:
			r = self.server.queue.get()
			if r:
				try:
					self.output.write( r )
				except:
					self.reader.stop()
					self.output.close()
					return
					
				b += len( r )
				p = float( b ) / float( self.inputsize ) * 100
				if p - p_old >= STEP:
					self.server.log( '%d%% Completado' % p )
					p_old = int( p )
			else:					
				break							

		self.output.close()
	def stop( self ): 
		self.output.close()
		self.server.transfer.set()

class backharddi_server( threading.Thread ):
	port = 7000
	active = {} #Mapa de listas de hilos backharddi_server. La llave del mapa es el nombre de fichero.
	lock = threading.Lock()
	def __init__( self, clientsock, serveraddress ):
		threading.Thread.__init__( self )
		self.sock = clientsock
		self.address = serveraddress
		self.msg = self.sock.recv( 1024 ).split()
		self.serving = 0 
		self.transfer = threading.Event() 
		self.requests = 0
		self.participants = 0
		self.minclients = MINCLIENTS
		self.maxwait = 0
		self.myport = backharddi_server.port
		self.imgs = []
	def waiting( self ):
		if self.master in backharddi_server.active:
			for s in backharddi_server.active[self.master]:
				if s.address == self.address and not s.serving:
					return s
	def set_active( self ):
		if self.master in backharddi_server.active:
			backharddi_server.active[self.master].append( self )
		else:
			backharddi_server.active[self.master] = [ self ]
	def unset_active( self ):
		backharddi_server.active[self.master].remove( self )
	def log( self, msg ):
		print '%s %-25s: file[%s] puerto[%d] interfaz[%s] peticiones[%d] clientesmin[%d] participantes[%d]\n' % ( datetime.datetime.now(), msg, ROOT + self.master, self.myport, str( self.address ), self.requests, int( self.minclients ), self.participants )
	def monitor( self, output ):
		while 1:
			line = output.readline()
			if not line:
				return
			if line.find( "Starting transfer" ) >= 0:
				self.serving = 1
				self.transfer.set()
				self.log( 'Iniciando transferencia' )
			elif line.find( "Transfer complete." ) >= 0:
				self.minclients = self.participants
				self.finish = 1
				self.transfer.clear()
				self.log( 'Transferencia finalizada' )
			elif line.find( "New connection" ) >= 0:
				self.participants += 1
				self.log( 'Nueva conexion' )
			elif line.find( "Dropping client" ) >= 0:
				self.log( 'Tiempo de espera agotado' )
				self.minclients -= 1
			elif line.find( "Disconnecting" ) >= 0:
				self.participants -= 1
				self.log( 'Desconexion' )
	def run( self ):
		self.command = self.msg[0]
		if self.command == "GET":
			self.master = self.msg[1]
			if len(self.msg) == 3:
				self.minclients = self.msg[2]
			if not os.path.exists( ROOT + self.master):
				self.sock.send( "master_not_exists" )
				self.sock.close()
				return
			backharddi_server.lock.acquire()
			parent_server = self.waiting()
			if parent_server: 
				self.sock.send( str( parent_server.myport ) )
				parent_server.requests += 1
				backharddi_server.lock.release()
				parent_server.log( 'Nueva peticion' )
				self.sock.close()
			else:
				active_ports = [ server.myport for file in backharddi_server.active.itervalues() for server in file ]
				while self.myport in active_ports: 
					self.myport += 2
				self.set_active()
				self.requests += 1
				backharddi_server.lock.release()
				
				for root, dirs, files in os.walk( ROOT + self.master ):
					if 'img.00' in files and 'detected_filesystem' in files:
						if not 'linux-swap' in open( root + '/detected_filesystem' ).read():
							self.imgs.append( root + '/img' )
				self.sock.send( str( self.myport ) )
				self.sock.close()
				
				self.imgs.sort()	
				for self.filename in self.imgs:
					print self.filename
					self.finish = 0
					p = subprocess.Popen( ( UDPSENDER, "--nokbd", "--full-duplex", "--retriesUntilDrop", "50", "--interface", str( self.address ), "--portbase", str( self.myport ), "--min-clients", str( self.minclients ), "--max-wait", str( self.maxwait ) ), stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True )
					self.input, stdout, output = p.stdin, p.stdout, p.stderr
					self.maxwait = 240
					self.log( 'UDP-Sender levantado' )
					self.queue = Queue.Queue( BUFFERS )
					self.pipe = progress_pipe( self )
					self.pipe.start()
					self.monitor( output )
					stdout.close()
					output.close()
					os.wait()
					self.input.close()

					if not self.finish:
						self.log( 'Transferencia abortada' )
						break
					
				backharddi_server.lock.acquire()
				self.unset_active()
				backharddi_server.lock.release()
		elif self.command == "Completado" or self.command == "Error":
			out = os.popen( "grep " + self.sock.getpeername()[0] + " /proc/net/arp | cut -c 42-58" )
			mac = out.read(17)
			out.close()
			self.sock.close()
			fifofile = ROOT + "01-" + mac.replace( ":", "-" ).lower() + ".fifo"
			if os.path.exists( fifofile ):
				f = open( fifofile, 'w' )
				f.write( self.command )
				f.close
		else:
			self.sock.send( "unrecognized_command" )
			self.sock.close()

if __name__ == '__main__':
	if len( sys.argv ) > 1 and ( sys.argv[1] == '--daemon' or sys.argv[1] == '-d' ):
		#if ( os.fork() ): sys.exit()

		posix.close( sys.stdin.fileno() )
		sys.stdin = open( '/dev/null' )

		posix.close( sys.stdout.fileno() )
		sys.stdout = log( open( LOGFILE, 'w' ) )
		
		posix.close( sys.stderr.fileno() )
		sys.stderr = log( open( LOGFILE, 'w' ) )
	
	serversock = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
	serversock.setsockopt( socket.SOL_SOCKET, socket.SO_REUSEADDR, 1 )
	serversock.bind( ('', PORT) )
	serversock.listen( 5 )
	
	print '%s Backharddi NG Server arrancado... pid %d' % ( datetime.datetime.now(), os.getpid() )
	while 1:
		try:
			( clientsock, address ) = serversock.accept()
		except:
			for master in backharddi_server.active:
				for thread in backharddi_server.active[master]:
					thread.sock.close()
					try:
						thread.pipe.reader.stop()
						thread.pipe.stop()
					except:
						continue
			break
		serveraddress= clientsock.getsockname()[0]
		s = backharddi_server( clientsock, serveraddress )
		s.start()
