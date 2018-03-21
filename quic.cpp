/* 
 * udpclient.c - A simple QUIC client
 * usage: udpclient <host> <port>
 */
#include "quic.hpp"


int alarm_fired = 0;
queue<char> sender_buffer;
queue<char> receiver_buffer;

int acknowledgement_received;
int receiver_window;
int acknowledgement_changed;
int dup_ack=0;
int window_size=3;
int ssthresh=64;
int sock,serverlen;
struct sockaddr_in serveraddr;
char  *data;
int len;
int exp_ack;
int recv_len;
char *recv_data;
sem_t mtx1,mtx2,mtx3,mtx4,mtx5; 

// mtx1 is for queue sender buffer
//mtx2 is for acknowlegement_changed
// mtx3 is for acknowledgement received
//mtx4 is for window size
// mtx5 is for queue receiver_buffer

void error(char *msg) {
  perror(msg);
  exit(1);
}
void timeout_sig(int sig)
{
    pid_t pid;
    if (sig == SIGALRM)
    {
        alarm_fired = 1;
    }
    signal(SIGALRM, timeout_sig);
}



// Definition of the buffer thread after calling appsend

void *sendbuffer_handler(void *arg){

	while(len!=0){
		if(sender_buffer.size()!=QUEUE_SIZE){
			sem_wait(&mtx1);
			sender_buffer.push(*data);
			sem_post(&mtx1);
			data++;
			len--;

		}
		else
			continue;
	}
	pthread_exit(NULL);

}




void* rate_control(void* arg){
    cout<<"File sending service[Reliable]:"<<endl;
    char buf[1033]; 
    int32_t ack=0;
    int32_t chunk_size=0;

    struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	
    int ctr=1;
    int nextseqnum=1;
    int base=0;
    int sent,t_recv,ackn;
    int max_window_size=3000;
    char window_mem[max_window_size][1033];
    int endt=-1;
    int total_chunks;
    int resend_ctr=0;
    int close=0;
    float additive_increase=0;
    float additive_increase_ctr=0;    
    sem_wait(&mtx2);
    acknowledgement_changed=0;
    int close_connection=0;
    sem_post(&mtx2);
    while(1){
    	alarm_fired=0;
    	if(len==0&&sender_buffer.empty()&&base==nextseqnum-1)
    		break;


    	//cout<<"Window Size: "<<window_size<<endl;
		
		cout<<"Base: "<<base<<" | nextseqnum: "<<nextseqnum<<endl;
		//cout<<receiver_window<<" | "<<window_size<<endl;
		window_size=min(receiver_window,window_size);

		// If there is space in the current window, take a packet from sender_buffer and send it.
  		while(nextseqnum<=base+window_size&&!sender_buffer.empty()){
			int minm=1024;
			
			sem_wait(&mtx1);
			if(minm>sender_buffer.size())
			    minm=sender_buffer.size();
			int count_min=minm;
			int counter=0;
			while(count_min--){
					//window_mem[nextseqnum-base-1][counter]=sender_buffer.front();
					buf[counter+1]=sender_buffer.front();
					counter++;
					sender_buffer.pop();
			}
			sem_post(&mtx1);
			chunk_size=minm;
			char *size_c=(char*) &minm;
			char* ctr_c=(char*) &ctr;
			char *chunk_size_c= (char*)&chunk_size;
			//Copy the is_ackn region
			int is_ackn=0;
			char* is_ackn_c=(char*)&is_ackn;
			memcpy(buf,is_ackn_c,1);
			// Copy the size of the chunk
			memcpy(buf+1025 ,size_c, 4 );
			//counterpy the sequence number of the chunk
			memcpy(buf+1029,ctr_c , 4 );
			// Copy to memory
			memcpy(&window_mem[nextseqnum-base-1][0],buf, 1033);
			cout<<"Send data: "<<ctr<< "| "<<minm<<endl;
			sent = sendto(sock, buf, 1033, 0, (struct sockaddr *)&serveraddr, (socklen_t)serverlen);
			ctr++;
			nextseqnum++;

    	}

    	alarm(SLEEP_VAL);
    	(void) signal(SIGALRM, timeout_sig);
    	int flag_timeout=1;
    	do
		    {	
		    	// If correct acknowledgment is received by the receiver thread
		    	//cout<<"REACHES wait for lock!"<<endl;

		    	sem_wait(&mtx3);
				if(acknowledgement_received>base){
					ackn=acknowledgement_received;
					int j=0;
					//shift the window
					for(int i=ackn-base;i<nextseqnum;i++){
						for(int k=0;k<1033;k++){
							window_mem[j][k]=window_mem[i][k];
						}
						j++;
					}

						if(window_size<ssthresh)	
							window_size=min(window_size+ackn-base,max_window_size);
						else{
							if(additive_increase==0){
								additive_increase=1/window_size;
								additive_increase_ctr=0;
								additive_increase_ctr+=(ackn-base);
							}
							else{
								additive_increase_ctr+=(ackn-base);
							}
							if(additive_increase_ctr*additive_increase>=1){
								additive_increase_ctr=0;
								additive_increase=0;
								window_size+=1;
								window_size=max(window_size,max_window_size);
							}
						}

						base=ackn;
						//dup_ack=0;
						flag_timeout=0;
						acknowledgement_changed=0;
						break;
				
				}
				else if(acknowledgement_received<base&&acknowledgement_received==0){
					close_connection=1;
					break;	

				}
				else if(acknowledgement_changed==1){

					acknowledgement_changed=0;
					flag_timeout=0;
					break;

				}
				else{
					// Wait till the timeout to see if the receiver thread can get a correct acknowledgment
					//do nothing
				}
				sem_post(&mtx3);
				
					        
		    }while(!alarm_fired);
		alarm_fired=0;
		int ret_val;
	    sem_getvalue(&mtx3,&ret_val);
	    if(ret_val<=0){
	    	sem_post(&mtx3);
	    }

	    if(close_connection)
	    	break;
		// If timed out then reduce the window size and resend all packets
		if(flag_timeout){
			if(ssthresh>=2)
				ssthresh/=2;
			sem_wait(&mtx4);
			window_size=1;
			sem_post(&mtx4);
			/// Fix to close the connection
			// if(resend_ctr==4)
			// 	{
			// 		close=1;
			// 		break;
			// 	}
			// Resend the entire window
				cout<<"Resend Window"<<endl;
			for(int i=0;i<min(window_size,nextseqnum-base-1);i++){
				memcpy(buf,&window_mem[i][0], 1033 );
				sent = sendto(sock, buf, 1033, 0, (struct sockaddr *)&serveraddr, (socklen_t)serverlen);				
				}
			resend_ctr++;
		}


	}


	pthread_exit(NULL);

}

