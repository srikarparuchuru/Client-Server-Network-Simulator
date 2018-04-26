#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 

char* crc_message;
char* message=NULL;

int crc_check(char* s,char* k)
{
    int slen=0,klen=0;
    while(s[slen]!='\0')
        slen++;
    while(k[klen]!='\0')
        klen++;
    //printf("%d %d\n",slen,klen);
    char* dividend = (char*)malloc(sizeof(char)*slen+klen+1);
    for(int i=0;i<slen;i++)
        dividend[i]=s[i];
    for(int i=slen;i<=slen+klen-1;i++)
        dividend[i]=k[i-slen];
    int current=0;
    while(1)
    {
        while(current<=slen+klen-1 && dividend[current]=='0')
            current++;
        if(current > slen)
            break;
        for(int i=current;i<=current+klen-1;i++)
            dividend[i] = (dividend[i]==k[i-current])?'0':'1'; 
    }   
    dividend[slen+klen]='\0';
    for(int i=slen;i<=slen+klen-1;i++)
    {
        if(dividend[i]=='1')
            return 0;
    }
    return 1;
}

void corrupt(char* ber)
{
    int len=0;
    while(message[len]!='\0')
        len++;
    int a = 0,ber_i=0;
    while(ber[ber_i]!='\0' && ber[ber_i]!='.')
    {
        a = a*10 + (ber[ber_i]-'0'); 
        ber_i++;
    }
    int error_count = (a * len)/100;
    while(error_count--)
    {
        int i = rand()%len;
        message[i] = (message[i]=='0')?'1':'0';
    }
    return;
}

void crc(char* s,char* k)
{
    int slen=0,klen=0;
    while(s[slen]!='\0')
        slen++;
    while(k[klen]!='\0')
        klen++;
    //printf("%d %d\n",slen,klen);
    crc_message = (char*)malloc(sizeof(char)*slen+klen+1);
    for(int i=0;i<slen;i++)
        crc_message[i]=s[i];
    for(int i=slen;i<=slen+klen-1;i++)
        crc_message[i]=k[i-slen];
    int current=0;
    while(1)
    {
        while(current<=slen+klen-1 && crc_message[current]=='0')
            current++;
        if(current > slen)
            break;
        for(int i=current;i<=current+klen-1;i++)
            crc_message[i] = (crc_message[i]==k[i-current])?'0':'1'; 
    }   
    for(int i=0;i<=slen-1;i++)
        crc_message[i]=message[i];
    crc_message[slen+klen]='\0';
    return;
}


int main(int argc, char *argv[])
{
    //char mesg[] = "Hello Mouli :D\n";
    int sockfd = 0, n = 0, sent = 0;
    char recvBuff[1024];
    struct sockaddr_in serv_addr; 

    if(argc != 3)
    {
        printf("\n Usage: %s <ip of server> <port>\n",argv[0]);
        return 1;
    } 

    memset(recvBuff, '0',sizeof(recvBuff));
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        return 1;
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2])); 

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    } 

    if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       return 1;
    } 
    printf("enter data to be sent to server  ");
    char inp[20],br[3];
    scanf("%s",inp);
    scanf("%s",br);
    message = strdup(inp);
    corrupt(br);
    crc(inp,"100000111");
    while(1)
    {
        printf("hi");
        sent = send(sockfd, crc_message, strlen(crc_message), 0);
        if ( (n = read(sockfd, recvBuff, sizeof(recvBuff)-1)) >= 0)
        {
            recvBuff[n] = 0;
            if(fputs(recvBuff, stdout) == EOF)
            {
                printf("\n Error : Fputs error\n");
            }
            if(crc_check(recvBuff,"100000111")==1)
            {
                if(strcmp(recvBuff,"1111111111010011111")==0)
                {
                    printf("crc check succesful.ACK recieved from server.Enter new data,BER to send\n");
                    scanf("%s",inp);
                    scanf("%s",br);
                    message = strdup(inp);
                    corrupt(br);
                    crc(inp,"100000111");        
                    printf("%s",crc_message);
                }
                else
                    printf("crc check succesful.NACK recieved from server.Enter new data,BER to send\n");
                    scanf("%s",br);
                    message = strdup(inp);
                    corrupt(br);
                    crc(inp,"100000111");
            }
            else
            {
                printf("crc check for ack/nack unsuccesful .. retransmitting same data,BER to server\n");
            } 
        } 
        if(n < 0)
        {
            printf("\n Read error \n");
        } 
    }
    close(sockfd);
    return 0;
}