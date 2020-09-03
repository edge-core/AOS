#ifndef SSL_MEM_GET_H
#define SSL_MEM_GET_H



/*#include <stdio.h>*/
/*#include <string.h>*/



char *mem_fgets(char * ,int  ,void *);
int mem_fclose(void *);
int get_pseek(void *);
int set_pseek(void *, int);
int clear_pseek(void *);
int mem_fwrite(char * ,int ,void *);



#endif /*SSL_MEM_GET_H */
