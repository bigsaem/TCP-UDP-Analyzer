#include "TCP_UDP_Analyzer.h"
/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: TCP_UDP_Analyzer.cpp - An application that can transform into a server and client to receive and 
-- send data with TCP and UDP to analyze the characteristics of each protocol
--
-- PROGRAM: TCP_UDP_Analyzer
--
-- FUNCTIONS:
-- ATOM                 MyRegisterClass(HINSTANCE hInstance);
-- BOOL                 InitInstance(HINSTANCE, int);
-- BOOL					addFile(OPENFILENAME& ofn, HWND hdl, int role);
-- LRESULT CALLBACK     WndProc(HWND, UINT, WPARAM, LPARAM);
-- INT_PTR CALLBACK     About(HWND, UINT, WPARAM, LPARAM);
-- INT_PTR CALLBACK		ProtocolandIP(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
-- INT_PTR CALLBACK		DataConfiguration(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
--
--
-- DATE: Feb 18, 2020
--
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- NOTES:
-- The program can be a server or a client as a tool to analyze the characteristics of TCP and UDP
-- As a server, this program can receive data from client and print the time taken to receive the all packet, 
-- the amount of data and the size of the packet usign completion routine.
-- As a client, this program can send data according to the specification that user configured, and print the time taken
-- to send all the packets, the number of bytes and packets sent.
----------------------------------------------------------------------------------------------------------------------*/


//application struct that has shared info
struct wnd_Info wnd_Info;

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: wWinMain
--
-- DATE: Feb 18, 2020
--
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: int APIENTRY wWinMain(
--			  _In_ HINSTANCE hInstance,           //handle for the current instance of the application
--            _In_opt_ HINSTANCE hPrevInstance,   //handle to the precious instance of the application
--            _In_ LPWSTR    lpCmdLine,			  //command line for the application
--            _In_ int       nCmdShow			  
--			  )
--
-- RETURNS: int
--
-- NOTES:
-- As a driver function, it runs the application and display the application window. 
-- It also runs the message loop and dispatch it to message processor
----------------------------------------------------------------------------------------------------------------------*/

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, wnd_Info.szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TCPUDPANALYZER, wnd_Info.szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TCPUDPANALYZER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: MyRegisterClass
--
-- DATE: Feb 18, 2020
--
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: ATOM MyRegisterClass(HINSTANCE hInstance)
--
-- RETURNS: ATOM //system-defined table
--
-- NOTES:
-- This function sets the attributes of the window class and register it to be used
----------------------------------------------------------------------------------------------------------------------*/

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;
	wcex.cbSize			= sizeof(WNDCLASSEXW);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TCPUDPANALYZER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TCPUDPANALYZER);
    wcex.lpszClassName  = wnd_Info.szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}


/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: InitInstance
--
-- DATE: Feb 18, 2020
--
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
--
-- RETURNS: BOOL
--
-- NOTES:
-- This function creates the window with specified attributes and display it
----------------------------------------------------------------------------------------------------------------------*/

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   wnd_Info.hInst = hInstance; // Store instance handle in our global variable
   WCHAR title[20] = L"TCP UDP Analyzer";
   wnd_Info.hWnd = CreateWindowW(wnd_Info.szWindowClass, title, WS_OVERLAPPEDWINDOW,
	   0, 20, 700, 370, nullptr, nullptr, hInstance, nullptr);

   if (!wnd_Info.hWnd)
   {
      return FALSE;
   }
   wnd_Info.textHwnd = CreateWindowW(L"EDIT", L"", WS_VISIBLE | WS_CHILD | SS_LEFT | ES_MULTILINE | ES_READONLY,
	   0, 0, 685, 370, wnd_Info.hWnd, NULL, wnd_Info.hInst, NULL);
   SetWindowTextW(wnd_Info.textHwnd, L"Outcome will be displayed here");
   ShowWindow(wnd_Info.hWnd, nCmdShow);
   UpdateWindow(wnd_Info.hWnd);
   return TRUE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: WndProc