void update_window(int ackn,int rwnd){
	sem_wait(&mtx3);
	if(acknowledgement_received>0&&ackn==0)
		acknowledgement_received=0;
	else{
	acknowledgement_received=max(ackn,acknowledgement_received);
	}


	sem_post(&mtx3);
	if(ackn<acknowledgement_received){
		dup_ack++;
	}
	if(dup_ack==3){
		if(ssthresh>=2)
			ssthresh/=2;
		sem_wait(&mtx4);
		if(window_size>=2)
			window_size/=2;
		sem_post(&mtx4);
		dup_ack=0;
	}
	if(receiver_window>0)
		receiver_window=rwnd;

}


void recvbuffer_handler(char* packet_recv){

	char buff[1032];
	memcpy(buff,packet_recv+1,1032);
	// If queue if full drop packet
	sem_wait(&mtx5);
	if(QUEUE_SIZE-receiver_buffer.size()<1032){
		cout<<"Receiver queue is full!"<<endl;
		sem_post(&mtx5);
		return;
	}
	else{
		// cout<<"Filling the receiver queue"<<endl;

		char ackn[1033];
		
		int is_ackn=1;
		int rwnd;
		if(!receiver_buffer.empty())
		 {
		 	rwnd=QUEUE_SIZE-receiver_buffer.size();
		}		
		else
			rwnd=QUEUE_SIZE;

		for(int i=0;i<1032;i++){
			receiver_buffer.push(buff[i]);
		}

		int32_t ack;	
	 	int32_t size_r;
	 	char *size_c=(char*)&size_r;
	 	int ctr=1;
	 	int recv_s=0;
	 	char *recv_c=(char*)&recv_s;

		memcpy(recv_c,buff+1028,4);
        memcpy(size_c,buff+1024,4);
    	ack = htonl(exp_ack-1);
    	char* ack_c= (char*)&ack;


    	memcpy(ackn,(char*)&is_ackn,1);
    	memcpy(ackn+1,ack_c,4);
    	memcpy(ackn+5,(char*)&rwnd,4);


    	if(recv_s!=exp_ack){
        	int sent;
        	sent = sendto(sock, ackn, sizeof(ackn), 0,  (struct sockaddr *) &serveraddr, (socklen_t)serverlen);
        	cout<<"[Old]ACK sent for number: "<<exp_ack-1<<" Bytes:"<<sent<<endl;
    	}
    	else if(recv_s==exp_ack){
        	//Some bug here
        	int sent;
        	exp_ack+=1;
        	ack = htonl(exp_ack-1);
        	sent = sendto(sock, ackn, sizeof(ackn), 0,  (struct sockaddr *) &serveraddr, (socklen_t)serverlen);
			cout<<"ACK sent for number: "<<exp_ack-1<<" Bytes:"<<sent<<endl;
			//exp_ack+=1;
			//ack = htonl(exp_ack);
        	}
	}
	sem_post(&mtx5);
	
}

