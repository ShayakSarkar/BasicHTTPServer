#include "unp.h"
#define REQ_LEN 1000
#define RES_LEN 100
#define SEVER_PORT 55555

long unsigned int get_length(FILE* fstr){
    fseek(fstr,0,SEEK_END);
    long unsigned int len=ftell(fstr);
    fseek(fstr,0,0);
    return len;
}

char* get_file_path(char* request){
    int index=4;
    char* path=(char*)malloc(sizeof(char)*100);
    path[0]='.';
    int ctr=1;
    for(;request[index]!=' ';index++){
        path[ctr++]=request[index];
    }
    printf("Path extracted: %s",path);
    return path;
}
void handle_client(int sockfd){
    char* request=(char*)malloc(REQ_LEN);  
    char* response=(char*)malloc(RES_LEN);
    bzero(response,RES_LEN);
    read(sockfd,request,REQ_LEN);
    printf("Request receieved: \n%s",request);
    char* http_header="HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: %ld\n\n";
    char* file_name=get_file_path(request);
    printf("%s\n=======\n",file_name);
    FILE* fstr=fopen(file_name,"r");

    perror("file open status:");
    if(fstr==NULL){
	printf("Memory leak occured\n");
	printf("File not found\nShould send error code\n");
        exit(0);    
    }

    long unsigned int file_len=get_length(fstr);
    printf("Value of file_len %ld\n",file_len);
    sprintf(response,http_header,file_len);
    
    printf("Response:\n %s\n",response);

    long unsigned int bytes_read=0;
    int index=strlen(response);
    while(bytes_read<file_len){
        int cur_read=fread(response+index,1,RES_LEN-index,fstr);
        bytes_read+=cur_read;
        index=0;
        write(sockfd,response,RES_LEN);
        printf("Written %d bytes to socket\n",cur_read);
    }
    fclose(fstr);
    free(request);free(response);free(file_name);
}
int main(){
    int sockfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    struct sockaddr_in info;
    info.sin_family=AF_INET;
    info.sin_port=htons(55555);
    info.sin_addr.s_addr=htonl(INADDR_ANY);

    bind(sockfd,(struct sockaddr*)&info,sizeof(info));
    perror("Bind status");
    
    listen(sockfd,10);
    struct sockaddr_in cliaddr;
    socklen_t cliaddr_len=sizeof(cliaddr);

    int connfd=accept(sockfd,(struct sockaddr*)&cliaddr,&cliaddr_len);
    handle_client(connfd);
    close(connfd);
}