--
-- DATE: Feb 18, 2020
--
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: LRESULT CALLBACK WndProc(
--			  HWND hWnd,			//wndow handle
--			  UINT message,			//window message
--			  WPARAM wParam,		//message info
--			  LPARAM lParam			//message info
--			  )
--
-- RETURNS: LRESULT
--
-- NOTES:
-- This function handles dispatched window messages such as clicking menu items
----------------------------------------------------------------------------------------------------------------------*/

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case CLIENT:
				wnd_Info.role = CLIENT;
				DialogBox(wnd_Info.hInst, MAKEINTRESOURCE(IDD_PROTOCOL_IP_PROMPT), hWnd, ProtocolandIP);
                break;
			case SERVER:
				wnd_Info.role = SERVER;
				DialogBox(wnd_Info.hInst, MAKEINTRESOURCE(IDD_PROTOCOL_IP_PROMPT), hWnd, ProtocolandIP);
				break;
			case ID_CLIENTMENU_CONNECT:
			{
				WCHAR connect_message[150];
				if (TCP_Client_Connect(&wnd_Info)) {
					size_t convertedChars;
					WCHAR serverName[100];
					WCHAR serverAdd[100];
					mbstowcs_s(&convertedChars, serverName, 100, wnd_Info.c.hp->h_name, 100);
					mbstowcs_s(&convertedChars, serverAdd, 100, inet_ntoa(wnd_Info.c.server.sin_addr), 100);
					wsprintfW(connect_message, L"Connected\r\nServer Name: %s\r\nIP Address: %s\r\nChoose the option for sending a file or packets",
						serverName, serverAdd);
					SetWindowText(wnd_Info.textHwnd, connect_message);
					EnableMenuItem(wnd_Info.hMenu, ID_TRANSMITDATA_PACKETS, MF_ENABLED);
					EnableMenuItem(wnd_Info.hMenu, ID_TRANSMITDATA_FILE, MF_ENABLED);
				}
				else {
					SetWindowTextW(wnd_Info.textHwnd, L"Connect failed");
				}
				break;
			}
			case ID_TRANSMITDATA_PACKETS:
				DialogBox(wnd_Info.hInst, MAKEINTRESOURCE(IDD_DATA_CONFIGURATION), hWnd, DataConfiguration);
				break;
			case ID_SERVERMENU_DATACONFIGURATION:
				DialogBox(wnd_Info.hInst, MAKEINTRESOURCE(IDD_DATA_CONFIGURATION), hWnd, DataConfiguration);
				break;
			case ID_SERVERMENU_RUNSERVER:
			{
				if (wnd_Info.protocol == TCP) {
					if ((wnd_Info.ThreadHandle = CreateThread(NULL, 0, TCP_server,
						(LPVOID)&wnd_Info, 0, &wnd_Info.ThreadId)) == NULL)
					{
						printf("CreateThread failed with error %d\n", GetLastError());
						return 0;
					}
				} 
				if (wnd_Info.protocol == UDP) {
					if ((wnd_Info.ThreadHandle = CreateThread(NULL, 0, UDP_server,
						(LPVOID)&wnd_Info, 0, &wnd_Info.ThreadId)) == NULL)
					{
						printf("CreateThread failed with error %d\n", GetLastError());
						return 0;
					}
				}
				break;
			}
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: ProtocolandIP
--
-- DATE: Feb 18, 2020
--
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE:	INT_PTR CALLBACK ProtocolandIP(
--				 HWND hDlg,		//handle for dialog box
--				 UINT message,	//window message
--				 WPARAM wParam, 
--				 LPARAM lParam
--				 )
--
-- RETURNS: INT_PTR
--
-- NOTES:
-- This function prompts protocol and IP in a dialog box. 
-- If the role of the program is server, it asks port number and protocol to use
-- If the role of the program is client, it asks host information as well as its own port number and protocol
----------------------------------------------------------------------------------------------------------------------*/

