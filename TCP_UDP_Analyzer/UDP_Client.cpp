#include "UDP_Client.h"
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: TCP_Client.cpp - This file contain the main function for UDP client to send multiple packets to server.
--
-- PROGRAM: TCP_UDP_Analyzer
--
-- FUNCTIONS:
-- DWORD WINAPI	UDP_client(LPVOID lpParameter);
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- NOTES:
-- UDP client first leans on TCP_client's IP validation function to create socket. 
-- Since it doesn't need to 'connect' to the server, after binding the socket with server information it send the packets
-- as many times as user specified.
----------------------------------------------------------------------------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: UDP_client
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: DWORD WINAPI	UDP_client(LPVOID lpParameter)  // application info struct
--
-- RETURNS: DWORD
--
-- NOTES:
-- It sends the packets as many time as user specified.
-- It generates the timestamp for the first packet sent and the last packet sent and calculate the time taken.
-- It eventually prints the number of total bytes sent, packet size and taken time and close the socket.
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI	UDP_client(LPVOID lpParameter) {
	struct wnd_Info* w = (struct wnd_Info*)lpParameter;
	client_Info* c = &w->c;
	int i = 0;
	unsigned long totalByte = 0;
	WCHAR message[150];
	int client_len;
	vector<char*> sbv;
	char* sbuf;
	
	memset((char*)&c->client, 0, sizeof(c->client));
	c->client.sin_family = AF_INET;
	c->client.sin_port = htons(0);  // bind to any available port
	c->client.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(c->sd, (struct sockaddr*) &c->client, sizeof(c->client)) == -1)
	{
		perror("Can't bind name to socket");
		exit(1);
	}
	client_len = sizeof(c->client);
	if (getsockname(c->sd, (struct sockaddr*) &c->client, &client_len) < 0)
	{
		perror("getsockname: \n");
		exit(1);
	}
	swprintf_s(message, L"Port aasigned is %d\r\n", ntohs(c->client.sin_port));
	SetWindowTextW(w->textHwnd, message);
	GetLocalTime(&c->StartT);
	if (w->dataType == FILE) {
		packetizeFile(w->filePath, &sbv, c->packetSize);
		for (i; i < sbv.size(); i++) {
			c->nSent = sendto(c->sd, sbv.at(i), c->packetSize, 0, (struct sockaddr*) & c->server, sizeof(c->server));
			totalByte += c->nSent;
		}
	}
	if (w->dataType == PACKET) {
		sbuf = (char*)malloc(c->packetSize);
		memset(sbuf, 'k', c->packetSize);
		for (i; i < c->numberToSend; i++) {
			c->nSent = sendto(c->sd, sbuf, c->packetSize, 0, (struct sockaddr*) & c->server, sizeof(c->server));
			totalByte += c->nSent;
		}
	}
	GetLocalTime(&c->EndT);
	long td = 0;
	td = (c->EndT.wSecond - c->StartT.wSecond) * 1000;
	td += (c->EndT.wMilliseconds - c->StartT.wMilliseconds);
	swprintf_s(message, L"Start Time: %hu:%hu:%hu and %hu ms\r\nEnd Time: %hu:%hu:%hu and %hu ms\r\nDelay: %ld Milliseconds\r\nTotal bytes sent: %lu Bytes\r\n Packet size: %lu Bytes",
		c->StartT.wHour, c->StartT.wMinute, c->StartT.wSecond, c->StartT.wMilliseconds,
		c->EndT.wHour, c->EndT.wMinute, c->EndT.wSecond, c->EndT.wMilliseconds, td,
		totalByte, c->packetSize);
	SetWindowTextW(w->textHwnd, message);
	closesocket(c->sd);
	WSACleanup();
	return 0;
}