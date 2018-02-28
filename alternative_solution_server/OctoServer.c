#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>


#define MAX_BUF_LEN 512

// Global
FILE *fp;

int main(int argc, char *argv[])
{

	char *content = NULL;

	//TCP address init
	struct sockaddr_in server;
	int MYPORTNUM=1234567;
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(MYPORTNUM);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	char buf[1024];

	// create the listening socket
	int lstn_sock;
	lstn_sock = socket(AF_INET, SOCK_STREAM, 0);
	if(lstn_sock < 0)
	{
		printf("error in socket\n");
		exit(-1);
	}

	//bind the socket
	int status;
	status = bind(lstn_sock, (struct sockaddr *) &server, sizeof(struct sockaddr_in));
	if(status < 0)
	{
		printf("error in bind\n");
	}

	//connect to tcp
	status = listen(lstn_sock, 5);
	if(status < 0)
	{
		printf("error in listen");
		exit(-1);
	}


	//accept connection
	int connected_sock;
	connected_sock = accept(lstn_sock, NULL, NULL);
	if(connected_sock < 0)
	{
		printf("error in accept\n");
		exit(-1);
	}

	// receive data
	int count;
	char rcv_message[1024];
	count = recv(connected_sock, rcv_message, sizeof(rcv_message), 0);
	if(count<0)
	{
		printf("error in recv\n");
	}
	else
	{
		printf("client said: %s\n", rcv_message);
	}
	printf("Opening, %s\n", rcv_message);

	fp = fopen(rcv_message, "r");
	if(!fp)
	{
		printf("Error opening\n");
		return 1;
	}

	fseek(fp, 0, SEEK_END);
	int lengthOfFile = ftell(fp);
	rewind(fp);

	content = malloc((lengthOfFile+1)*sizeof(*content));
	fread(content, lengthOfFile, 1, fp);
	content[lengthOfFile]='\0';
	printf("%s\n", content);
	fclose(fp);

	//send file info
	char tosend[10];
	sprintf(tosend, "%d", lengthOfFile);
	count = send(connected_sock, tosend, sizeof(tosend), 0);
	if(count < 0)
	{
		printf("Error in send\n");
	}


	close(connected_sock);
	close(lstn_sock);

	//UDP setup
	int SERVER_PORT = 8001;
	int chunkSize;
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;

	//char *content = NULL;


	server_address.sin_port = htons(SERVER_PORT);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);

	//create UDP socket
	int sock;
	if((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
	{
		printf("could not create udp socket\n");
		return 1;
	}

	printf("UDP socket created\n");

	//bind
	if((bind(sock, (struct sockaddr *)&server_address, sizeof(server_address))) < 0)
	{
		printf("could not bind\n");
		return 1;
	}

	printf("binded\n");

	struct sockaddr_in client_address;
	int client_address_len = sizeof(client_address);
	char client_name[100];

	//Calculate each chunk size
	// if(lengthOfFile % 8 == 0)
	// {
	// 	printf("File size is evenly divisible by 8, equal sized chunks\n");
	// 	chunkSize = (lengthOfFile/8);
	// 	printf("Each chunk size is %d\n", chunkSize);
	// }


	printf("File\n %s", content);

	char buffer[1111];

	char fileBufferNew[500000] = "";


	int fileRemaining = atoi(lengthOfFile);
	int octoleg = 1111;
	int partial_octoblock;
	int tiny_octoblock;
	int quit = 0;
	while(!quit){
		//while file size is  > 8888 bytes
		while(fileRemaining > 8888){

			for(int i=0; i<8; i++){
				int recv_bytes = sendto(sockU, buffer, octoleg, 0, NULL, NULL);
				printf("sent bytes = %d\n", recv_bytes);
				buffer[recv_bytes] = '\0';
				printf("sent: '%s'\n", buffer);
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

			fileRemaining -= tinyOctoblock;
		}
		//checks for errors
		printf("remaining part: %d\n", fileRemaining);
		//finally when no more data, exit
		if(fileRemaining <= 0)
		{
			quit = 1;
		}
	}
	printf("File Sent:");




	close(sockU);
	return 0;
}
