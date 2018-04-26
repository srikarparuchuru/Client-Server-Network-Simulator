#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

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


int main(int argc, char * argv[]) 
{
    if(argc!=2)
    printf("Error format : %s <PORT>", argv[0]);
    int sock, cli, sent,n;
    struct sockaddr_in server, client;
    int len;
    char ack[] = "1111111111010011111";
    char nack[] = "11111111111";
    char recvBuff[1024];

    
    if((sock = socket(AF_INET, SOCK_STREAM,0)) == -1)
    {
        perror("socket : ");
        exit(-1);
    }
    
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(argv[1]));
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    bzero(&server.sin_zero, 8);

    len = sizeof(struct sockaddr_in);

    if((bind(sock, (struct sockaddr *)&server, len)) == -1)
    {
        perror("bind");
        exit(-1);
    }

    if((listen(sock, 5)) == -1)
    {
        perror("listen");
        exit(-1);
    }

    while(1)
    {
        if((cli = accept(sock, (struct sockaddr *)&client, &len)) == -1)
        {
            perror("bind");
            exit(-1);
        }
        if(!fork())
        {
            close(sock);
            while(1)
            {
                if ((n = read(cli, recvBuff, sizeof(recvBuff) - 1)) >= 0)
                {
                    recvBuff[n] = 0;
                    if (fputs(recvBuff, stdout) == EOF)
                    {
                        printf("\n Error : Fputs error\n");
                    }
                }
                if (n < 0)
                {
                    printf("\n Read error \n");
                }
                if(crc_check(recvBuff,"100000111")==1)
                    sent = send(cli, ack, strlen(ack), 0);
                else
                    sent = send(cli, nack, strlen(nack), 0);
                printf("sent %d bytes to client : %s\n",sent,inet_ntoa(client.sin_addr));
            }
        }
         close(cli);
    }
}