/* 
 * udpserver.c - A UDP echo server 
 * usage: udpserver <port_for_server>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include  <iostream>
#include "quic.hpp"

using namespace std;
#define BUFSIZE 1024





int write_all(FILE *file, const void *buf, int len)
{
    const char *pbuf = (const char *) buf;

    cout<<"Length="<<len<<endl;

    while (len > 0)
    {
        int written = fwrite(pbuf, 1, len, file);
        if (written < 1)
        {
            printf("Can't write to file");
            return -1;
        }

        pbuf += written;
        len -= written;
    }

    return 0;
}

void read_file(int size,const char* filename,int chunks,int sock, struct sockaddr_in clientaddr,int clientlen ) 
{ 
    int rval; 
    char buf[1033];
    int32_t ack;
  int32_t size_r;
  char *size_c=(char*)&size_r;

    cout<<"Receiving File:"<<endl;

    //Send the filename here  
    string str1;

    //Read Size of file
    FILE *file;
    file = fopen(filename, "wb"); 
    
    if (!file)
    {
        printf("Can't open file for writing");
        return;
    }

    int ctr=1;
  int recv_s=0;
  char *recv_c=(char*)&recv_s;
    
    while (size > 0)
    {
      cout<<"Size= "<<size<<endl;

        long minm=1024;
        if(minm>size)minm=size;

        int recv= apprecv( buf, 1024,sock, clientaddr,sizeof(clientaddr));

      for(int i=0;i<minm;i++){
        cout<<buf[i]<<" ";
      }   
      cout<<endl;

        // cout<<"Bytes received:"<<recv<<endl;
        cout<<"Received: "<<recv<<endl;


        int write_size=min(1024,size);
          write_all(file, buf, write_size);
          ctr++;
          size-=minm;

        
    } 

  fclose(file);

  
} 


int main(int argc, char **argv) {
  int sockfd; /* socket file descriptor - an ID to uniquely identify a socket by the application program */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port_for_server>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the socket 
   */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
       (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
     sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
    char * filename;
    int size;
    int  chunks;

 
  cout<<"Enter folder you want to copy the file to(eg. /home/harshit/):";
  //cin>>location;
   char *loc="/home/harshit/Documents/networks/baby_quic/";
   string location(loc);
   filename="test1.jpg";
    string temp(filename);
    location=location+temp;
 
  // while (1) {

    
    // filename=read_info(&size,filename,&chunks,sockfd,clientaddr,clientlen);
    chunks=2;
 
    cout<<"Copying to : "<<location<<endl;

    //char * filename_new="axes_recv.png";
    read_file(9033,location.c_str(),chunks,sockfd,clientaddr,clientlen);

  // }
}
