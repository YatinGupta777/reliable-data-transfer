#include "pch.h"
#include "SenderSocket.h" 

SenderSocket::SenderSocket() {
    start_time = clock();
    current_time = clock();
    syn_start_time = clock();
    connection_open = false;
    rto = 1;
}

int SenderSocket::Open(char* host, int port, int senderWindow, LinkProperties* lp)
{
    if (connection_open) return ALREADY_CONNECTED;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        printf("socket() generated error %d\n", WSAGetLastError());
        return -1;
    }

    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(0);
    if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind() generated error %d\n", WSAGetLastError());
        return -1;
    }

    struct hostent* remote;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port); 

    DWORD IP = inet_addr(host);
    if (IP == INADDR_NONE)
    {
        // if not a valid IP, then do a DNS lookup
        if ((remote = gethostbyname(host)) == NULL)
        {
            printf("failed with %d\n", WSAGetLastError());
            return INVALID_NAME;
        }
        else { // take the first IP address and copy into sin_addr
            memcpy((char*)&(server.sin_addr), remote->h_addr, remote->h_length);
        }
    }
    else
    {
        server.sin_addr.S_un.S_addr = IP;
    }

    SenderSynHeader* ssh = new SenderSynHeader();

    ssh->sdh.flags.reserved = 0;
    ssh->sdh.flags.SYN = 1;
    ssh->sdh.flags.FIN = 0;
    ssh->sdh.flags.ACK = 0;
    ssh->sdh.seq = 0;

    ssh->lp = *lp;
    ssh->lp.bufferSize = senderWindow + 3;
    syn_start_time = clock() - start_time;
    for (int i = 0; i < MAX_SYN_ATTEMPTS; i++) {
        current_time = clock() - start_time;
        printf("[%.3f] --> SYN 0 (attempt %d of %d, RTO %.3f) to %s\n", (float)(current_time / (float)1000), i+1, MAX_SYN_ATTEMPTS, rto, inet_ntoa(server.sin_addr));

        if (sendto(sock, (char*)ssh, sizeof(SenderSynHeader), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            printf("failed sendto with %d\n", WSAGetLastError());
            return FAILED_SEND;
        };

        fd_set fd;
        FD_ZERO(&fd); // clear the set
        FD_SET(sock, &fd); // add your socket to the set
        timeval tp;
        tp.tv_sec = 10;
        tp.tv_usec = 0;

        struct sockaddr_in res_server;
        int res_server_size = sizeof(res_server);
        char* res_buf = new char[sizeof(ReceiverHeader)];

        int available = select(0, &fd, NULL, NULL, &tp);

        if (available == 0) {
            continue;
        }

        if (available == SOCKET_ERROR)
        {
            printf("failed recvfrom with %d\n", WSAGetLastError());
            return FAILED_RECV;
        };

        if (available > 0)
        {
            int bytes_received = recvfrom(sock, res_buf, sizeof(ReceiverHeader), 0, (struct sockaddr*)&res_server, &res_server_size);

            if (bytes_received == SOCKET_ERROR)
            {
                printf("failed recvfrom with %d\n", WSAGetLastError());
                return FAILED_RECV;
            };

            ReceiverHeader* rh = (ReceiverHeader*)res_buf;

            if (rh->flags.SYN == 1 && rh->flags.ACK == 1) {
                clock_t temp = current_time;
                current_time = clock() - start_time;
                syn_end_time = current_time;
                rto = ((float)((current_time) -temp) * 3) / 1000;
                printf("[%.3f] <-- SYN-ACK 0 window %d; setting initial RTO to %.3f\n", (float)(current_time / (float)1000), rh->recvWnd, rto);
                connection_open = true;
                return STATUS_OK;
            }
        }
    }
    return TIMEOUT;
}
int SenderSocket::Send()
{
    if (!connection_open) return NOT_CONNECTED;
}


int SenderSocket::Close(int senderWindow, LinkProperties* lp)
{
    SenderSynHeader* ssh = new SenderSynHeader();

    ssh->sdh.flags.reserved = 0;
    ssh->sdh.flags.SYN = 0;
    ssh->sdh.flags.FIN = 1;
    ssh->sdh.flags.ACK = 0;
    ssh->sdh.seq = 0;

    ssh->lp = *lp;
    ssh->lp.bufferSize = senderWindow + 3;

    for (int i = 0; i < MAX_ATTEMPTS; i++) {
        current_time = clock() - start_time;
        printf("[%.3f] --> FIN 0 (attempt %d of %d, RTO %.3f) \n", (float)(current_time / (float)1000), i + 1, MAX_ATTEMPTS, rto);

        if (sendto(sock, (char*)ssh, sizeof(SenderSynHeader), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            printf("failed sendto with %d\n", WSAGetLastError());
            return FAILED_SEND;
        };

        fd_set fd;
        FD_ZERO(&fd); // clear the set
        FD_SET(sock, &fd); // add your socket to the set
        timeval tp;
        tp.tv_sec = 10;
        tp.tv_usec = 0;

        struct sockaddr_in res_server;
        int res_server_size = sizeof(res_server);
        char* res_buf = new char[sizeof(ReceiverHeader)];

        int available = select(0, &fd, NULL, NULL, &tp);

        if (available == 0) {
            continue;
        }

        if (available == SOCKET_ERROR)
        {
            printf("failed recvfrom with %d\n", WSAGetLastError());
            return FAILED_RECV;
        };

        if (available > 0)
        {
            int bytes_received = recvfrom(sock, res_buf, sizeof(ReceiverHeader), 0, (struct sockaddr*)&res_server, &res_server_size);

            if (bytes_received == SOCKET_ERROR)
            {
                printf("failed recvfrom with %d\n", WSAGetLastError());
                return FAILED_RECV;
            };

            ReceiverHeader* rh = (ReceiverHeader*)res_buf;

            if (rh->flags.FIN == 1 && rh->flags.ACK == 1) {
                current_time = clock() - start_time;
                printf("[%.3f] <-- FIN-ACK 0 window %d\n", (float)(current_time / (float)1000), rh->recvWnd);
                connection_open = false;
                return STATUS_OK;
            }
        }
    }
    return TIMEOUT;
}