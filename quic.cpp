
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
int sock,serverlen,clientlen;
struct sockaddr_in serveraddr,clientaddr;

char  *data;
int len;
int exp_ack=1;
int recv_len;
char *recv_data;
sem_t mtx1,mtx2,mtx3,mtx4,mtx5; 
int glob_ctr=0;

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


void init_quic(){
	 sem_init(&mtx1, 0, 1);
	 sem_init(&mtx2, 0, 1);
	 sem_init(&mtx3, 0, 1);
	 sem_init(&mtx4, 0, 1);
	 sem_init(&mtx5, 0, 1);

	acknowledgement_received=0;
	ssthresh=64;
	clear(sender_buffer);
	clear(receiver_buffer);
	exp_ack=1;
	receiver_window=MAX_WINDOW;
	acknowledgement_received=0;

	pthread_t rate_controller,packet_parser;
	pthread_t recv_buffer;

	int ret=pthread_create(&rate_controller, NULL, &rate_control, NULL);
	ret= pthread_create(&packet_parser, NULL, &parse_packets, NULL);

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
    // is_ackn|data|size|seq_n
    // 

    int32_t ack=0;
    // int32_t chunk_size=0;

    struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	
    int ctr=1;
    // nextseqnum is the number whose acknowledgment is not yet received
    int nextseqnum=1;
    // base is the number whose acknowledgement is received
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
    int close_connection=0;
  
    sem_wait(&mtx2);  
    acknowledgement_changed=0;
    sem_post(&mtx2);

    while(1){
    	alarm_fired=0;


    	if(sender_buffer.empty()&&base==nextseqnum-1)
    		{
    			continue;
    		}


		cout<<"Base: "<<base<<" | nextseqnum: "<<nextseqnum<<endl;

    	cout<<"Buffer Size = "<<sender_buffer.size()<<endl;
    	cout<<"Window Size: |"<<window_size<<" |"<<endl;
		
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
					buf[counter+1]=sender_buffer.front();
					counter++;
					sender_buffer.pop();
			}

			sem_post(&mtx1);
			// chunk_size=minm;

			char *size_c=(char*) &minm;
			char* ctr_c=(char*) &ctr;
			// char *chunk_size_c= (char*)&chunk_size;

			//Copy the is_ackn region
			char is_ackn='0';
			// is_ackn=htonl(is_ackn);
			// char* is_ackn_c=(char*)&is_ackn;

			memcpy(buf,&is_ackn,1);
			// Copy the size of the chunk
			memcpy(buf+1025 ,size_c, 4 );
			//counter, is the sequence number of the chunk
			memcpy(buf+1029,ctr_c , 4 );
			// Copy to memory
			cout<<"Copied to: "<<nextseqnum-base-1<<endl;
			memcpy(&window_mem[nextseqnum-base-1][0],buf, 1033);
			sent = sendto(sock, buf, 1033, 0, (struct sockaddr *)&serveraddr, (socklen_t)serverlen);
			cout<<"Send data: "<<ctr<< "| "<<minm<<endl;

			ctr++;
			nextseqnum++;

    	}

    	alarm(SLEEP_VAL);
    	(void) signal(SIGALRM, timeout_sig);
    	int flag_timeout=1;
    	do
		    {	
		    	// If correct acknowledgment is received by the receiver thread

		    	sem_wait(&mtx3);
				if(acknowledgement_received>base){
					ackn=acknowledgement_received;
					int j=0;


					//shift the window
					for(int i=ackn-base;i<nextseqnum-base-1;i++){
						for(int k=0;k<1033;k++){
							window_mem[j][k]=window_mem[i][k];
						}
						j++;
						}


						if(window_size<ssthresh)	
							window_size=min(window_size+ackn-base,max_window_size);
						else{
							if(additive_increase==0){
								additive_increase=1.00/window_size;
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
						cout<<"base= "<<base<<endl;
						//dup_ack=0;
						flag_timeout=0;
						acknowledgement_changed=0;
						break;
				
				}
				// No need for this
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
			if(ssthresh>=32)
				ssthresh/=2;
			
			sem_wait(&mtx4);
			//window_size=1;
			window_size=200;
			sem_post(&mtx4);
			/// Fix to close the connection
			// if(resend_ctr==4)
			// 	{
			// 		close=1;
			// 		break;
			// 	}
			// Resend the entire window
				cout<<"Resend Window"<<endl;
				cout<<min(window_size,nextseqnum-base-1)<<endl;
			for(int i=0;i<min(window_size,nextseqnum-base-1);i++){
					memcpy(buf,&window_mem[i][0], 1033 );
					
					sent = sendto(sock, buf, 1033, 0, (struct sockaddr *)&serveraddr, (socklen_t)serverlen);
					// printf("Master at ip = : %s , port = %d \n" , inet_ntoa(serveraddr.sin_addr) , ntohs(serveraddr.sin_port));	
					 cout<<"Sent: "<<sent<<endl;			
				}
			resend_ctr++;
		}


	}
	cout<<"Rate controller Exit"<<endl;

	

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
		if(ssthresh>=32)
			ssthresh/=2;
		sem_wait(&mtx4);
		//if(window_size>=2)
			//window_size/=2;
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
	if(QUEUE_SIZE-receiver_buffer.size()<1024){
		cout<<"Receiver queue is full!"<<endl;
		sem_post(&mtx5);
		return;
	}
	else{
		// cout<<"Filling the receiver queue"<<endl;

		char ackn1[1033];
		
		char is_ackn='1';
		int rwnd;

		if(!receiver_buffer.empty())
		 {
		 	rwnd=QUEUE_SIZE-receiver_buffer.size();
		}		
		else
			rwnd=QUEUE_SIZE;

		int32_t ack;	
	 	int32_t size_r;
	 	char *size_c=(char*)&size_r;
	 	int ctr=1;
	 	int recv_s=0;
	 	char *recv_c=(char*)&recv_s;

		 memcpy(recv_c,packet_recv+1029,4);
         memcpy(size_c,packet_recv+1025,4);

    	ack = htonl(exp_ack-1);
    	 char* ack_c= (char*)&ack;


    	memcpy(ackn1,&is_ackn,1);
    	memcpy(ackn1+1,ack_c,4);
    	memcpy(ackn1+5,(char*)&rwnd,4);

    	cout<<"Data seq is: "<<recv_s<<" Required: "<<exp_ack<<endl;

    	if(recv_s!=exp_ack){
        	int sent;
        	sent = sendto(sock, ackn1, sizeof(ackn1), 0,  (struct sockaddr *) &clientaddr, (socklen_t)clientlen);
        	cout<<"[Old]ACK sent for number: "<<exp_ack-1<<" Bytes:"<<sent<<endl;
    	}
    	else if(recv_s==exp_ack){
		

			// Push the data in queue
			cout<<"size_r: "<<size_r<<endl;
			for(int i=0;i<size_r;i++){
				receiver_buffer.push(buff[i]);
				glob_ctr++;
			}
        	//Some bug here
        	cout<<"Received bytes: "<<glob_ctr<<endl;
        	int sent;
        	exp_ack+=1;
        	ack = htonl(exp_ack-1);
	    	// memcpy(ackn,&is_ackn,1);
		
        	//See what is happening here
	    	 memcpy(&ackn1[1],ack_c,4);

        	sent = sendto(sock, ackn1, sizeof(ackn1), 0,  (struct sockaddr *) &clientaddr, (socklen_t)clientlen);
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
	int t_recv=recvfrom(sock, packet_recv, sizeof(packet_recv), 0,(struct sockaddr *)&clientaddr, (socklen_t*)&clientlen);
	// printf("Master at ip = : %s , port = %d \n" , inet_ntoa(clientaddr.sin_addr) , ntohs(clientaddr.sin_port));
	// cout<<t_recv<<endl;
	if(t_recv>=0){
		char is_ackn;
		// char *is_ackn_c=(char*)&is_ackn;
		memcpy(&is_ackn,packet_recv,1);

			if(is_ackn=='0'){
			// Is not an acknowlodegment / Is Data
			cout<<"Packet is a data packet"<<endl;

			recvbuffer_handler(packet_recv);

		}
		else{
			cout<<"Packet is an acknowledgement"<<endl;
			//Is an acknowledgement
			// Acknowledgement will be cumulative_sequence_number|receiver_window_size
			int ackn;
			char* ackn_c=(char*)&ackn;
			int rwnd;
			char* rwnd_c=(char*)&rwnd;
			sem_wait(&mtx2);
			acknowledgement_changed=1;
			sem_post(&mtx2);
			memcpy(ackn_c,packet_recv+1, 4);
			ackn=ntohl(ackn);
			cout<<"ackn:"<<ackn<<endl;
			memcpy(rwnd_c,packet_recv+5, 4);

			// Got ackn and rwnd
			update_window(ackn,rwnd);
			}


	}

	}	



}




void* acquire_data(void* arg){
	int len_rem=recv_len;
	int j=0;
	int temp=len_rem;
	
	while(len_rem>0){
		// cout<<len_rem<<endl;
		sem_wait(&mtx5);
		if(receiver_buffer.size()==0){
			//do nothing
			// cout<<len_rem<<endl;
			// cout<<"empty buffer"<<endl;
		}
		else{
		


			int temp=receiver_buffer.front();
			receiver_buffer.pop();
			recv_data[j]=temp;
			j++;
			len_rem--;


		}
		sem_post(&mtx5);
	}
		cout<<"-----------Acquring finish-----------"<<endl;

	pthread_exit(NULL);

}

void clear( std::queue<char> &q )
{
   std::queue<char> empty;
   std::swap( q, empty );
}

int appsend(char* datat,int lent, int sockfd, struct sockaddr_in serveraddrt,int serverlent){
	// Initialize all the global variables and create a new thread
	sock=sockfd;

	serverlen=serverlent;
	serveraddr=serveraddrt;

	data=datat;
	len=lent;
	pthread_t send_buffer;
	int ret =  pthread_create(&send_buffer, NULL, &sendbuffer_handler, NULL);

	pthread_join(send_buffer,NULL);
	return 1;

}

int apprecv(char* datat,int lent, int sockfd, struct sockaddr_in &clientaddrt,int &clientlent){
	// Initialize all the global variables and create a new thread
	sock=sockfd;
	// clientlen=clientlent;
	// clientaddr=clientaddrt;
	recv_len=lent;
	recv_data=datat;

	pthread_t recv_buffer;
	int ret =  pthread_create(&recv_buffer, NULL, &acquire_data, NULL);

	cout<<"Start receiving:"<<endl;

	pthread_join(recv_buffer,NULL);


	clientaddrt=clientaddr;
	clientlent=sizeof(clientaddr);

	return 1;

}




