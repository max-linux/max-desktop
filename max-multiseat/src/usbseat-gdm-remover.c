/*
* usbseat-gdm-remover.c poll device and call gdmdynamic -d
* Copyright (C) 2011  mariodebian at gmail
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

/*

  Usage:
     usbseat-gdm-remover /dev/usbseat/10/sound 10

*/
#include <stdio.h>
#include <string.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>


#define MSG_BUFF 4096
#define POLL_TIMEOUT 2*1000



int snprintf(char *str, size_t size, const char *format, ...);
FILE *popen(const char *cmd, const char *type);
int pclose(FILE *fp);
int nanosleep(const struct timespec *req, struct timespec *rem);


int file_exists (char * fileName) {
    struct stat buf;
    int i = stat ( fileName, &buf );
    /* File found */
    if ( i == 0 )
        return 1;
    
    return 0;
}


void msleep(int ms) {
    struct timespec time;
    time.tv_sec = ms / 1000;
    time.tv_nsec = (ms % 1000) * (1000 * 1000);
    nanosleep(&time,NULL);
}


int main (int argc, char *argv[]) {
    int fd_file;
    struct pollfd fdarray;
    int nfds, rc;
    char cmd[MSG_BUFF];
    FILE *fp;

    if (argc != 3) {
        printf("usbseat-gdm-remover, bad arguments\n");
        return -1;
    }

    if ((fd_file = open(argv[1], O_RDONLY, 0)) < 0) {
        /*perror("Error opening file");*/
        return -1;
    }

    snprintf( (char*) &cmd, MSG_BUFF, "gdmdynamic -d %s", argv[2]);

    for (;;) {
        fdarray.fd = fd_file;
        fdarray.events = POLLIN | POLLERR;
        nfds = 1;

        rc = poll(&fdarray, 1, POLL_TIMEOUT);

        if (rc < 0) {
            perror("error reading poll() \n");
            return -1;
        }

        else if(rc > 0) {
            /*  printf("  DEBUG: Changes %s rc=%d revents=%d\n", argv[1], rc, fdarray.revents);*/
            /* ugly hack to not eat all CPU when poll() return inmediatly */
            msleep(100);
            
            if ( file_exists(argv[1] ) ){
                printf("      sleeping 2 seconds \n");
                msleep(2000);
            }
            else {
                printf("usbseat-gdm-remover, %s disconnected, exec => %s\n",argv[1], cmd);
                fp=popen(cmd, "r");
                pclose(fp);
                return 0;
            }

        }
    }
    /* never here */
    return 0;
}