INT_PTR CALLBACK ProtocolandIP(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG: //fill the combo box with content
	{
		HWND comboBox = GetDlgItem(hDlg, IDC_COMBO_PROTOCOL);
		WCHAR protocols[2][4] = {L"TCP", L"UDP"};
		WCHAR options[6];
		memset(&options, 0, sizeof(options));
		for (int i = 0; i < 2; i++) {
			wcscpy_s(options, protocols[i]);
			SendMessage(comboBox, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)options);
		}
		SendMessage(comboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		SetWindowTextW(GetDlgItem(hDlg, IDC_PORT), L"5150"); //default port number
		SetWindowTextW(GetDlgItem(hDlg, IDC_PORT_DESTINATION), L"5150");
		if (wnd_Info.role == SERVER) {
			HWND host_prompt = GetDlgItem(hDlg, IDC_HOST_PROMPT);
			HWND host_edit = GetDlgItem(hDlg, IDC_IP_HOST);
			HWND host_prompt_port = GetDlgItem(hDlg, IDC_HOST_PORT);
			HWND host_port = GetDlgItem(hDlg, IDC_PORT_DESTINATION);
			ShowWindow(host_prompt, SW_HIDE);
			ShowWindow(host_edit, SW_HIDE);
			ShowWindow(host_prompt_port, SW_HIDE);
			ShowWindow(host_port, SW_HIDE);
		}
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			bool ipvalid = true;
			wnd_Info.hMenu = GetMenu(wnd_Info.hWnd);
			WCHAR protocol[4];
			WCHAR port[6];
			GetDlgItemTextW(hDlg, IDC_COMBO_PROTOCOL, protocol, 4);
			GetDlgItemTextW(hDlg, IDC_PORT, port, 6);
			wnd_Info.protocol = (wcscmp(protocol, L"TCP")==0? TCP : UDP);
			wnd_Info.port = (_wtoi(port) == 0 || _wtoi(port) == NULL? 5150 : _wtoi(port));
			//server
			if (wnd_Info.role == SERVER) {
				EnableMenuItem(wnd_Info.hMenu, 0, MF_BYPOSITION | MF_DISABLED);
				EnableMenuItem(wnd_Info.hMenu, 3, MF_BYPOSITION | MF_ENABLED);
			}
			//client
			else if (wnd_Info.role == CLIENT) {
				WCHAR dest_port[6];
				GetDlgItemTextW(hDlg, IDC_IP_HOST, wnd_Info.c.server_add, 30);
				GetDlgItemTextW(hDlg, IDC_PORT_DESTINATION, dest_port, 6);
				wnd_Info.c.server_port = (_wtoi(dest_port) == 0 || _wtoi(dest_port) == NULL ? 5150 : _wtoi(dest_port));
				if (!(ipvalid = IP_validation(&wnd_Info))) {
					ShowWindow(GetDlgItem(hDlg, IDC_IP_WARNING), SW_SHOW);
					ipvalid = false;
				}
				if (ipvalid) {
					SetWindowTextW(wnd_Info.textHwnd, L"Click Client Menu to transmit data");
					EnableMenuItem(wnd_Info.hMenu, 1, MF_BYPOSITION | MF_DISABLED);
					EnableMenuItem(wnd_Info.hMenu, 2, MF_BYPOSITION | MF_ENABLED);
					if (wnd_Info.protocol == UDP) {
						EnableMenuItem(wnd_Info.hMenu, ID_CLIENTMENU_CONNECT, MF_DISABLED);
						EnableMenuItem(wnd_Info.hMenu, ID_TRANSMITDATA_PACKETS, MF_ENABLED);
						EnableMenuItem(wnd_Info.hMenu, ID_TRANSMITDATA_FILE, MF_ENABLED);
					}
				}
			}
			if (ipvalid) { //it doesn't close the dialog box if ip is not valid
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
		}

		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DataConfiguration
--
-- DATE: Feb 18, 2020
--
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: INT_PTR CALLBACK DataConfiguration(
--			  HWND hDlg,		
--			  UINT message, 
--			  WPARAM wParam, 
--			  LPARAM lParam
--			  )
--
-- RETURNS: INT_PTR
--
-- NOTES:
-- This function prompts data specification, such as data type, packet size and the number to send the packets
-- If the role of the program is server, it asks expected packetsize and number of the packets.
-- It also prompts where to store the received data as an option.
-- If the role of the program is client, it asks whether to generate pseudo packets or send a text file. 
-- It also prompts the packet size and how many times to send it
----------------------------------------------------------------------------------------------------------------------*/

INT_PTR CALLBACK DataConfiguration(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:
	{
		HWND comboBox_packet = GetDlgItem(hDlg, IDC_COMBO_PACKET);
		HWND comboBox_time = GetDlgItem(hDlg, IDC_PACKET_TIME);
		TCHAR protocols[6][20] = 
		{ TEXT("1 KBytes"), TEXT("4 KBytes"),
			TEXT("20 KBytes"), TEXT("60 KBytes"),
			TEXT("100 KBytes"), TEXT("140 KBytes")};
		for (int i = 0; i < 6; i++) {
			SendMessage(comboBox_packet, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)protocols[i]);
		}
		TCHAR times[3][10] =
		{ TEXT("10"), TEXT("100"), TEXT("1000")};
		for (int i = 0; i < 3; i++) {
			SendMessage(comboBox_time, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)times[i]);
		}
		CheckDlgButton(hDlg, IDC_RADIO_PACKET, true);
		SendMessage(comboBox_packet, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		SendMessage(comboBox_time, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_RADIO_PACKET || LOWORD(wParam) == IDC_RADIO_FILE) {
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_PACKET)) {
				wnd_Info.dataType = PACKET;
				EnableWindow(GetDlgItem(hDlg, IDC_FILE_PATH), false);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_FILE), false);
			}
			if (IsDlgButtonChecked(hDlg, IDC_RADIO_FILE)) {
				wnd_Info.dataType = FILE;
				EnableWindow(GetDlgItem(hDlg, IDC_FILE_PATH), true);
				EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_FILE), true);
			}
		}
		if (LOWORD(wParam) == IDC_BUTTON_FILE) {
			OPENFILENAME ofn;
			if (addFile(ofn, hDlg, wnd_Info.role)) {
				SetDlgItemTextW(hDlg, IDC_FILE_PATH, ofn.lpstrFile);
				wnd_Info.filePath = ofn.lpstrFile;
			}
		}
		if (LOWORD(wParam) == IDOK)
		{
			int tempSize;
			WCHAR packetsize[20];
			WCHAR numberToSend[5];
			GetDlgItemTextW(hDlg, IDC_COMBO_PACKET, packetsize, 20);
			GetDlgItemText(hDlg, IDC_PACKET_TIME, numberToSend, 5);
			if (wnd_Info.role == CLIENT) {
				swscanf_s(packetsize, L"%d", &tempSize);
				swscanf_s(numberToSend, L"%d", &wnd_Info.c.numberToSend);
				wnd_Info.c.packetSize = tempSize * KB;
				int a = wnd_Info.c.numberToSend;
				if (wnd_Info.protocol == TCP) {
					if ((wnd_Info.ThreadHandle = CreateThread(NULL, 0, TCP_client,
						(LPVOID)&wnd_Info, 0, &wnd_Info.ThreadId)) == NULL)
					{
						printf("CreateThread failed with error %d\n", GetLastError());
						return 0;
					}
				}
				if (wnd_Info.protocol == UDP) {
					if ((wnd_Info.ThreadHandle = CreateThread(NULL, 0, UDP_client,
						(LPVOID)&wnd_Info, 0, &wnd_Info.ThreadId)) == NULL)
					{
						printf("CreateThread failed with error %d\n", GetLastError());
						return 0;
					}
				}
			} 
			if (wnd_Info.role == SERVER) {
				int tempSize;
				swscanf_s(packetsize, L"%d", &tempSize);
				swscanf_s(numberToSend, L"%d", &wnd_Info.s.EPackets);
				wnd_Info.s.EPacketSize = tempSize * KB;
				EnableMenuItem(wnd_Info.hMenu, ID_SERVERMENU_RUNSERVER, MF_ENABLED);
			}
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}

		if (LOWORD(wParam) == IDCANCEL){
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: DataConfiguration
--
-- DATE: Feb 18, 2020
--
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: INT_PTR CALLBACK DataConfiguration(
--			  HWND hDlg,
--			  UINT message,
--			  WPARAM wParam,
--			  LPARAM lParam
--			  )
--
-- RETURNS: INT_PTR
--
-- NOTES:
-- This function displays a simple box that contains the information about this application
----------------------------------------------------------------------------------------------------------------------*/

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: addFile
--
-- DATE: Feb 18, 2020
--
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: BOOL addFile(
--			  OPENFILENAME& ofn, //a structure that contains the information of the file
--			  HWND hdl,			 //the handle for the dialog box that called this function
--			  int role			 //the role of the application
--			  )
--
-- RETURNS: INT_PTR
--
-- NOTES:
-- This function displays a simple box that contains the information about this application
----------------------------------------------------------------------------------------------------------------------*/

BOOL addFile(OPENFILENAME& ofn, HWND hdl, int role) {
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hdl;
	ofn.lpstrFile = (LPWSTR)wnd_Info.szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(wnd_Info.szFile);
	ofn.lpstrFilter = L"Text\0*.TXT\0";
	//ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (role == CLIENT) {
		return GetOpenFileNameW((LPOPENFILENAMEW)&ofn);
	}
	if (role == SERVER) {
		return GetSaveFileNameW((LPOPENFILENAMEW)&ofn);
	}
	return false;
}