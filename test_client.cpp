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

 
    
while (1) {

    //string filename="axes.png";
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
    //memset(data,0,sizeof(data));
    appsend(data,30000,sockfd,serveraddr,sizeof(serveraddr));


    char recv_buf[30000];
    apprecv(recv_buf,30000,sockfd,serveraddr,sizeof(serveraddr)); 
    for(int i=30000-10;i<30000;i++){
        cout<<recv_buf[i];
    }
    cout<<endl;
}	
	return 0;
}
