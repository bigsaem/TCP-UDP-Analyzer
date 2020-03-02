#include "UDP_Server.h"
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: UDP_server.cpp - This file contains the functions to set up the receiving socket and
--								 the completion routine to receive the data with overlapped structure
--
-- PROGRAM: TCP_UDP_Analyzer
--
-- FUNCTIONS:
-- DWORD WINAPI UDP_server(LPVOID wnd_Info);
-- void CALLBACK WorkerRoutine_UDP(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- NOTES:
-- The main function, UDP_server is designed to be called on a new thread apart from the main thread for asynchronous receiving.
-- It creates and sets up the socket with UDP specification.
-- It also creates an event for signaling to call the completion routine. 
----------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: UDP_server
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: DWORD WINAPI UDP_server(
--					LPVOID lpParameter //application info struct
--					)
--
-- RETURNS: DWORD
--
-- NOTES:
-- As a main function in UDP server, it creates sockets according to UDP specification.
-- It creates an event and put it in overlapped structure to check overlapped operation completes.
-- It receives the packet and call the completion routinie to handle it.
----------------------------------------------------------------------------------------------------------------------*/

DWORD WINAPI UDP_server(LPVOID lpParameter) {
	struct wnd_Info* w = (struct wnd_Info*)lpParameter;
	struct server_Info_TCP* s = &w->s;
	WCHAR message[150];
	int rc;
	int error;
	if ((s->Ret = WSAStartup(0x0202, &s->wsaData)) != 0)
	{
		printf("WSAStartup failed with error %d\n", s->Ret);
		WSACleanup();
		return 0;
	}
	if ((s->ListenSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, NULL, 0,
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

	LPSOCKET_INFORMATION SocketInfo = &s->socket_Info;
	DWORD Flags;

	if ((SocketInfo = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc() failed with error %d\n", GetLastError());
		return FALSE;
	}
	SocketInfo->Socket = s->ListenSocket;
	ZeroMemory(&(SocketInfo->Overlapped), sizeof(WSAOVERLAPPED));
	SocketInfo->Size = sizeof(SocketInfo->SenderAddr);
	SocketInfo->BytesSEND = 0;
	SocketInfo->BytesRECV = 0;
	SocketInfo->DataBuf.len = s->EPacketSize;
	SocketInfo->DataBuf.buf = (char*)malloc(s->EPacketSize);
	SocketInfo->textHwnd = w->textHwnd;
	SocketInfo->filePath = w->filePath;
	SocketInfo->Overlapped.hEvent = WSACreateEvent();
	if (SocketInfo->Overlapped.hEvent == NULL) {
		wprintf(L"WSACreateEvent failed with error: %d\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	Flags = 0; 
	
	SetWindowTextW(w->textHwnd, L"Socket created");
	while (TRUE)
	{
		if (WSARecvFrom(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SocketInfo->RecvBytes, &Flags,
			(SOCKADDR *)&SocketInfo->SenderAddr, &SocketInfo->Size, &(SocketInfo->Overlapped), WorkerRoutine_UDP) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				swprintf_s(message, L"WSARecvFrom() failed with error %d\r\n", WSAGetLastError());
				SetWindowTextW(w->textHwnd, message);

				return FALSE;
			}
			else {
				rc = WSAWaitForMultipleEvents(1, &SocketInfo->Overlapped.hEvent, TRUE, INFINITE, TRUE);
				WSAResetEvent(SocketInfo->Overlapped.hEvent);
				rc = WSAGetOverlappedResult(SocketInfo->Socket, &SocketInfo->Overlapped, &SocketInfo->RecvBytes,
					FALSE, &Flags);
				if (rc == FALSE) {
					error = WSAGetLastError();
					if (error != WSA_IO_INCOMPLETE) {
						closesocket(SocketInfo->Socket);
						CloseHandle(SocketInfo->Overlapped.hEvent);
						wprintf(L"WSArecvFrom failed with error: %d\n", error);
						return 0;
					}
				}
			}
		}
	}
	return 1;
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WorkerRoutine_UDP
--
-- DATE: Feb 18, 2020
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: void CALLBACK WorkerRoutine_UDP(
--							DWORD Error,				//completion satus indicator
--							DWORD BytesTransferred,		//the number of bytes received
--							LPWSAOVERLAPPED Overlapped, //overlapped structure pointer
--							DWORD InFlags				//information if the receive operation had completed immediately
--							)
--
-- RETURNS: DWORD
--
-- NOTES:
-- This is the completion routine that handles incoming packets in asynchronous way.
-- Every time this function is called, it increments the number of received packets,
-- and adding up the number of received bytes. It also make timestamp for the first and last bytes
-- to calculate the time taken to receive all the packets. It also stores data into a file if the user
-- chose to do so.
----------------------------------------------------------------------------------------------------------------------*/

void CALLBACK WorkerRoutine_UDP(DWORD Error, DWORD BytesTransferred,
	LPWSAOVERLAPPED Overlapped, DWORD InFlags)
{
	DWORD Flags;
	WCHAR Analysis[300];
	// Reference the WSAOVERLAPPED structure as a SOCKET_INFORMATION structure
	LPSOCKET_INFORMATION SI = (LPSOCKET_INFORMATION)Overlapped;

	if (SI->NumofPacket == 0) {
		GetSystemTime(&SI->StartT);
	}
	GetLocalTime(&SI->EndT);
	if (BytesTransferred > 0) {
		SI->NumofPacket++;
		SI->PacketSize = BytesTransferred;
		if (SI->filePath != NULL) {
			ofstream file;
			file.open(SI->filePath, std::ofstream::out | std::ofstream::app);
			char* buf = (char*)malloc(SI->PacketSize+1);
			int size = sizeof(buf);
			strncpy_s(buf, SI->PacketSize+1, SI->DataBuf.buf, SI->DataBuf.len);
			file << buf;
			file.close();
		}
	}
	if (Error != 0)
	{
		printf("I/O operation failed with error %d\n", Error);
	}

	//if (Error != 0 || BytesTransferred == 0)
	//{
	//	long td;
	//	td = (SI->EndT.wSecond - SI->StartT.wSecond) * 1000;
	//	td += (SI->EndT.wMilliseconds - SI->StartT.wMilliseconds);
	//	wsprintfW(Analysis,
	//		L"Start Time: %hu:%hu:%hu and %hu ms\r\nEnd Time: %hu:%hu:%hu and %hu ms\r\nDelay: %ld Miliseconds\r\nReceived bytes: %d Bytes\r\nPacket size: %d Bytes\r\nThe number of packets: %d",
	//		SI->StartT.wHour, SI->StartT.wMinute, SI->StartT.wSecond, SI->StartT.wMilliseconds,
	//		SI->EndT.wHour, SI->EndT.wMinute, SI->EndT.wSecond, SI->EndT.wMilliseconds, td,
	//		SI->BytesRECV, SI->PacketSize, SI->NumofPacket);
	//	SetWindowTextW(SI->textHwnd, Analysis);
	//	closesocket(SI->Socket);
	//	GlobalFree(SI);
	//	return;
	//}

	SI->BytesRECV += BytesTransferred;
	
	long td;
	td = (SI->EndT.wSecond - SI->StartT.wSecond) * 1000;
	td += (SI->EndT.wMilliseconds - SI->StartT.wMilliseconds);
	wsprintfW(Analysis,
		L"Start Time: %hu:%hu:%hu and %hu ms\r\nEnd Time: %hu:%hu:%hu and %hu ms\r\nDelay: %ld Miliseconds\r\nReceived bytes: %d Bytes\r\nPacket size: %d Bytes\r\nThe number of packets: %d",
		SI->StartT.wHour, SI->StartT.wMinute, SI->StartT.wSecond, SI->StartT.wMilliseconds,
		SI->EndT.wHour, SI->EndT.wMinute, SI->EndT.wSecond, SI->EndT.wMilliseconds, td,
		SI->BytesRECV, SI->PacketSize, SI->NumofPacket);
	SetWindowTextW(SI->textHwnd, Analysis);
	Flags = 0;
	//ZeroMemory(&(SI->Overlapped), sizeof(WSAOVERLAPPED));
	WSARecvFrom(SI->Socket, &(SI->DataBuf), 1, &SI->RecvBytes, &Flags,
		(SOCKADDR*)&SI->SenderAddr, &SI->Size, &(SI->Overlapped), WorkerRoutine_UDP);
	if (WSAGetLastError() != WSA_IO_PENDING)
	{
		printf("WSARecv() failed with error %d\n", WSAGetLastError());
		return;
	}
}