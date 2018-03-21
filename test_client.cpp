#include "quic.hpp"


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
    char data[60000];
    for(int i=0;i<60000;i++){
    	data[i]='A';
    }
    data[60000-1]='G';
    data[60000-2]='G';
    data[60000-3]='G';
    data[60000-4]='G';
    data[60000-5]='G';
    
    data[60000-6]='B';
    //memset(data,0,sizeof(data));
    appsend(data,60000,sockfd,serveraddr,sizeof(serveraddr));

    //rate_control(sockfd,serveraddr,sizeof(serveraddr));
	
	return 0;
}
