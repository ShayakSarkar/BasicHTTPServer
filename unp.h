#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>

#define SERV_PORT1 9877
#define BUFF_LEN 1000
#define SERV_PORT2 8877

void Listen(int,int);
ssize_t readn(int,void *,size_t);
ssize_t writen(int,void *,size_t);


static int read_cnt;
static char *read_ptr;
static char read_buf[BUFF_LEN];
static ssize_t my_read(int fd,char *ptr){
	if(read_cnt<=0){
		again:
			if((read_cnt=read(fd,read_buf,sizeof(read_buf)))<0){
				if(errno==EINTR){
					goto again;
				}
				return -1;
			}
			else if(read_cnt==0){
				return 0;
			}
			read_ptr=read_buf;
	}
	read_cnt--;
	*ptr=*read_ptr++;
	return 1;
}

ssize_t readline(int fd,void *vptr,size_t maxlen){
	ssize_t n,rc;
	char c,*ptr;

	ptr=(char*)vptr;
	for(n=1;n<maxlen;n++){
		if((rc=my_read(fd,&c))==1){
			*ptr++=c;
			if(c=='\n'){
				break;
			}
		}
		else if(rc==0){
			*ptr=0;
			return n-1;
		}
		else{
			return -1;
		}
		
	}
	*ptr=0;
	return n;
}

ssize_t readlinebuf(void **vptrptr){
	if(read_cnt){
		*vptrptr=read_ptr;
	}
	return read_cnt;
}

ssize_t writen(int fd,void *vptr,size_t n){
	size_t nleft;
	ssize_t nwritten;	
	const char *ptr;
	ptr=(const char *)vptr;
	nleft=n;
	while(nleft>0){
		if((nwritten=write(fd,ptr,nleft))<=0){
			if(nwritten<0 && errno==EINTR){
				nwritten=0;
			}
			else{
				return -1;
			}
		}
		nleft-=nwritten;
		ptr+=nwritten;
	}
	return n;	
}
ssize_t readn(int fd,void *vptr,size_t n){
	size_t nleft;
	size_t nread;
	char *ptr;

	ptr=(char *)vptr;
	nleft=n;
	while(nleft>0){
		if((nread=read(fd,ptr,nleft))<0){
			if(errno==EINTR){
				nread=0;
			}
			else{
				return -1;
			}
		}
		else if(nread==0){
			break;
		}
		nleft-=nread;
		ptr+=nread;
	}
	return n-nleft;
}

void Listen(int fd,int backlog){
	char* ptr;
	/* can override 2nd argument with environment variable*/

	if((ptr=getenv("LISTENQ"))!=NULL){
		backlog=atoi(ptr);
	}
	if(listen(fd,backlog)<0){
		printf("listen error");
	}
}

