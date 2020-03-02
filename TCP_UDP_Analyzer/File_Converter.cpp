#include "File_Converter.h"

/*------------------------------------------------------------------------------------------------------------------
-- SOURCE FILE: File_Converter.cpp - This application processes incomming and outgoing frames using the provided functions.
--
--
-- PROGRAM: TCP_UDP_Analyzer
--
-- FUNCTIONS: bool packetizeFile(const WCHAR* filePath, vector<char*>* sbuf, int pSize);
--
-- DATE: Feb 18, 2020
--
-- REVISIONS: none
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- NOTES:
-- This file has functionalities that packetize a text file using vector
----------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------------------
-- FUNCTION: packetizeFile
--
-- DATE: Feb 18, 2020
--
-- REVISIONS: none
--
-- DESIGNER: Sam Lee
--
-- PROGRAMMER: Sam Lee
--
-- INTERFACE: bool packetizeFile(
--					const WCHAR *filePath, //file path
--					vector<char*>* sbuf,   //buffer to store the file
--					int pSize			   //packet size
--					)
--
--
-- RETURNS: bool
--
-- NOTES:
-- It reads a text file and put it into memory as multiple packets with the size specified by user
----------------------------------------------------------------------------------------------------------------------*/

bool packetizeFile(const WCHAR *filePath, vector<char*>* sbuf, int pSize) {
	ifstream file{ filePath };
	char c;
	int cCounter;
	int pCounter;
	cCounter = 0;
	pCounter = -1;

	//filling characters 
	while (file.get(c)) {

		if (cCounter == 0) {
			char* buffer = (char *)malloc(pSize);
			sbuf->push_back(buffer);
			pCounter++;
		}
		sbuf->at(pCounter)[cCounter++] = c;
		if (cCounter == pSize - 1) {
			cCounter = 0;
		}
	}
	if (file.eof()) {
		memset(&sbuf->at(pCounter)[cCounter], 0, pSize - cCounter);
	}
	return true;
}
