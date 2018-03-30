/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/stat.h> 
#include  <iostream>
#include "quic.hpp"


using namespace std;

#define BUFSIZE 1024



void send_file(int sock,const char* filename,long size,struct sockaddr_in serveraddr,int serverlen) 
{ 
    cout<<"File sending service[UDP]:"<<endl;
    char buf[1032]; 
    int32_t ack=0;
    string str1;
    struct stat s;
    if (stat(filename, &s) == -1)
    {
        printf("Can't get file info"); 
        return;
    }
    FILE *file = fopen(filename, "rb"); 
    if (!file)
    {
        printf("Can't open file for reading"); 
        return;
    }

    size = s.st_size;
    long tmp_size = htonl(size);
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    int ctr=1;

    while (size > 0)
    { 
        cout<<"Size = "<<size<<endl;
        int minm=1024;
        if(minm>size)
            minm=size;


        int rval = fread(buf, 1, minm, file);


          // for(int i=0;i<minm;i++){
          //   cout<<buf[i]<<" ";
          // }   
          cout<<endl;
        int sent = appsend( buf, 1024, sock, serveraddr, sizeof(serveraddr));

        char *size_c=(char*) &minm;
        char* ctr_c=(char*) &ctr;
        char* ack_c=(char*) &ack;
        size-=minm;



    }

   
} 



int main(int argc, char **argv) {
    int sockfd, portno, n;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
      (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);

 
    //string filename="axes.png";
    //char* filename_n ="axes.png";
    //cout<<"Enter the filename you want to send:";
    //cin>>temp;
    char * filn="axes.png";
    // send_info(temp.c_str(),sockfd,serveraddr,sizeof(serveraddr));
    string temp(filn);
    
    struct stat s;
    if (stat(temp.c_str(), &s) == -1)
    {
        printf("Can't get file info"); 
        return 0;
    }

    long size = s.st_size;
    cout<<size<<endl;
    
    send_file(sockfd,temp.c_str(),size,serveraddr,sizeof(serveraddr));
    


    /* get a message from the user */
    // bzero(buf, BUFSIZE);
    // printf("Please enter msg: ");
    // fgets(buf, BUFSIZE, stdin);

    // /* send the message to the server */
    // serverlen = sizeof(serveraddr);
    // n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr, (socklen_t)serverlen);
    // if (n < 0) 
    //   error("ERROR in sendto");
    
    // /* print the server's reply */
    // n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr,(socklen_t*) &serverlen);
    // if (n < 0) 
    //   error("ERROR in recvfrom");
    // printf("Echo from server: %s", buf);
    return 0;
}
