/*
*  CALL multiseat-udisks to umount a MULTISEAT storage device
*               (this app have bit SUID)
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


#define BSIZE 512
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
  /*printf("%s\n", cmd);*/

  if ( setuid(0) ) {
    printf("No puedo convertirme en root\n");
    return -1;
  }

  fp=(FILE*)popen( cmd , "r");
  if (fp == NULL) {
    return 1;
  }
  fret = fgets( line, sizeof line, fp);
  remove_line_break(line);
  pclose(fp);

  /*fprintf(stderr,"line=%s\n",line);*/
  printf("%s\n",line);
  return 0;
}
