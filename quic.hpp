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
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <bits/stdc++.h>
#include <mutex> 
#include <semaphore.h>
using namespace std;

#define SLEEP_VAL 1
#define QUEUE_SIZE 10000
#define BUFSIZE 1024
#define MAX_WINDOW 3000

extern int alarm_fired;
extern queue<char> sender_buffer;
extern int acknowledgement_received;
extern int receiver_window;
extern int acknowledgement_changed;
extern int dup_ack;
extern int window_size;
extern int ssthresh;
extern int sock,serverlen;
extern struct sockaddr_in serveraddr;
extern char* data;
extern int len;
extern int exp_ack;
extern int recv_len;
extern char *recv_data;
extern sem_t mtx1,mtx2,mtx3,mtx4,mtx5;

void error(char *msg);
void timeout_sig(int sig);
void *sendbuffer_handler(void *arg);
void* rate_control(void* arg);
void update_window(int ackn,int rwnd);
void* parse_packets(void* arg);
void appsend(char* datat,int lent, int sockfd, struct sockaddr_in serveraddrt,int serverlent);
void recvbuffer_handler(char* packet_recv);
void* parse_packets(void* arg);
void* acquire_data(void* arg);
void clear( std::queue<char> &q );
void apprecv(char* datat,int lent, int sockfd, struct sockaddr_in serveraddrt,int serverlent);







