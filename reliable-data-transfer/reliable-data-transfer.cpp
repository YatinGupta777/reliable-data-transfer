// reliable-data-transfer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/*
    Yatin Gupta
    Class 612 : Networks and distributed processing
    1st Semester
    530006656
    yatingupta@tamu.edu
    +1 (979) 422-1194
*/

#include "pch.h"
#include <iostream>
#include "SenderSocket.h" 

int main(int argc, char** argv)
{
    std::cout << "Hello World!\n";
    if (argc != 8)
    {
        printf("Invalid Usage: Required [Lookup hostname/IP] [DNS server IP]");
        return 0;
    }

    // parse command-line parameters
    char* targetHost = argv[1];
    int power = atoi(argv[2]); // command-line specified integer
    int senderWindow = atoi(argv[3]); // command-line specified integer
    UINT64 dwordBufSize = (UINT64)1 << power;
    DWORD* dwordBuf = new DWORD[dwordBufSize]; // user-requested buffer
    for (UINT64 i = 0; i < dwordBufSize; i++) // required initialization
        dwordBuf[i] = i;
    SenderSocket ss; // instance of your class
    if ((status = ss.Open(targetHost, MAGIC_PORT, senderWindow, ...)) != STATUS_OK)
        // error handling: print status and quit
        char* charBuf = (char*)dwordBuf; // this buffer goes into socket
    UINT64 byteBufferSize = dwordBufSize << 2; // convert to bytes
    UINT64 off = 0; // current position in buffer
    while (off < byteBufferSize)
    {
        // decide the size of next chunk
        int bytes = min(byteBufferSize - off, MAX_PKT_SIZE - sizeof(SenderDataHeader));
        // send chunk into socket
        if ((status = ss.Send(charBuf + off, bytes)) != STATUS_OK)
            // error handing: print status and quit
            off += bytes;
    }
    if ((status = ss.Close()) != STATUS_OK)
        // error handing: print status and quit 
}
