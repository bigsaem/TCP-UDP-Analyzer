#pragma once
#include "TCP_UDP_Analyzer.h"

DWORD WINAPI	TCP_server(LPVOID wnd_Info);
DWORD WINAPI	WorkerThread(LPVOID lpParameter);
void CALLBACK	WorkerRoutine(DWORD Error, DWORD BytesTransferred, LPWSAOVERLAPPED Overlapped, DWORD InFlags);



