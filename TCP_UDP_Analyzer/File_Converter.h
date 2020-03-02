#pragma once
#include "TCP_UDP_Analyzer.h"
using namespace std;
bool packetizeFile(const WCHAR* filePath, vector<char*>* sbuf, int pSize);

