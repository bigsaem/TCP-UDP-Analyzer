#include "TCP_Server.h"
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: TCP_server.cpp - This file contains the functions to set up listen and accept socket and 
--								 the completion routine to receive the data with overlapped structure
--
-- PROGRAM: TCP_UDP_Analyzer
--
-- FUNCTIONS:
-- DWORD WINAPI		TCP_server(LPVOID wnd_Info);
-- DWORD WINAPI		WorkerThread(LPVOID lpParameter);
-- void CALLBACK	WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- NOTES:
-- The main function, TCP_server is designed to be called on a new thread apart from the main thread for async receiving.
-- It sets up WSA settings with TCP specification and create another thread for completion routine with overlapped struct
-- It creates listen socket first and creates accept socket when a client requested to connect.
----------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: TCP_server
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: DWORD WINAPI TCP_server(
--			  LPVOID lpParameter //application info struct
--			  )
--
-- RETURNS: DWORD
--
-- NOTES:
-- As a main function in TCP server, it creates sockets according to TCP specification.
-- It creates a new thread for actual receiving with overlapped struct in completion routine.
-- It creates an accept event and signals it when a client requested to connect
----------------------------------------------------------------------------------------------------------------------*/

DWORD WINAPI TCP_server(LPVOID lpParameter) {
	struct wnd_Info* w = (struct wnd_Info*)lpParameter;
	struct server_Info_TCP* s = &w->s;
	if ((s->Ret = WSAStartup(0x0202, &s->wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", s->Ret);
		WSACleanup();
		return 0;
	}

	if ((s->ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("Failed to get a socket %d\n", WSAGetLastError());
		return 0;
	}
	s->InternetAddr.sin_family = AF_INET;
	s->InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	s->InternetAddr.sin_port = htons(w->port);
	if (bind(s->ListenSocket, (PSOCKADDR)&s->InternetAddr,
		sizeof(s->InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind() failed with error %d\n", WSAGetLastError());
		return 0;
	}
	if (listen(s->ListenSocket, 5))
	{
		printf("listen() failed with error %d\n", WSAGetLastError());
		return 0;
	}

	if ((s->AcceptEvent = WSACreateEvent()) == WSA_INVALID_EVENT)
	{
		printf("WSACreateEvent() failed with error %d\n", WSAGetLastError());
		return 0;
	}

	// Create a worker thread to service completed I/O requests. 

	if ((s->ThreadHandle = CreateThread(NULL, 0, WorkerThread, (LPVOID)w, 0, &s->ThreadId)) == NULL)
	{
		printf("CreateThread failed with error %d\n", GetLastError());
		return 0;
	}
	SetWindowTextW(w->textHwnd, L"Socket created and thread made");
	while (TRUE)
	{
		s->AcceptSocket = accept(s->ListenSocket, NULL, NULL);

		if (WSASetEvent(s->AcceptEvent) == FALSE)
		{
			printf("WSASetEvent failed with error %d\n", WSAGetLastError());
			return 0;
		}
	}
	return 1;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WorkerThread
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: DWORD WINAPI WorkerThread(
--				LPVOID lpParameter //application info structure
--				)
--
-- RETURNS: DWORD
--
-- NOTES:
-- This is also a thread function that waits for accept event. 
-- If it noticed the accept event signaled, it triggers overlapped operation with completion routine
----------------------------------------------------------------------------------------------------------------------*/
DWORD WINAPI WorkerThread(LPVOID lpParameter)
{
	struct wnd_Info* w = (struct wnd_Info*)lpParameter;
	struct server_Info_TCP* s = (struct server_Info_TCP*) & w->s;
	DWORD Flags;
	LPSOCKET_INFORMATION SocketInfo = &s->socket_Info;
	WSAEVENT EventArray[1];
	DWORD Index;
	DWORD RecvBytes;
	WCHAR message[50];
	memset(message, 0, sizeof(message));
	int numofP = 0;
	// Save the accept event in the event array.

	EventArray[0] = (WSAEVENT)s->AcceptEvent;

	while (TRUE)
	{
		// Wait for accept() to signal an event and also process WorkerRoutine() returns.

		while (TRUE)
		{
			Index = WSAWaitForMultipleEvents(1, EventArray, FALSE, WSA_INFINITE, TRUE);

			if (Index == WSA_WAIT_FAILED)
			{
				wsprintfW(message, L"WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
				SetWindowTextW(w->textHwnd, message);
				printf("WSAWaitForMultipleEvents failed with error %d\n", WSAGetLastError());
				return FALSE;
			}

			if (Index != WAIT_IO_COMPLETION)
			{
				// An accept() call event is ready - break the wait loop
				break;
			}
		}

		WSAResetEvent(EventArray[Index - WSA_WAIT_EVENT_0]);

		// Create a socket information structure to associate with the accepted socket.

		if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
			sizeof(SOCKET_INFORMATION))) == NULL)
		{
			printf("GlobalAlloc() failed with error %d\n", GetLastError());
			return FALSE;
		}
	
		// Fill in the details of our accepted socket.

		SocketInfo->Socket = s->AcceptSocket;
		ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
		SocketInfo->BytesSEND = 0;
		SocketInfo->BytesRECV = 0;
		SocketInfo->DataBuf.len = s->EPacketSize;
		SocketInfo->DataBuf.buf = (CHAR *)malloc(s->EPacketSize);
		SocketInfo->textHwnd = w->textHwnd;
		SocketInfo->filePath = w->filePath;
		Flags = 0;
		if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,
			&(SocketInfo->Overlapped), WorkerRoutine) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				printf("WSARecv() failed with error %d\n", WSAGetLastError());
				return FALSE;
			}
		}
		wsprintfW(message, L"socket %d connected %d", s->AcceptSocket, SocketInfo->NumofPacket);
		SetWindowTextW(w->textHwnd, message);
	}

	return TRUE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WorkerRoutine
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: void CALLBACK WorkerRoutine(
--			DWORD Error,					//completion satus indicator
--			DWORD BytesTransferred,			//the number of bytes received
--			LPWSAOVERLAPPED Overlapped,		//overlapped structure pointer
--			DWORD InFlags					//information if the receive operation had completed immediately
--			)
--
-- RETURNS: void
--
-- NOTES:
-- This is the completion routine that handles incoming packets in asynchronous way.
-- Every time this function is called, it increments the number of received packets,
-- and adding up the number of received bytes. It also make timestamp for the first and last bytes
-- to calculate the time taken to receive all the packets. It also stores data into a file if the user
-- chose to do so.
----------------------------------------------------------------------------------------------------------------------*/
void CALLBACK WorkerRoutine(DWORD Error, DWORD BytesTransferred,
	LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	DWORD RecvBytes;
	DWORD Flags;
	WCHAR Analysis[500];
	
	// Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;
	
	if (SI->NumofPacket == 0) {
		GetLocalTime(&SI->StartT);
	}
	GetLocalTime(&SI->EndT);
	if (BytesTransferred > 0) {
		if (BytesTransferred > SI->PacketSize) {
			SI->PacketSize = BytesTransferred;
		}
		if (SI->filePath != NULL) {
			ofstream file;
			file.open(SI->filePath, std::ofstream::out | std::ofstream::app);
			char* buf = (char *)malloc(SI->PacketSize+1);
			int size = sizeof(buf);
			strncpy_s(buf, SI->PacketSize+1, SI->DataBuf.buf, SI->DataBuf.len);
			file << buf;
			file.close();
		}
		SI->NumofPacket++;
	}
	if (Error != 0)
	{
		printf("I/O operation failed with error %d\n", Error);
	}

	if (Error != 0 || BytesTransferred == 0)
	{
		//calculate delay
		long td;
		td = (SI->EndT.wSecond - SI->StartT.wSecond) * 1000;
		td += (SI->EndT.wMilliseconds - SI->StartT.wMilliseconds);
		wsprintfW(Analysis,
			L"Start Time: %hu:%hu:%hu and %hu ms\r\nEnd Time: %hu:%hu:%hu and %hu ms\r\nDelay: %ld Miliseconds\r\nReceived bytes: %d Bytes\r\nPacket size: %d Bytes\r\nThe number of packets: %d",
			SI->StartT.wHour, SI->StartT.wMinute, SI->StartT.wSecond, SI->StartT.wMilliseconds,
			SI->EndT.wHour, SI->EndT.wMinute, SI->EndT.wSecond, SI->EndT.wMilliseconds, td, 
			SI->BytesRECV, SI->PacketSize, SI->NumofPacket);
		SetWindowTextW(SI->textHwnd, Analysis);
		closesocket(SI->Socket);
		GlobalFree(SI);
		return;
	}

	SI->BytesRECV += BytesTransferred;

	Flags = 0;
	ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));

	if (WSARecv(SI->Socket, &(SI->DataBuf), 1, &RecvBytes, &Flags,
		&(SI->Overlapped), WorkerRoutine) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			printf("WSARecv() failed with error %d\n", WSAGetLastError());
			return;
		}
	}

}