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
    double rtt = atof(argv[4]);
    double loss_prob_fwd = atof(argv[5]);
    double loss_prob_rev = atof(argv[6]);
    double bottleneck_link_speed = atof(argv[7]);

    printf("Main:\tsender W = %d, RTT %.3f sec, loss %g / %g, link %.3f Mbps\n", senderWindow, rtt,
        loss_prob_fwd, loss_prob_rev, bottleneck_link_speed);

    printf("Main:\tinitializing DWORD array with 2^%d elements...", power);
    clock_t start_t, end_t;

    start_t = clock();

    UINT64 dwordBufSize = (UINT64)1 << power;
    DWORD* dwordBuf = new DWORD[dwordBufSize]; // user-requested buffer
    for (UINT64 i = 0; i < dwordBufSize; i++) // required initialization
        dwordBuf[i] = i;

    end_t = clock();

    printf(" done in %d ms\n", (end_t - start_t));
    
    LinkProperties lp;
    lp.RTT = rtt;
    lp.speed = 1e6 * bottleneck_link_speed; // convert to megabits
    lp.pLoss[FORWARD_PATH] = loss_prob_fwd;
    lp.pLoss[RETURN_PATH] = loss_prob_rev;

    SenderSocket ss; // instance of your class
    int status;
    if ((status = ss.Open(targetHost, MAGIC_PORT, senderWindow, &lp)) != STATUS_OK) {

    }
    printf("Main:\tconnected to %s in %.3f sec, pkt size %d bytes\n", targetHost, (float)(ss.syn_end_time - ss.syn_start_time)/ (float)1000, MAX_PKT_SIZE);
    start_t = clock();

    char* charbuf = (char*)dwordBuf; // this buffer goes into socket
    UINT64 bytebuffersize = dwordBufSize << 2; // convert to bytes
    UINT64 off = 0; // current position in buffer
    while (off < bytebuffersize)
    {
        // decide the size of next chunk
        //int bytes = min(bytebuffersize - off, max_pkt_size - sizeof(senderdataheader));
        //// send chunk into socket
        //if ((status = ss.send(charbuf + off, bytes)) != status_ok)
        //    // error handing: print status and quit
        //    off += bytes;
        off += 10000;
    }
    end_t = clock();
    if ((status = ss.Close(senderWindow, &lp)) != STATUS_OK){
    }
    printf("Main:\ttransfer finished in %.3f sec\n", (float)(end_t - start_t) / (float)1000);
        // error handing: print status and quit 
}
