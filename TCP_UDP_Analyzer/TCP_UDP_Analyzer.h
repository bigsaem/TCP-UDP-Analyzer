#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN         
#include <windows.h>
#include "resource.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32.lib")
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>
#include <string>
#include <commdlg.h>
#include <ctime>
#include <iomanip>
#include <vector>
#include <fstream>
#include "UDP_Server.h"
#include "UDP_Client.h"
#include "TCP_Server.h"
#include "TCP_Client.h"
#include "File_Converter.h"

#define IDLE			0
#define TCP				1
#define UDP				2
#define FILE			3
#define PACKET			4
#define DATA_BUFSIZE	8192
#define MAX_LOADSTRING	100
#define DEFAULT_PORT	5150
#define KB				1024

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
BOOL				addFile(OPENFILENAME& ofn, HWND hdl, int role);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ProtocolandIP(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK	DataConfiguration(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


typedef struct _SOCKET_INFORMATION {
	OVERLAPPED			Overlapped;
	SOCKET				Socket;
	std::vector<char*>	Buffer;
	const WCHAR*		filePath;
	WSABUF				DataBuf;
	DWORD				BytesSEND;
	DWORD				BytesRECV;
	DWORD				RecvBytes;
	HWND				textHwnd;
	int					NumofPacket;
	SOCKADDR_IN			SenderAddr;
	INT					Size;
	SYSTEMTIME			StartT;
	SYSTEMTIME			EndT;
	unsigned long		PacketSize;
} SOCKET_INFORMATION, * LPSOCKET_INFORMATION;

typedef struct server_Info_TCP {
	INT					EPackets;
	INT					EPacketSize;
	INT					Ret;
	WSADATA				wsaData;
	SOCKET				AcceptSocket;
	SOCKET				ListenSocket;
	SOCKADDR_IN			InternetAddr;
	HANDLE				ThreadHandle;
	DWORD				ThreadId;
	WSAEVENT			AcceptEvent;
	SOCKET_INFORMATION	socket_Info;
}server_Info;

typedef struct client_Info {
	INT					nSent;
	INT					err;
	INT					server_port;
	INT					numberToSend;
	DWORD				packetSize;
	WCHAR				server_add[30];
	struct hostent*		hp;
	struct sockaddr_in	server;
	struct sockaddr_in	client;
	SOCKET				sd;
	WORD				wVersionRequested;
	WSADATA				WSAData;
	SYSTEMTIME			StartT;
	SYSTEMTIME			EndT;
}client_Info;

struct wnd_Info{
	INT					protocol;
	INT					port;
	INT					role = IDLE;
	INT					dataType = PACKET;
	HMENU				hMenu;
	DWORD				ThreadId;
	HANDLE				ThreadHandle;
	WCHAR				szFile[260];
	const WCHAR*		filePath;
	HWND				hWnd;
	HWND				textHwnd;									// text window handle
	WCHAR				szTitle[MAX_LOADSTRING];                  // The title bar text
	WCHAR				szWindowClass[MAX_LOADSTRING];            // the main window class name
	OPENFILENAME		ofn;
	HINSTANCE			hInst;                                // current instance
	server_Info			s;
	client_Info			c;
};



