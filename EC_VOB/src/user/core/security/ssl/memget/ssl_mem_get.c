#include "ssl_mem_get.h"

#define NULL    0

struct ptr_st
{
	void *ptr;
	int p_seek;
};

static struct ptr_st ptr_arr[10];



char *mem_fgets(char *buf ,int size ,void *ptr)
{
	int pseek;
	char *a,*b;
	int i,n;



/*	static struct ptr_st ptr_arr[10];*/

	a=buf;
	n=size-1;
	b=(char *)ptr;
	pseek = get_pseek(ptr);
	b=b+pseek;
	
	
	if (*b == '\0')
	{
/*isiah.2003-05-06*/
	    return NULL;
#if 0 
		*a=*b;
//		pseek=0;
		set_pseek(ptr,pseek);
/*		show_pseek(ptr_arr);*/
		return(a);
#endif
	}
	for (i=0;i<n;i++)
	{
		if ( *b == '\0' )
		{
			*a = '\0';
//			pseek=0;
			break;
		}
		else if ( *b == '\n' )
		{
			/**(a++) = *(b++);*/
			*(a++) = 13;
			*(a++) = 10;
			/**a = '\0';*/
			pseek+=1;
			break;
		}
		else
		{
			*(a++) = *(b++);
			pseek+=1;
		}
	}
	*a = '\0';
	set_pseek(ptr,pseek);
/*	show_pseek(ptr_arr);*/
	return(buf);
}



int mem_fclose(void *ptr)
{
	return(clear_pseek(ptr));
}



int get_pseek(void *p)
{

	int i;


	for ( i=0;i<10;i++)
	{
		if (ptr_arr[i].ptr == p)
		{
			return(ptr_arr[i].p_seek);
		}
		
	}
	for ( i=0;i<10;i++)
	{
		if ( ptr_arr[i].ptr == NULL )
		{
			ptr_arr[i].ptr = p;
			ptr_arr[i].p_seek = 0;
			return(ptr_arr[i].p_seek);
		}
	}
	return(-1);
}



int set_pseek(void *p, int p_seek)
{
	int i;

	for ( i=0;i<10;i++)
	{
		if ( ptr_arr[i].ptr == p )
		{
			ptr_arr[i].p_seek = p_seek;
			return(1);
		}
	}
	return(0);
}



int clear_pseek(void *p)
{
	int i;

	for ( i=0;i<10;i++)
	{
		if ( ptr_arr[i].ptr == p )
		{
			ptr_arr[i].p_seek = 0;
			ptr_arr[i].ptr = NULL;
			return(1);
		}
	}
	return(0);
}


int mem_fwrite(char *ptr ,int size ,void *buf)
{
	int pseek;
	char *a,*b;
	int i,n;



/*	static struct ptr_st ptr_arr[10];*/

	a=ptr;
	n=size;
	b=(char *)buf;
	pseek = get_pseek(buf);
	b=b+pseek;
	
	
	if (*a == '\0')
	{
		*b=*a;
		pseek=0;
		set_pseek(buf,pseek);
/*		show_pseek(ptr_arr);*/
		return(b);
	}
	for (i=0;i<n;i++)
	{
		if ( *a == '\0' )
		{
			*b = '\0';
			pseek=0;
			break;
		}
//		else if ( *a == '\n' )
//		{
			/**(a++) = *(b++);*/
//			*(b++) = 13;
//			*(b++) = 10;
//			/**a = '\0';*/
//			pseek+=2;
//			break;
//		}
		else
		{
			*(b++) = *(a++);
			pseek+=1;
		}
	}
	*b = '\0';
	set_pseek(buf,pseek);
/*	show_pseek(ptr_arr);*/
	return(i);
}
