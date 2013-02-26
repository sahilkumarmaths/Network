/*
 * Proxy Server
 */
#include<iostream>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<netdb.h> 			
#include<arpa/inet.h>
using namespace std;

int proxy_port= 3490;		//Port the clients will be connecting on
#define BACKLOG	10			//The pending connections

int check_port(int argc,char *argv[]);

int main(int argc, char *argv[])
{
	printf("\n");
	proxy_port = check_port(argc,argv);			//Getting the listening port
	printf("\n");
	int proxy_fd;								//Proxy socket file descriptor
	int client_fd;								//Client's socket file descriptor
	struct sockaddr_in proxy_addr;			//proxy address info
	struct sockaddr_in client_addr;			//Client address info
	int sin_size = sizeof(struct sockaddr_in);
	char buf[1000];
	int data_recieved =0;						// Bytes of data recieved from client
	
	proxy_fd= socket(PF_INET, SOCK_STREAM,0); 	// Creating active socket
	if(proxy_fd<0)
	{
			perror("Proxy-Socket: Could not create socket.\n");
			exit(1);
	}
	printf("Proxy-Socket Creation-Successful.\n");
	
	//Filling struct sockaddr_in for proxy
	proxy_addr.sin_family = AF_INET;			//Host Byte Order
	proxy_addr.sin_port = htons(proxy_port);	//Short Network Byte Order
	proxy_addr.sin_addr.s_addr= INADDR_ANY;		//Auto fll my IP
	memset(&(proxy_addr.sin_zero),'\0',8);		//Zero the rest of struct

	//Binding to local address
	if( bind(proxy_fd,(struct sockaddr *)&proxy_addr, sizeof(struct sockaddr))<0)
	{
			perror("Proxy-Bind: Could not bind to local address.\n");
			exit(1);
	}
	printf("Proxy-Binding-Successful.\n");
	//Listening incoming connections
	if(listen(proxy_fd, BACKLOG)<0) 
	{
			perror("Proxy-Listen: Could not listen.\n");
			exit(1);	
	}
	
	printf("Proxy-Listening-Successful.");
	while(1)
	{
		printf("\n\n\t\tProxy-Wating for New client connections.\n");
		client_fd = accept(proxy_fd,(struct sockaddr *)&client_addr,(unsigned int *)&sin_size);
		if( client_fd<0)
		{
				perror("Proxy-Accept: Could not accept connection.\n");
				exit(1);	
		}
		printf("Proxy-Client Connection-Successful.\n");
		printf("Proxy-Recievind Data \n\n");
		data_recieved = recv(client_fd, buf, 1000, 0);
		
		string str(buf);						//Converting char[] to string
		string s = str.substr(0,data_recieved);	//Taking substring of data
	
		printf("Bytes of Data Recieved: %d \n",data_recieved);
		printf("Data: %s",s.c_str());
		printf("\nProxy-Data Recieved \n");
		
		/***********Code to Send the data to the Remote Host *********/
		
		int http_fd;								//HTTP socket on sock_fd
		int http_data_recieved;						//Data_recieved from Http
		struct sockaddr_in http_addr;				//HTTP Address info
		struct hostent *h;							//HTTP hostnet
		char *message,http_reply[3000];
		
		http_fd = socket(PF_INET, SOCK_STREAM,0); 	//Active Open
		
		if(http_fd<0)
		{
				perror("Server-Socket: Could not create socket.\n");
				exit(1);
		}
	
		//Filling struct sockaddr_in for remote server
		h = gethostbyname("iitg.ernet.in");			//Getting hostname
		http_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr) ));	//Setting address
		http_addr.sin_family = AF_INET;				//Host Byte Order
		http_addr.sin_port = htons(80);				//Short Network Byte Order
		memset(&(http_addr.sin_zero),'\0',8);		//Zero the rest of struct
		
		//Connect to remote server
		if( connect(http_fd,(struct sockaddr *)&http_addr,sizeof(struct sockaddr) )<0)
		{
					perror("Server-Connect: Could not connect to remote server.\n");
					exit(1);
		}
		printf("Connected To: %s IP address: %s\n",h->h_name, inet_ntoa(*((struct in_addr *)h->h_addr)));
		//message = "GET / HTTP/1.0\r\n\r\n";
		message = "GET / HTTP/1.0\r\nHost: www.iitg.ernet.in\r\n";
		//Sending Data
		if(send(http_fd,message,strlen(message),0) < 0)
		{
				printf("Server-Send Data: Sending of data failed.");
				exit(1);
		}
		printf("Data sent to remote server.\n");
		
		//Recieving Data
		if( (http_data_recieved= recv(http_fd,http_reply,3000,0))<0)
		{
				printf("Server-Recieve Data: Reciving of data failed.");
				exit(1);
		}
		printf("Data recieved from remote server.\n\n");
		puts(http_reply);
		printf("\n\nData sent to client.\n");
		
		if( send(client_fd, http_reply,http_data_recieved, 0) <0)
		{
				printf("Proxy-Send Data to Client: Sending data to client failed.\n");
				exit(1);
		}else
		{
				printf("Proxy:Reply to Client-Successful\n");
		}
		close(http_fd);
		printf("Connection closed of remote server.\n");
		close(client_fd);
		printf("Connection closed of client.\n");
	}
	close(proxy_fd);
	printf("Connection closed of proxy.\n");
}
/*
 * Function to validate the ports
 */
int check_port(int argc,char *argv[])
{
	int server_port = 5432;
	if(argc==1)
	{
		printf("Enter port number >1023 and <49152: ");
		scanf("%d",&server_port);
	}else if(argc == 2)
	{
		server_port = atoi(argv[1]);	
	}else
	{
		printf("Bad Input\n");
		exit(1);
	}
	while(server_port<=1023 || server_port>= 49152)
	{
		printf("Enter port number >1023 and <49152: ");
		scanf("%d",&server_port);
	}
	return server_port;
}



