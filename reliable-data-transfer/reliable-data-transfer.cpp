// reliable-data-transfer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/*
    Yatin Gupta
    Class 612 : Networks and distributed processing
    1st Semester
    yatingupta@tamu.edu
    +1 (979) 422-1194
*/

#include "pch.h"
#include "SenderSocket.h" 
#pragma comment(lib, "ws2_32.lib")


//destination
//server(hostname or IP), a power - of - 2 buffer size to be transmitted(in DWORDs), sender window(in packets), the round - trip propagation delay(in seconds), the probability of loss in each
//direction, and the speed of the bottleneck link(in Mbps)

int main(int argc, char** argv)
{
    if (argc != 8)
    {
        printf("Invalid Usage: Required destination server(hostname or IP), a power - of - 2 buffer size to be transmitted(in DWORDs), sender window(in packets), the round - trip propagation delay(in seconds), the probability of loss in each direction, and the speed of the bottleneck link(in Mbps)");
        return 0;
    }

    WSADATA wsaData;

    //Initialize WinSock; once per program run 
    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        printf("WSAStartup failed with %d\n", WSAGetLastError());
        WSACleanup();
        return 0;
    }

    // parse command-line parameters
    char* targetHost = argv[1];
    int power = atoi(argv[2]); 
    int senderWindow = atoi(argv[3]); 
    char* rtt = argv[4];
    char* loss_prob_fwd = argv[5];
    char* loss_prob_rev = argv[6];
    char* bottleneck_link_speed = argv[7];

    UINT64 dwordBufSize = (UINT64)1 << power;
    DWORD* dwordBuf = new DWORD[dwordBufSize]; // user-requested buffer
    for (UINT64 i = 0; i < dwordBufSize; i++) // required initialization
        dwordBuf[i] = i;

    LinkProperties lp;
    lp.RTT = atof(rtt);
    lp.speed = 1e6 * atof(bottleneck_link_speed); // convert to megabits
    lp.pLoss[FORWARD_PATH] = atof(loss_prob_fwd);
    lp.pLoss[RETURN_PATH] = atof(loss_prob_rev);

    SenderSocket ss; // instance of your class
    DWORD status;
    if ((status = ss.Open(targetHost, MAGIC_PORT, senderWindow, &lp)) != STATUS_OK) {

    }
        // error handling: print status and quit
    //char* charBuf = (char*)dwordBuf; // this buffer goes into socket
    //UINT64 byteBufferSize = dwordBufSize << 2; // convert to bytes
    //UINT64 off = 0; // current position in buffer
    //while (off < byteBufferSize)
    //{
    //    // decide the size of next chunk
    //    //int bytes = min(byteBufferSize - off, MAX_PKT_SIZE - sizeof(SenderDataHeader));
    //    //// send chunk into socket
    //    //if ((status = ss.Send(charBuf + off, bytes)) != STATUS_OK)
    //    //    // error handing: print status and quit
    //    //    off += bytes;
    //}
   // if ((status = ss.Close()) != STATUS_OK){}
        // error handing: print status and quit 
}
