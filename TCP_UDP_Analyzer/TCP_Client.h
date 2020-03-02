#pragma once
#include "TCP_UDP_Analyzer.h"

DWORD WINAPI		TCP_client(LPVOID lpParameter);
int					IP_validation(struct wnd_Info* wnd_Info);
int					TCP_Client_Connect(struct wnd_Info* wnd_Info);