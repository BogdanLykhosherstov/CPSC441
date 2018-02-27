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
	if(lengthOfFile % 8 == 0)
	{
		printf("File size is evenly divisible by 8, equal sized chunks\n");
		chunkSize = (lengthOfFile/8);
		printf("Each chunk size is %d\n", chunkSize);
	}


	printf("File\n %s", content);

	while(1)
	{
		char buffer[1000];

		//read content into buffer from client
		int len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_address, &client_address_len);

		//print client ip address
		buffer[len] = '\0';
		printf("received: '%s' from client %s on port %d\n", buffer, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));


			char part[1024];
			//sending loop, the size of each chunk sent is chunksize
			for(int i = 0; i < lengthOfFile; i = i+chunkSize)
			{

				strncpy(part, content+i, chunkSize);
				printf("Chunk: \n %s\n", part);
				part[chunkSize] = '\0';

				//printf("Chunk: \n %s\n", part);
				int sent_len = sendto(sock, part, sizeof(part), 0, (struct sockaddr *)&client_address, client_address_len);
				printf("Server sent: %d\n", sent_len);


			}


	}

	close(sock);


	/*// Ask user for port number to listen to
	printf("Please enter port number to listen to: \n");
	scanf("%d", &port);

	printf("Listening on port: %d \n", port);


	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	{
		printf("Could not setup a socket!\n");
		return 1;
	}
	else
	{
		printf("Created socket\n");
	}

	// Setup structs
	memset((char *) &si_server, 0, sizeof(si_server));
	si_server.sin_family = AF_INET;
	si_server.sin_port = htons(port);
	si_server.sin_addr.s_addr = htonl(INADDR_ANY);
	server = (struct sockaddr *) &si_server;
	client = (struct sockaddr *) &si_client;

	// Bind to port
	if(bind(s, server, sizeof(si_server))==-1)
	{
		printf("Could not bind to port %d\n", port);
		return 1;
	}
	else
	{
		printf("Binded to port: %d\n", port);
	}

	// Listen loop

	int quit = 0;
	while(!quit)
	{
		if((readBytes=recvfrom(s, buf, MAX_BUF_LEN, 0, client, &len))==-1)
		{
			printf("Read error\n");
			quit = 1;
		}
		buf[readBytes] = '\0';	// padding

		printf(" Server recieved command \"%s\" from client\n", buf);

		if(strncmp(buf, "quit", 4) == 0)
			quit = 1;

		if(quit == 1)
		{
			sprintf(tosend, "%s", "OK");
		}
		else if(strncmp(buf, "get", 3) == 0)
		{
			sprintf(tosend, "Okay. What file?");
		}
		else
		{


			//Calculate each chunk size
			if(lengthOfFile % 8 == 0)
			{
				printf("File size is evenly divisible by 8, equal sized chunks\n");
				chunkSize = (lengthOfFile/8);
				printf("Each chunk size is %d\n", chunkSize);
			}


			//Send message to client that file is coming
			//sprintf(tosend, "Okay file %s, coming...", buf);
			//sendto(s, tosend, strlen(tosend), 0, client, len);

			//Move file pointer back to the start of the file
			rewind(fp);


			content = (char*) malloc(sizeof(char)*lengthOfFile);

			fread(content, 1, lengthOfFile, fp);
			content[lengthOfFile] = '\0';
			fclose(fp);

			printf("File\n %s", content);

			//sending loop, the size of each chunk sent is chunksize
			for(int i = 0; i < lengthOfFile; i = i+chunkSize)
			{

				char part[1024];
				strncpy(part, content+i, chunkSize);
				printf("Chunk: \n %s\n", part);
				part[chunkSize] = '\0';


				sendto(s, part, sizeof(part), 0, client, len);

			}
			//sendto(s, content, strlen(content), 0, client, len);

			printf("File size is %d\n", lengthOfFile);
		}

		printf("	Sending back \"%s\" as a responce\n", tosend);

		sendto(s, tosend, strlen(tosend), 0, client, len);

		memset(buf, 0, sizeof(buf));

	}*/
	//close(s);
	return 0;

}
