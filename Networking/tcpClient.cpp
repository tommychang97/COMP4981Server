/*---------------------------------------------------------------------------------------
--	SOURCE FILE:		tcp_clnt.c - A simple TCP client program.
--
--	PROGRAM:		tclnt.exe
--
--	FUNCTIONS:		Berkeley Socket API
--
--	DATE:			February 2, 2008
--
--	REVISIONS:		(Date and Description)
--				January 2005
--				Modified the read loop to use fgets.
--				While loop is based on the buffer length 
--
--
--	DESIGNERS:		Aman Abdulla
--
--	PROGRAMMERS:		Aman Abdulla
--
--	NOTES:
--	The program will establish a TCP connection to a user specifed server.
-- The server can be specified using a fully qualified domain name or and
--	IP address. After the connection has been established the user will be
-- prompted for date. The date string is then sent to the server and the
-- response (echo) back from the server is displayed.
---------------------------------------------------------------------------------------*/
#include <iostream>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


#define SERVER_TCP_PORT		7000	// Default port
#define BUFLEN			3500  	// Buffer length
#define SEND_COUNT 10
#define GAME_OBJECT_BUFFER 65000

using namespace rapidjson;

void recvUpdates(int fd) {
	int count = 0;
	char recvbuf[GAME_OBJECT_BUFFER];
	printf("inside recvUpdate");
	while(true) {
		if (recvfrom(fd, recvbuf, sizeof(recvbuf), 0, NULL, NULL) < 0) {
			perror("read failed \n");
		};
		count++;
		printf("total received: %d\n", count);
	}
}

int main (int argc, char **argv)
{
	int n, bytes_to_read;
	int sd, udpSocket, port;
	struct hostent	*hp;
	struct sockaddr_in server;
	char  *host, *bp, rbuf[BUFLEN], sbuf[BUFLEN], **pptr;
	char str[16];

	switch(argc)
	{
		case 2:
			host =	argv[1];	// Host name
			port =	SERVER_TCP_PORT;
		break;
		case 3:
			host =	argv[1];
			port =	atoi(argv[2]);	// User specified port
		break;
		case 4:
			host =	argv[1];
			port =	atoi(argv[2]);
			break;
		default:
			fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
			exit(1);
	}
    const char * json2 = "{"
    "\"players\":["
        "{"
                "\"id\":0,"
                "\"x\":0,"
                "\"formatname\":\"mp3\","
                "\"title\":\"Zombie\","
                "\"bitrate\":160000,"
                "\"artist\":\"Cranberries\","
                "\"track_number\":\"01\","
                "\"codecname\":\"mp3\","
				"\"formatname\":\"mp3\","
                "\"title\":\"Zombie\","
                "\"bitrate\":160000,"
                "\"artist\":\"Cranberries\","
                "\"track_number\":\"01\","
                "\"codecname\":\"mp3\""
       " }"
	"]"
	"}";

	// Create the socket
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Cannot create socket");
		exit(1);
	}
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if ((hp = gethostbyname(host)) == NULL)
	{
		fprintf(stderr, "Unknown server address\n");
		exit(1);
	}
	bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

	// Connecting to the server
	if (connect (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		fprintf(stderr, "Can't connect to server\n");
		perror("connect");
		exit(1);
	}
	printf("Connected:    Server Name: %s\n", hp->h_name);
	pptr = hp->h_addr_list;
	printf("\t\tIP Address: %s\n", inet_ntop(hp->h_addrtype, *pptr, str, sizeof(str)));
	printf("Transmit:\n");
	//gets(sbubufferf); // get user's text

	//read udp port
	memset(rbuf, 0, sizeof(rbuf));
	int nread;
	if((nread = recv(sd, rbuf, sizeof(rbuf), 0)) < 0) {
		perror("recv");
		exit(2);
	}
	printf("\n\nRECEIVED PORT NUMBER AND CLIENT ID: %s\n", rbuf);
	char delim[] = ",";
	char *ptr = strtok(rbuf, delim);
	int portNumber = atoi(ptr);
	ptr = strtok(NULL, delim);
	int client_id = atoi(ptr);
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(portNumber);
	server.sin_addr = *((struct in_addr *)hp->h_addr);

	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket == -1) {
        perror("udpSocket creation");
        exit(0);
    }

	Document d;
    d.Parse(json);
   
    // 2. Modify it by DOM.
    Value & players = d["players"];
	players[0]["x"].SetInt(0);
	players[0]["id"].SetInt(client_id);
    // 3. Stringify the DOM
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    d.Accept(writer);
	if(fork() == 0) {
		recvUpdates(udpSocket);
	}
	for(int i = 0; i < SEND_COUNT; i++) {
		players[0]["x"].SetInt(i);
		StringBuffer wbuffer;
		Writer<StringBuffer>writer2(wbuffer);
		d.Accept(writer2);
		strcpy(sbuf,wbuffer.GetString());
		//printf("SENDING: %s\n", sbuf);
		if(sendto (udpSocket, sbuf, BUFLEN, 0,(struct sockaddr *)&server, sizeof(server)) < 0) {
			printf("client %d:",client_id);
			perror("send to\n");
		}
		//printf("client %d sent: %d\n", client_id, i);
		usleep(30000);
	}
	

	bp = rbuf;
	bytes_to_read = BUFLEN;

	// client makes repeated calls to recv until no more data is expected to arrive.
	n = 0;
	//Wait for server response that will never arrive
	//recvfrom(udpSocket, rbuf, sizeof(rbuf), 0, NULL, NULL);
	while(true) {
		//hack to keep thread alive
	}
	printf ("\n\nReceived: %s\n", rbuf);
	fflush(stdout);
	close (sd);
	return (0);
}