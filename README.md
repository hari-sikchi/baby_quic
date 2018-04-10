# baby_quic

## Miniature implementation of internet transport layer protocol based on Google's QUIC architecture.

![alt text](https://github.com/hari-sikchi/baby_quic/blob/master/structure.png)
* Design Details: The ppt design_document.pptx contains all the information regarding code structure.
* test_client1 and test_server1 are the test files for sending and receiving data.


## Compilation instruction  

 1. Add your test files in the format given on the Makefile.  
 2. Run `make`



## Running instruction
* To run a server version of the protocol 
1. Run `./server <port number>`

* To run a client version of the protocol
1. Run `./client <server address> <port number on which server is hosted>`