void* parse_packets(void* arg){

	while(1){
	char packet_recv[1033];
	// Acknowledgement will be cumulative_sequence_number|receiver_window_size
    struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	int t_recv=recvfrom(sock, packet_recv, sizeof(packet_recv), 0,(struct sockaddr *)&serveraddr, (socklen_t*)&serverlen);
	
	if(t_recv>=0){
		int is_ackn;
		char *is_ackn_c=(char*)&is_ackn;
		memcpy(is_ackn_c,packet_recv,1);
		if(ntohl(is_ackn)==0){
			// Is not an acknowlodegment / Is Data
			//cout<<"Packet is a data packet"<<endl;

			recvbuffer_handler(packet_recv);

		}
		else{
			//cout<<"Packet is an acknowledgement"<<endl;
			//Is an acknowledgement
			// Acknowledgement will be cumulative_sequence_number|receiver_window_size
			int ackn;
			char* ackn_c=(char*)&ackn;
			int rwnd;
			char* rwnd_c=(char*)&rwnd;
			sem_wait(&mtx1);
			acknowledgement_changed=1;
			sem_post(&mtx1);
			memcpy(ackn_c,packet_recv+1, 4);
			ackn=ntohl(ackn);
			cout<<"ackn:"<<ackn<<endl;
			memcpy(rwnd_c,packet_recv+5, 4);
			//cout<<"Rwnd:"<<rwnd<<endl;
			//rwnd=ntohl(rwnd);

			// Got ackn and rwnd
			update_window(ackn,rwnd);
			}


	}
	}	



}




void* acquire_data(void* arg){
	int len_rem=recv_len;
	int ctr=1	;
	int j=0;
	int temp=len_rem;
	while(len_rem){
		sem_wait(&mtx5);
		if(receiver_buffer.size()<1032){
			//do nothing
		}
		else{
			char buff[1032];
			for(int i=0;i<1032;i++){

				buff[i]=receiver_buffer.front();
				receiver_buffer.pop();
			}
			int val,size;
			char* val_c=(char*)&val;
			char * size_c=(char*)&size;
			memcpy(size_c,buff+1024,4);
			memcpy(val_c,buff+1028,4);
			cout<<len_rem<<endl;
			
			if(val==ctr){
				cout<<val<<" | "<<size<<endl;
				int minm=min(1024,size);
				for(int i=0;i<minm;i++){
					recv_data[j]=buff[i];
					j++;
					len_rem--;
				}

				ctr++;
			}


		}
		sem_post(&mtx5);
	}
		cout<<temp<<endl;
		cout<<"Acquring finish!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;

	pthread_exit(NULL);

}

void clear( std::queue<char> &q )
{
   std::queue<char> empty;
   std::swap( q, empty );
}

void appsend(char* datat,int lent, int sockfd, struct sockaddr_in serveraddrt,int serverlent){
	// Initialize all the global variables and create a new thread
	 sem_init(&mtx1, 0, 1);
	 sem_init(&mtx2, 0, 1);
	 sem_init(&mtx3, 0, 1);
	 sem_init(&mtx4, 0, 1);
	 sem_init(&mtx5, 0, 1);
	 clear(sender_buffer);

	sock=sockfd;
	serverlen=serverlent;
	serveraddr=serveraddrt;
	data=datat;
	len=lent;
	receiver_window=MAX_WINDOW;

	pthread_t send_buffer,rate_controller,packet_parser;
	int ret =  pthread_create(&send_buffer, NULL, &sendbuffer_handler, NULL);
	ret=	 pthread_create(&rate_controller, NULL, &rate_control, NULL);
	ret= pthread_create(&packet_parser, NULL, &parse_packets, NULL);

	pthread_join(send_buffer,NULL);
	pthread_join(rate_controller,NULL);
	pthread_cancel(packet_parser);
	return;

}

void apprecv(char* datat,int lent, int sockfd, struct sockaddr_in serveraddrt,int serverlent){
	// Initialize all the global variables and create a new thread
	 sem_init(&mtx1, 0, 1);
	 sem_init(&mtx2, 0, 1);
	 sem_init(&mtx3, 0, 1);
	 sem_init(&mtx4, 0, 1);
	 sem_init(&mtx5, 0, 1);

	 clear(receiver_buffer);
	 receiver_window=MAX_WINDOW;
	sock=sockfd;
	serverlen=serverlent;
	serveraddr=serveraddrt;
	exp_ack=1;
	recv_len=lent;
	recv_data=datat;

	pthread_t recv_buffer,packet_parser,data_get;
	int ret =  pthread_create(&recv_buffer, NULL, &acquire_data, NULL);
	ret= pthread_create(&packet_parser, NULL, &parse_packets, NULL);

	cout<<"Created Data Aquirer and Packet Parser:"<<endl;

	pthread_join(recv_buffer,NULL);
	pthread_cancel(packet_parser);
	return;

}




