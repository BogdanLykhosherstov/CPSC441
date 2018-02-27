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


	char buffer[1000];
	char fileBufferNew[50000] = "";

	for(int i =0; i < 8; i++)
	{
		int recv_bytes = recvfrom(sockU, buffer, sizeof(buffer), 0, NULL, NULL);
		printf("received bytes = %d\n", recv_bytes);
		buffer[recv_bytes] = '\0';
		printf("recieved: '%s'\n", buffer);
		strcat(fileBufferNew, buffer);
	}
	printf("FULL FILE: %s\n", fileBufferNew);
	/* Write the content on a different file */
	//UNCOMMENT WHEN READY TO WRITE

	// fp = fopen("new_file.txt", "w");
	// int lengthFile = sizeof(fileBufferNew);
	// for (int i = 0; i < lengthFile; i++) {
	// 	fputc(fileBufferNew[i], fp);
	// }
	// fclose(fp);
	// printf("Wrote the data to 'new_file.txt'\n");




	close(sockU);

	/*quit = 0;

	while(!quit)
	{
		printf("Enter a command to send: ");
		scanf("%s", buf);

		if(strncmp(buf, "quit", 4) == 0)
		{
			quit = 1;
		}
		int len = sendto(s, buf,strlen(buf), 0, server, sizeof(si_server));



		for(int i =0; i < 8; i++)
		{
		readBytes=recvfrom(s, buf, len, 0, NULL, NULL);
		printf("received bytes = %d\n",readBytes);
		buf[len] = '\0';
		printf("received from server\n %s\n",buf);
		}

			//Check to see if file size was sent
			if(strncmp("File-Size:", buf, 9) == 0 )
			{
				token = strtok(buf, " ");
				token = strtok(NULL, " ");
				fileSize = atoi(token);
				printf("Client now has file-size: %d\n", fileSize);
			}

			fp = fopen("myfile.txt", "w");
			if(fp == NULL)
			{
				printf("Failed to open file\n");
				return 1;
			}

			fprintf(fp, "%s\n", buf);


			fclose(fp);
			fp = NULL;


		memset(buf, 0, sizeof(buf));

	}*/
	//close(s);
	return 0;
}
