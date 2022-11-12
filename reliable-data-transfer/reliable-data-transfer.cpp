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
#include "Checksum.h" 
#pragma comment(lib, "ws2_32.lib")

UINT stats_thread(LPVOID pParam)
{
    SenderSocket* ss = ((SenderSocket*)pParam);
    int count = 0;
    while (WaitForSingleObject(ss->eventQuit, 2000) == WAIT_TIMEOUT)
    {
        double speed = ((ss->current_ack - ss->last_base) * 8 * (MAX_PKT_SIZE - sizeof(SenderDataHeader))) / (2 * 1000000.0);
        double current_speed_total = ss->average_rate * count;
        count++;
        ss->average_rate = (current_speed_total + speed) / (double)count;

        printf("[%3d] B %4d (%.1f MB) N %4d T %d F 0 W 1 S %.3f Mbps RTT %.3f\n", (clock() - ss->start_time)/1000, ss->current_seq, ((float)ss->bytes_acked)/1000000.0, ss->current_seq+1, ss->timed_out_packets, speed, ss->estimated_rtt);
        ss->last_base = ss->current_ack;
    }

    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 8)
    {
        printf("Invalid Usage: [hostname or IP], [a power of 2 buffer size], [sender window], [rtt], [fwd loss], [backward loss], [link speed]");
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
    int bottleneck_link_speed = atof(argv[7]);

    printf("Main:\tsender W = %d, RTT %.3f sec, loss %g / %g, link %d Mbps\n", senderWindow, rtt,
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
        printf("Main : connect failed with status %d", status);
        return 0;
    }
    printf("Main:\tconnected to %s in %.3f sec, pkt %d bytes\n", targetHost, (float)(ss.syn_end_time - ss.syn_start_time)/ (float)1000, MAX_PKT_SIZE);
    start_t = clock();

    char* charBuf = (char*)dwordBuf; // this buffer goes into socket
    UINT64 byteBufferSize = dwordBufSize << 2; // convert to bytes

    Checksum cs;
    DWORD sent_checksum = cs.CRC32((unsigned char*)charBuf, byteBufferSize);

    HANDLE stats_thread_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)stats_thread, &ss, 0, NULL);

    UINT64 off = 0; // current position in buffer
    ss.start_data_time = clock();
    while (off < byteBufferSize)
    {
        //decide the size of next chunk
        int bytes = min(byteBufferSize - off, MAX_PKT_SIZE - sizeof(SenderDataHeader));
        // send chunk into socket
        if ((status = ss.Send(charBuf + off, bytes)) != STATUS_OK) {
            printf("Main : connect failed with status %d", status);
            return 0;
        }
        off += bytes;
    }
    end_t = clock();

    if ((status = ss.Close(senderWindow, &lp)) != STATUS_OK){
        printf("Main : connect failed with status %d", status);
        return 0;
    }

    SetEvent(ss.eventQuit);

    if (ss.received_checksum != sent_checksum) {
        printf("Receiver sent wrong checksum");
        return 0;
    }
    printf("Main:\ttransfer finished in %.3f sec, %.2f Kbps, checksum %X\n", (ss.end_data_time - ss.start_data_time) / 1000.0, ss.average_rate*1000, ss.received_checksum);
    printf("Main:\testRTT %.3f, ideal rate %.2f Kbps", ss.estimated_rtt, (((MAX_PKT_SIZE-sizeof(SenderDataHeader)) * 8) / ss.estimated_rtt)/ 1000.0);
}
//C:\Users\yatingupta\source\repos\YatinGupta777\reliable-data-transfer\x64\Debug