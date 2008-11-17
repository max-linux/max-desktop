/*
 * (c) Pedro Peña Pérez <pedro.pena@edmanufacturer.es>, 
 * Copyright: GNU GPL v2
 * Soporte para versiones 5.3 y 5.5b, 4/07
 * Añadido soporte para versión 1.68, 15/06/07
 * Comprobado soporte para versiones 5.7 y 7.2, 05/07/07
 */

#define _FILE_OFFSET_BITS 64
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>

#define SECTOR_SIZE 512
#define METADATA_SECTORS 12
#define H 255
#define S 63
#define STI_V1_INIT_SECTOR -14079
#define STI_V5_INIT_SECTOR 3335
#define STI_V5_OFFSET -16065
#define STI_V1_LEN 40
#define STI_V5_LEN 32
#define meg_to_cil(m) ((((unsigned long long)m)<<20)/(H*S*SECTOR_SIZE)+1)
#define cil_to_meg(c) (((H*S*SECTOR_SIZE*(unsigned long long)c)>>20))
#define cil_to_sec(c) ((unsigned long long)c*H*S)

static void fatal(char *s, int ret) {
	fprintf(stderr, "%s: %s\n", "ERROR", s);
	exit(ret);
}

struct sti_v1_ {
	unsigned char padding1[28];
	unsigned char name[10];
	unsigned char padding2[63];
	unsigned char sti_type;
	unsigned char type;
	unsigned char backup;
	short unsigned start;
	short unsigned size;
	short unsigned bstart;
	short unsigned bsize;
	unsigned char padding3[16];
};

struct sti_v5_ {
	unsigned char pos;
	unsigned char sti_type;
	unsigned char name[11];
	unsigned char type;
	short unsigned size;
	short unsigned bsize;
	unsigned char backup;
};
#define STI_V5_STRUCT_TAM 19

int main(int argc, char **argv) {
	int fd,version=0;
	unsigned i,j,k;
	unsigned long long dev_size,offset=0,first,size,bsize,sec=0;
	unsigned char buffer[SECTOR_SIZE*METADATA_SECTORS];
	struct sti_v5_ sti_v5[STI_V5_LEN];
	struct sti_v1_ sti_v1[STI_V1_LEN];

	if ((fd = open(argv[1], O_RDONLY)) < 0)
              fatal("Cannot open disk drive", 2);
	
	dev_size = lseek(fd, 0, SEEK_END)>>9;
	if (dev_size < 0)
		fatal("Cannot get disk size", 2);

	lseek(fd, 8, SEEK_SET);
	if (read(fd, (void *)&offset, 4) != 4)
		fatal("Cannot read disk drive", 2);
	
	if (offset < dev_size)
	{
		lseek(fd, (unsigned long long)(STI_V5_INIT_SECTOR + offset ) * SECTOR_SIZE, SEEK_SET);
		if (read(fd, buffer, SECTOR_SIZE * METADATA_SECTORS) == SECTOR_SIZE * METADATA_SECTORS)
			if (!memcmp(buffer, "K-10", 4))
				version=5;
	}

	if ( !version )
	{
                for ( i = 1; i <= 0x28000; i++ ) {
                        lseek(fd, (unsigned long long)(-METADATA_SECTORS * SECTOR_SIZE * (unsigned long long)i * 2), SEEK_END);
			if (read(fd, buffer, SECTOR_SIZE * METADATA_SECTORS) != SECTOR_SIZE * METADATA_SECTORS)
				fatal("Cannot read disk drive", 2);
			for ( j = METADATA_SECTORS; j > 0; j-- ) {
				if ( buffer[ j*SECTOR_SIZE - 2 ] == 0x55 && buffer[ j*SECTOR_SIZE - 1 ] == 0xaa ) {
					lseek(fd, -(METADATA_SECTORS - (unsigned long long)j) * SECTOR_SIZE, SEEK_CUR);
					lseek(fd, STI_V1_INIT_SECTOR * SECTOR_SIZE, SEEK_CUR);
					if (read(fd, buffer, SECTOR_SIZE * METADATA_SECTORS) != SECTOR_SIZE * METADATA_SECTORS)
						fatal("Cannot read disk drive", 2);
					for ( k = 0; k < SECTOR_SIZE * METADATA_SECTORS; k++ )
						buffer[k] ^= 'U';
					if (memcmp(buffer+4, "X-PARA10", 8)) {
						lseek(fd, (-STI_V1_INIT_SECTOR-METADATA_SECTORS) * SECTOR_SIZE, SEEK_CUR);
					}
					else {
						version=1;
						goto found;
					}
				}
			}
		}
	}

	found:
	if ( !version )
		fatal("Not STI data", 2);
		
	close(fd);
	
	printf("unit: sectors\n\n");

	switch (version) {
		case 1:
			for (i = 0; i < STI_V1_LEN; i++)
			{
				memcpy(&sti_v1[i], (void *)buffer+i*sizeof(struct sti_v1_)+0x2e, sizeof(struct sti_v1_));
				if ( !sti_v1[i].size )
					break;
		 		fprintf(stderr,"%2u T%x %-10s %2x %6llu %6llu\n", i+1, sti_v1[i].sti_type, sti_v1[i].name, sti_v1[i].type, cil_to_meg(sti_v1[i].size), cil_to_meg(sti_v1[i].bsize));
				printf("%2u : start=%10llu, size=%10llu, Id=%2x\n",i+1,cil_to_sec(sti_v1[i].start)+63,cil_to_sec(sti_v1[i].size)-63,sti_v1[i].type);
			}
		break;
		case 5:
			for (i=0; i<STI_V5_LEN; i++)
				memcpy(&sti_v5[i], (void *)buffer+i*STI_V5_STRUCT_TAM+0x6e, STI_V5_STRUCT_TAM);
			
			first=63;
			for (i=0; i<STI_V5_LEN; i++) {
				if ( !sti_v5[i].size )
					break;
				k=0;
				j=0;
				while (cil_to_meg(meg_to_cil(sti_v5[i].size+j)) != sti_v5[i].size + j + k ) {
					sec=cil_to_sec( meg_to_cil(sti_v5[i].size+j) );
					j+=0x10000;
					if ( (first + sec) > offset) {//Se asume que la última partición será la única que podrá superar el tamaño 0x10000. Se asume que los metadatos estarán después de la última partición.
						k++;
						j=0;
					}
				}
				size=meg_to_cil(sti_v5[i].size+j)*H*S;
				fprintf(stderr,"%2u T%x %-11s %2x %6u %6u %2x\n",sti_v5[i].pos,sti_v5[i].sti_type,sti_v5[i].name,sti_v5[i].type,(unsigned)sti_v5[i].size+j,sti_v5[i].bsize,sti_v5[i].backup);
				bsize=0;
				if ( sti_v5[i].sti_type == 2 )
					bsize=size;
				else if ( sti_v5[i].sti_type == 1 ) {
					fprintf(stderr,"T1 Backup %s%u begin: %llu\n",argv[1],sti_v5[i].pos,first);
					bsize=meg_to_cil(sti_v5[i].bsize)*H*S;
					fprintf(stderr,"T1 Backup %s%u size: %llu\n",argv[1],sti_v5[i].pos,bsize);
				}
				first+=bsize;
				printf("%2u : start=%10llu, size=%10llu, Id=%2x\n",sti_v5[i].pos,first,size-63,sti_v5[i].type);
				first+=size;
			}
		break;
	}
	return 0;
}
