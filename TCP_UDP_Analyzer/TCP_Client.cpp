#include "TCP_Client.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: TCP_Client.cpp - This file contains the functions to check the validity of host address, socket set up
--								, connect to server and transmit data.
--
-- PROGRAM: TCP_UDP_Analyzer
--
-- FUNCTIONS:
-- DWORD WINAPI		TCP_client(LPVOID lpParameter);
-- int				IP_validation(struct wnd_Info* wnd_Info);
-- int				TCP_Client_Connect(struct wnd_Info* wnd_Info);
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- NOTES:
-- This file provides the function to validate the host IP address and store the server info.
-- It connects to the server witt that information and eventually send the data.
----------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: IP_validation
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: int IP_validation(struct wnd_Info* wnd_Info) // application info struct
--
-- RETURNS: int
--
-- NOTES:
-- Depending on the protocol of the application it differently create a socket and check if the host address is valid
-- If it is valid, it stores that information in client structure.
----------------------------------------------------------------------------------------------------------------------*/

int IP_validation(struct wnd_Info* wnd_Info) {
	client_Info* c = &wnd_Info->c;
	c->wVersionRequested = MAKEWORD(2, 2);
	c->err = WSAStartup(c->wVersionRequested, &c->WSAData);
	if (c->err != 0) //No usable DLL
	{
		return 0;
	}
	// Create the socket
	if (wnd_Info->protocol == TCP) {
		if ((c->sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		{
			perror("Cannot create socket");
			return 0;
		}
	}
	if (wnd_Info->protocol == UDP) {
		if ((c->sd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		{
			perror("Cannot create socket");
			return 0;
		}
	}

	memset((char*)&c->server, 0, sizeof(struct sockaddr_in));
	c->server.sin_family = AF_INET;
	c->server.sin_port = htons(c->server_port);
	char* add = (char *)malloc(150);
	size_t i;
	wcstombs_s(&i, add, (size_t)150, c->server_add, (size_t)sizeof(c->server_add));
	if ((c->hp = gethostbyname(add)) == NULL)
	{
		return 0;
	}
	memcpy((char*)&c->server.sin_addr, c->hp->h_addr, c->hp->h_length);
	return 1;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCP_Client_Connect
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: int TCP_Client_Connect(struct wnd_Info* wnd_Info) // application info struct
--
-- RETURNS: int
--
-- NOTES:
-- With the client struct that contains the server address information it sends a connect request to server
----------------------------------------------------------------------------------------------------------------------*/

int TCP_Client_Connect(struct wnd_Info* wnd_Info) {
	client_Info* c = &wnd_Info->c;
	// Connecting to the server
	if (connect(c->sd, (struct sockaddr*) & c->server, sizeof(c->server)) == -1)
	{
		return 0;
	}
	return 1;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCP_client
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: DWORD WINAPI TCP_client(LPVOID lpParameter) // application info struct
--
-- RETURNS: DWORD
--
-- NOTES:
-- It sends the packets as many time as user specified.
-- It generates the timestamp for the first packet sent and the last packet sent and calculate the time taken.
-- It eventually prints the number of total bytes sent, packet size and taken time and close the socket.
----------------------------------------------------------------------------------------------------------------------*/

DWORD WINAPI TCP_client(LPVOID lpParameter) {
	struct wnd_Info* w = (struct wnd_Info*)lpParameter;
	client_Info* c = &w->c;
	WCHAR message[250];
	char* sbuf;
	vector<char*> sbv;
	int i = 0;
	unsigned long totalByte = 0;
	GetLocalTime(&c->StartT);
	if (w->dataType == FILE) {
		packetizeFile(w->filePath, &sbv, c->packetSize);
		for (i; i < sbv.size(); i++) {
			/*size_t convertedWchars;
			wcstombs_s(&convertedWchars, fbuf, (size_t)c->packetSize, sbv.at(i), (size_t)c->packetSize);*/
			c->nSent = send(c->sd, sbv.at(i), c->packetSize, 0);
			totalByte += c->nSent;
		}
	} 
	if (w->dataType == PACKET) {
		sbuf = (char*)malloc(c->packetSize);
		memset(sbuf, 'k', c->packetSize);
		for (i; i < (int)c->numberToSend; i++) {
			c->nSent = send(c->sd, sbuf, c->packetSize, 0);
			totalByte += c->nSent;
		}
	}
	GetLocalTime(&c->EndT);
	long td=0;
	td = (c->EndT.wSecond - c->StartT.wSecond) * 1000;
	td += (c->EndT.wMilliseconds - c->StartT.wMilliseconds);
	swprintf_s(message, L"Start Time: %hu:%hu:%hu and %hu ms\r\nEnd Time: %hu:%hu:%hu and %hu ms\r\nDelay: %ld Milliseconds\r\nTotal bytes sent: %lu Bytes\r\nPacket size: %lu Bytes\r\nNum of Packets: %d",
		c->StartT.wHour, c->StartT.wMinute, c->StartT.wSecond, c->StartT.wMilliseconds,
		c->EndT.wHour, c->EndT.wMinute, c->EndT.wSecond, c->EndT.wMilliseconds, td,
		totalByte, c->packetSize, i);
	SetWindowTextW(w->textHwnd, message);
	closesocket(c->sd);
	WSACleanup();
	return 0;
}