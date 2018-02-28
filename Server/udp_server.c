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
#include <stdbool.h>


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


	int quit = 0;
	int octLeg;
	int partialOctoblock;
	int tinyOctoblock;
	int pointer = 0;
	int remainingFile = lengthOfFile;


	int seq_num=0;
	char seq_num_holder[1];
	while(!quit)
	{
		char buffer[1000];

		//read content into buffer from client
		int len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &client_address_len);

		//print client ip address
		buffer[len] = '\0';
		printf("received: '%s' from client %s on port %d\n", buffer, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));


		char part[1111];

		printf("length of file %d\n",lengthOfFile);

		//algorithm from d2l
		while(remainingFile > 8888)
		{
			octLeg = 8888 / 8;

			printf("in first block\n");

			// send the legs

			for(int i =0; i < 8; i++)
			{
				bzero(part, sizeof(part));


				strncpy(part, content+pointer, octLeg);
				pointer += octLeg;
				//convert seq num to character and attach it to the end of the part
				// sprintf(seq_num_holder, "%d", seq_num);
				// strcat(part, seq_num_holder);
				// seq_num++;

				printf("Chunk: \n %s\n", part);
				part[octLeg] = '\0';

				int sent_len = sendto(sock, part, sizeof(part), 0, (struct sockaddr *)&client_address, client_address_len);
				printf("Server sent: %d\n", octLeg);

			}

			// seq_num = 0;
			remainingFile = remainingFile - 8888;
		}
		if(remainingFile > 8)
		{
			partialOctoblock = remainingFile - (remainingFile % 8);
			octLeg = partialOctoblock/8;

			printf("In second if statement\n");
			printf("partialOcto: %d\n", partialOctoblock);
			printf("octo leg: %d\n", octLeg);

			//send 8 legs
			for(int i =0; i < 8; i++)
			{
				bzero(part, sizeof(part));
				// bzero(seqnum, sizeof(seqnum));

				strncpy(part, content+pointer, octLeg);
				pointer += octLeg;

				// sprintf(seqnum, "%d", seqNum);
				// strcat(part, seqnum);
				// seqNum++;

				printf("Chunk: \n %s\n", part);
				part[octLeg+1] = '\0';


				int sent_len = sendto(sock, part, sizeof(part), 0, (struct sockaddr *)&client_address, client_address_len);
				printf("Server sent: %d\n", octLeg);
			}


			remainingFile -= partialOctoblock;

		}
		if(remainingFile > 0)
		{
			tinyOctoblock = 8;
			octLeg = tinyOctoblock / 8;
			// seqNum = 0;

			printf("In last if\n");

			//send 8 legs
			for(int i =0; i < 8; i++)
			{
				bzero(part, sizeof(part));
				// bzero(seqnum, sizeof(seqnum));

				if(remainingFile > 0)
				{

					strncpy(part, content+pointer, octLeg);
					pointer += octLeg;

					// sprintf(seqnum, "%d", seqNum);
					// strcat(part, seqnum);
					// seqNum++;

					printf("Chunk: \n %s\n", part);
					part[octLeg] = '\0';

					int sent_len = sendto(sock, part, sizeof(part), 0, (struct sockaddr *)&client_address, client_address_len);
					printf("Server sent: %d\n", octLeg);
				}
				else
				{

					char space[1] = " ";
					printf("Sending blank spaces");
					int sent_len = sendto(sock, space, sizeof(space), 0, (struct sockaddr *)&client_address, client_address_len);
					printf("Server sent: %d\n", sent_len);
				}

				remainingFile -= octLeg;

			}

		}

		if(remainingFile <= 0)
		{
			printf("File fully sent\n");
			quit = 1;
		}

	}

	close(sock);


	return 0;

}
