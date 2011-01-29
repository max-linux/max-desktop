/*
*  CALL multiseat-udisks to umount a MULTISEAT storage device
*               (this app have bit SUID)
* Copyright 2011, Mario Izquierdo, mariodebian at gmail
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

#include <stdlib.h>

int snprintf(char *str, size_t size, const char *format, ...);
char *fgets(char *s, int tam, FILE *flujo);
FILE *popen(const char *orden, const char *tipo);
int pclose(FILE *flujo);


#define BSIZE 1024
#define APP "/usr/sbin/multiseat-udisks"

/* replace \n in command output */
void remove_line_break( char *s ) {
    s[strcspn ( s, "\n" )] = '\0';
}


int main (int argc, char **argv) {
  uid_t my_uid=0;
  uid_t my_euid=0;
  FILE *fp;
  char cmd[BSIZE]="";
  char line[BSIZE]="";
  char *fret;

  if (argc != 2) {
    return 1;
  }

  my_uid=getuid();
  my_euid=geteuid();

  /*
  printf("uid=%d\n", my_uid );
  printf("euid=%d\n",my_euid);
  */

  snprintf( (char*) cmd, BSIZE, "%s %s %d 2>&1", APP, argv[1], my_uid);

  if ( setuid(0) ) {
    printf("/sbin/umount.multiseat: No se pudieron elevar los privilegios.\n");
    return 1;
  }

  fp=(FILE*)popen( cmd , "r");
  if (fp == NULL) {
    printf("/sbin/umount.multiseat: No se pudo ejecutar la aplicación esclava.\n");
    return 1;
  }

  fret = fgets( line, sizeof line, fp);
  remove_line_break(line);
  pclose(fp);

  printf("%s\n",line);
  return 0;
}
