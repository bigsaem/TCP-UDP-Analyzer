#pragma once
#include "TCP_UDP_Analyzer.h"


DWORD WINAPI UDP_server(LPVOID wnd_Info);
void CALLBACK WorkerRoutine_UDP(DWORD Error, DWORD BytesTransferred,
	LPWSAOVERLAPPED Overlapped, DWORD InFlags);