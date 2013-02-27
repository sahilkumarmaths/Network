/*
 *Proxy Server
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
#include<algorithm>

using namespace std;

int proxy_port= 3490;										//Port the clients will be connecting on
#define BACKLOG	10											//The pending connections
	
int check_port(int argc,char *argv[]);
void check_1(string s,int proxy_fd, int client_fd);
void check_2(string s,int proxy_fd,int client_fd);
void getdata(struct buf_msg *dta, int proxy_fd, int client_fd);
void check_data(struct buf_msg *dta, char *buffer, int len,int proxy_fd,int  client_fd);
void final(struct buf_msg *dta, string s[25], int len,int proxy_fd,int  client_fd);

struct buf_msg{
		string message;
		string port;
		string url;
		string host;
	};
	
int main(int argc, char *argv[])
{
	printf("\n");
	proxy_port = check_port(argc,argv);		//Getting the listening port
	printf("\n");
	int proxy_fd;																								//Proxy socket file descriptor
	int client_fd;																							//Client's socket file descriptor
	struct sockaddr_in proxy_addr;					//proxy address info
	struct sockaddr_in client_addr	;			//Client address info
	int sin_size = sizeof(struct sockaddr_in);
	
	proxy_fd= socket(PF_INET, SOCK_STREAM,0); 	// Creating active socket
	if(proxy_fd<0)
	{
		 close(proxy_fd);
			perror("Proxy-Socket: Could not create socket.\n");
			exit(1);
	}
	printf("Proxy-Socket Creation-Successful.\n");
	
	//Filling struct sockaddr_in for proxy
	proxy_addr.sin_family = AF_INET;									//Host Byte Order
	proxy_addr.sin_port = htons(proxy_port);	//Short Network Byte Order
	proxy_addr.sin_addr.s_addr= INADDR_ANY;		//Auto fll my IP
	memset(&(proxy_addr.sin_zero),'\0',8);			//Zero the rest of struct

	//Binding to local address
	if( bind(proxy_fd,(struct sockaddr *)&proxy_addr, sizeof(struct sockaddr))<0)
	{
		 close(proxy_fd);
			perror("Proxy-Bind: Could not bind to local address.\n");
			exit(1);
	}
	
	printf("Proxy-Binding-Successful.\n");
	//Listening incoming connections
	if(listen(proxy_fd, BACKLOG)<0) 
	{
		 close(proxy_fd);
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
			 close(client_fd);
			 close(proxy_fd);
				perror("Proxy-Accept: Could not accept connection.\n");
				exit(1);	
		}
		printf("Proxy-Client Connection-Successful.\n");
		printf("Proxy-Recievind Data \n\n");
		
		buf_msg *dta = new struct buf_msg;
		getdata(dta, proxy_fd,client_fd);
   	cout<<"Host: " << dta->host<<"\n";
	   cout<<"Url: " <<dta->url <<"\n";
    cout<<"Port: "<<dta->port <<"\n";
		  cout<<"Message: \n" << dta->message ;
		/***********Code to Send the data to the Remote Host *********/
		
		int http_fd;																															//HTTP socket on sock_fd
		int http_data_recieved = 0;																//Data_recieved from Http
		struct sockaddr_in http_addr;												//HTTP Address info
		struct hostent *h;																								//HTTP hostnet
		char http_reply[100000];
		
		http_fd = socket(PF_INET, SOCK_STREAM,0); 	//Active Open
		if(http_fd<0)
		{
				perror("Server-Socket: Could not create socket.\n");
				exit(1);
		}
	
		//Filling struct sockaddr_in for remote server
		
		h = gethostbyname((dta->host).c_str());																																													//Getting hostname
		http_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr) ));	//Setting address
		http_addr.sin_family = AF_INET;																																																					//Host Byte Order
		http_addr.sin_port = htons( atoi((dta->port).c_str()));																													//Short Network Byte Order
		memset(&(http_addr.sin_zero),'\0',8);																																															//Zero the rest of struct
		
		//Connect to remote server
		if( connect(http_fd,(struct sockaddr *)&http_addr,sizeof(struct sockaddr) )<0)
		{
					perror("Server-Connect: Could not connect to remote server.\n");
					exit(1);
		}
		printf("Connected To: %s IP address: %s\n",h->h_name, inet_ntoa(*((struct in_addr *)h->h_addr)));
		//message = "GET / HTTP/1.0\r\n\r\n";
		//message = "GET / HTTP/1.0\r\nHost: www.iitg.ernet.in\r\n\r\n";
	
		if(send(http_fd, (dta->message).c_str(),(dta->message).length(),0) < 0)
		{
				printf("Server-Send Data: Sending of data failed.");
				exit(1);
		}
		printf("Data sent to remote server.\n");
		
		int byterecieved = 0;																	// Bytes recieved at single time
		string prev="", next="",tot = "";					// Variables for computation
		http_data_recieved = 0;
		
		while(1)
		{
				prev = next;
				byterecieved = recv(http_fd,http_reply,100000,0);
				if( byterecieved == 0)
				{
						break;
				}else if(byterecieved == -1)
				{
						printf("Server-Recieve Data: Recieving of data failed.");
						exit(1);
				}
				http_data_recieved += byterecieved;
				next = http_reply;		
				tot= prev+next;
				
				if( send(client_fd, http_reply,byterecieved, 0) <0)
				{
						printf("Proxy-Send Data to Client: Sending data to client failed.\n");
						exit(1);
				}
		}//While ends
		
		close(http_fd);
		printf("\nConnection closed of remote server.\n");
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
		printf("1. Bad Input\n");
		exit(1);
	}
	while(server_port<=1023 || server_port>= 49152)
	{
		printf("Enter port number >1023 and <49152: ");
		scanf("%d",&server_port);
	}
	return server_port;
}

