#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>


#define MAX_BUF_LEN 512
#define MAX_FILE_SIZE 20000

#define OCTOBLOCK_SIZE 8888
#define OCTOLEG_SIZE (OCTOBLOCK_SIZE/8)

// Server running localy
#define SERVER_PORT 8886
#define SERVER_IP "127.0.0.1"	// loopback interface

// Global Var
FILE *fp;
char filebuffer[MAX_FILE_SIZE];
int fileSize;

int main(void)
{

	//TCP variable creation
	struct sockaddr_in server;
	int MYPORTNUM = 1234567;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(MYPORTNUM);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	// Create listen socket
	int sock;
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if(sock < 0)
	{
		printf("Error in socket creat\n");
		exit(-1);
	}

	// connect to tcp server
	int status;
	status = connect(sock, (struct sockaddr *) &server, sizeof(struct sockaddr_in));
	if(status < 0)
	{
		printf("Error in connect()\n");
		exit(-1);
	}
	else
	{
		printf("connected\n");
	}


	char name[1024];
	printf("This is the client side of Octoput\n");
	printf("Enter in file name: ");
	scanf("%s", name);

	//send data
	int count;
	count = send(sock, name, sizeof(name), 0);
	if(count < 0 )
	{
		printf("error in send\n");
	}

	// receive data
	char rcv_msg[1024];
	count = recv(sock, rcv_msg, sizeof(rcv_msg), 0);
	if(count < 0)
	{
		printf("error in recv\n");
	}
	else
	{
		printf("Server: %s\n",rcv_msg);
	}
	char file_size[1024];
	strcpy(file_size, rcv_msg);
	close(sock);

	//UDP setup
	const char* server_name = "localhost";
	const int server_port = 8001;

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	inet_pton(AF_INET, server_name, &server_address.sin_addr);

	server_address.sin_port = htons(server_port);

	int sockU;
	if((sockU = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("could not create socket\n");
		return 1;
	}

	printf("client socket created\n");

	char* data_to_send = "Hi Server";

	int len = sendto(sockU, data_to_send, strlen(data_to_send), 0, (struct sockaddr*)&server_address, sizeof(server_address));


	char buffer[1111];



	int fileRemaining = atoi(file_size);
	char fileBufferNew[fileRemaining];
	int octoleg = 1111;
	int partial_octoblock;
	int tiny_octoblock;
	int quit = 0;
	while(!quit){
		//while file size is  > 8888 bytes
		while(fileRemaining > 8888){

			for(int i=0; i<8; i++){
				int recv_bytes = recvfrom(sockU, buffer, octoleg, 0, NULL, NULL);
				printf("received bytes = %d\n", recv_bytes);
				buffer[recv_bytes] = '\0';
				printf("recieved: '%s'\n", buffer);
				strcat(fileBufferNew, buffer);
			}
			fileRemaining = fileRemaining - 8888;
		}
		//after, check if its between 8888 and 8
		if(fileRemaining > 8){
			partial_octoblock = fileRemaining - (fileRemaining % 8);
			octoleg = partial_octoblock / 8;
			//get all 8 legs from server
			for(int i =0; i < 8; i++)
			{
				int recv_bytes = recvfrom(sockU, buffer, octoleg, 0, NULL, NULL);
				printf("received bytes = %d\n", recv_bytes);
				buffer[recv_bytes] = '\0';
				printf("recieved: '%s'\n", buffer);
				strcat(fileBufferNew, buffer);
			}
			fileRemaining = fileRemaining - partial_octoblock;
		}
		//then check if between 8 and 0
		if(fileRemaining > 0){
			tiny_octoblock = 8;
			octoleg = tiny_octoblock / 8;
			for(int i =0; i < 8; i++)
			{
				int recv_bytes = recvfrom(sockU, buffer, octoleg, 0, NULL, NULL);
				printf("received bytes = %d\n", recv_bytes);
				buffer[recv_bytes] = '\0';
				printf("recieved: '%s'\n", buffer);
				strcat(fileBufferNew, buffer);
			}

			fileRemaining -= tiny_octoblock;
		}
		//checks for errors
		printf("remaining part: %d\n", fileRemaining);
		//finally when no more data, exit
		if(fileRemaining <= 0)
		{
			quit = 1;
		}
	}
	printf("FULL FILE: %s\n", fileBufferNew);
	/* Write the content on a different file */
	//UNCOMMENT WHEN READY TO WRITE

	fp = fopen("new_file.txt", "w");
	int lengthFile = sizeof(fileBufferNew);
	for (int i = 0; i < lengthFile; i++) {
		fputc(fileBufferNew[i], fp);
	}
	fclose(fp);
	printf("Wrote the data to 'new_file.txt'\n");


	close(sockU);
	return 0;
}
