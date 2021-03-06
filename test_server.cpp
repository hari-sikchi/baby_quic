#include "quic.hpp"


int main(int argc, char **argv) {
  int sockfd; /* socket file descriptor - an ID to uniquely identify a socket by the application program */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddrt; /* server's addr */
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
  bzero((char *) &serveraddrt, sizeof(serveraddrt));
  serveraddrt.sin_family = AF_INET;
  serveraddrt.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddrt.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(sockfd, (struct sockaddr *) &serveraddrt, 
       sizeof(serveraddrt)) < 0) 
    error("ERROR on binding");

  /* 
   * main loop: wait for a datagram, then echo it
   */
  clientlen = sizeof(clientaddr);
  init_quic();

  while (1) {

    char recv_buf[30000];
    apprecv(recv_buf,30000,sockfd,clientaddr,clientlen); 
    for(int i=30000-10;i<30000;i++){
        cout<<recv_buf[i];
    }
	cout<<endl;
    char data[30000];
    for(int i=0;i<30000;i++){
    	data[i]='A';
    }
    data[30000-1]='G';
    data[30000-2]='G';
    data[30000-3]='G';
    data[30000-4]='G';
    data[30000-5]='G';
    
    data[30000-6]='B';

 //  bzero((char *) &clientaddr, sizeof(clientaddr));
 // clientaddr.sin_family = AF_INET;
 
 //  clientaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
 //  clientaddr.sin_port = htons((unsigned short)2011);
    printf("Master at ip = : %s , port = %d \n" , inet_ntoa(clientaddr.sin_addr) , ntohs(clientaddr.sin_port));	

    //memset(data,0,sizeof(data));
    appsend(data,30000,sockfd,clientaddr,sizeof(clientaddr));


  }

}