/*
 * Taking multi line input from the client
 */
void getdata(struct buf_msg *dta, int proxy_fd, int client_fd)
{
	int data_recieved=-1;					//Data Recieved
	char buffer[2000]=""	; 			//Buffer
	int data_buffer = -1;					//Data in buffer
	char buf[1]="";
	while(1)
	{
		if((data_recieved = recv(client_fd, buf, 1, 0))<0 )
			break;
		data_buffer += data_recieved;	
		buffer[data_buffer] = buf[0];
		if(data_buffer >=4)
		{
				if(buffer[data_buffer-3] == 13 && buffer[data_buffer-2] == 10 && buffer[data_buffer-1] == 13 && buffer[data_buffer]	 == 10 )
					break;
		}
	}
	check_data(dta, buffer, strlen(buffer),proxy_fd, client_fd);
}

void check_data(struct buf_msg *dta,char *buffer, int len,int proxy_fd, int client_fd)
{
		string s = buffer;
		int pos = s.find("\r\n");
		int end = s.find("\r\n\r\n");
		int prev,sarrayc = 0;
		string sarray[25];
		string str1 = s.substr(0,pos);
		string str2;
		
		sarray[0] = str1;
		sarrayc++;
		check_1(str1, proxy_fd,  client_fd);					//Checking 1st line of string
		while(1)
		{
				prev = pos;
				if(pos 	== end)
					break;
				pos = s.find("\r\n",prev+2);
				str2 = s.substr(prev+2,pos-(prev+2));
				check_2(str2,proxy_fd, client_fd);					// Checking subsequent lines of string
				sarray[sarrayc] = str2; 
				sarrayc++;
		}
		final(dta,sarray,sarrayc,proxy_fd, client_fd);
}
/*
 * Final 
 */
void final(struct buf_msg *dta,string s[25], int len,int proxy_fd, int client_fd)
{
		string final="";
		string host_port_url="";														// May contain port
		string host="";
		string port="";
		string url = "";
		int valid = 0;																			// For checking validitiy
		int cas = 0;																					// Two cases in single line or many lines
		if(s[0].find(" /")!=-1)
		{
			port="80";
			url=s[0].substr(s[0].find("/"), s[0].find(" HTTP/1.0") -s[0].find("/") );
			cas = 0;
		}else
		{
				cas = 1;
				host_port_url =  s[0].substr(s[0].find(" http://")+8 , s[0].find(" HTTP/1.0") -(s[0].find(" http://")+8) );
				if(host_port_url.length()==0 )
				{
					 close(client_fd);
					 close(proxy_fd);
						printf("2. Bad Request (400) \n");
						exit(1);
				}
				valid = 1;
				int loc_col = host_port_url.find(':');
				int slash;
				if(loc_col >0)																		//Means there is port
				{
						host = host_port_url.substr(0,loc_col); 
						slash = host_port_url.find('/',loc_col +1);
						port = host_port_url.substr(loc_col +1, slash-(loc_col+1));
						url = host_port_url.substr(slash, s[0].find(" HTTP/1.0")-slash);
				}else
				{ 
							slash = host_port_url.find("/");
							if(slash<0)
							{
									host = host_port_url;
									port = "80";
									url = "/";
							}else
							{
									host = host_port_url.substr(0,slash);
									port = "80";
									url = host_port_url.substr(slash);
							}
				}
		}
		if(cas ==0)
				{
						for(int i=0; i<len; i++)
						{
							final += s[i] +"\r\n";
								if(s[i].find("Host: ")!=-1)
								{ 
									  int coln = s[i].find(":",5 );
									  if(coln>0)
									  {
												 host = s[i].substr(6,coln-6);
													port = s[i].substr(coln+1);
											}
									  else
									  {
													port = "80";
													host = s[i].substr(6);
											}
											valid = 1;
								}
						}
						final +="\r\n";
				}
				
		if(valid == 0 && cas == 0)
		{
				close(client_fd);
				close(proxy_fd);
				printf("3. Bad Request (400) \n");
				exit(1);
		}
		int get = s[0].find("GET ");
		if( get != -1)
		{
					if(cas ==1)
					{
							final += "GET "+url+" HTTP/1.0\r\n";
							final += "Host: "+host+":"+port+"\r\n";
							for(int i = 0 ; i < len; i++)
							{
									if(s[i].find("GET ") !=-1 || s[i].find("Host: ")!= -1)
									{
											continue;
									}
								final+= s[i] +"\r\n";
							}
							final+= "\r\n";	
					}
		}
		else
		{
			 close(client_fd);
			 close(proxy_fd);
				printf("Not Implemented (501) \n");
				exit(1);	
		}
		dta->message = final;
		dta->host = host;
		dta->url = url;
		dta->port = port;
		
}
/*
 * This funtion checks the validity of 1st string
 */
void check_1(string s,int proxy_fd,int  client_fd){
	 
	 if( (s.find(" /")>0  || s.find(" http://")>0 )&& s.find(" HTTP/1.0")>0 && count(s.begin(),s.end(),' ') == 2)
	 {
		}else
		{
			 close(client_fd);
			 close(proxy_fd);
				printf("4. Bad Request (400) \n");
				exit(1);
		}
	}
/*
 * This funtion checks the validity of the remaining strings
 */
void check_2(string s,int proxy_fd, int client_fd){
		int len = s.length(); 
		int pos_col1 = s.find(':');
		
		if( len<4 || pos_col1 ==-1 || pos_col1 ==0 || pos_col1 == len-1  )				// No : in header or start with :
		{
			 close(client_fd);
			 close(proxy_fd);
				printf("5. Bad Request (400) \n");
				exit(1);
		}
		if(pos_col1>=1 && s[pos_col1-1] ==' ' )																		// header :
		{
			 close(client_fd);
			 close(proxy_fd);
				printf("6. Bad Request (400) \n");
				exit(1);
		}
		
		if(pos_col1 <len-1 && pos_col1>=0 && s[pos_col1+1]!= ' ')									// No space after 1st : 
		{
			 close(client_fd);
			 close(proxy_fd);
				printf("7. Bad Request (400) \n");
				exit(1);
		}
		int pos_col2 = s.find(':',pos_col1 +1);
		if(pos_col2 >= 0)
		{
				if(s.find("Host: ")<0 || pos_col2 == len-1 )
				{
					 close(client_fd);
			   close(proxy_fd);
						printf("8. Bad Request (400) \n");
						exit(1);
				}
				if( (pos_col2<len-1 &&  s[pos_col2 +1] ==' ' ) ||  ( pos_col2>=1 && s[pos_col2-1] ==' '))
				{
					 close(client_fd);
						close(proxy_fd);
						printf("9. Bad Request (400) \n");
						exit(1);
				}
		}
}	
	

