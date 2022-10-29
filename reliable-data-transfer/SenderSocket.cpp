#include "pch.h"
#include "SenderSocket.h" 

SenderSocket::SenderSocket() {
    start_time = clock();
    current_time = clock();
}

int SenderSocket::Open(char* host, int port, int senderWindow, LinkProperties* lp)
{
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET)
    {
        printf("socket() generated error %d\n", WSAGetLastError());
        return 0;
    }

    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = htons(0);
    if (bind(sock, (struct sockaddr*)&local, sizeof(local)) == SOCKET_ERROR)
    {
        printf("bind() generated error %d\n", WSAGetLastError());
        return 0;
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
            return 0;
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

    for (int i = 0; i < 3; i++) {
        current_time = clock() - start_time;
        printf("[%.3f] --> SYN 0 (attempt %d of %d, RTO) to \n", (float)(current_time / (float)1000), i+1, MAX_SYN_ATTEMPTS);

        if (sendto(sock, (char*)ssh, sizeof(SenderSynHeader), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            printf("socket generated error %d\n", WSAGetLastError());
            return 0;
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
            printf("timeout\n");
            continue;
        }

        if (available == SOCKET_ERROR)
        {
            printf("socket generated error %d\n", WSAGetLastError());
            return 0;
        };

        if (available > 0)
        {
            int bytes_received = recvfrom(sock, res_buf, sizeof(ReceiverHeader), 0, (struct sockaddr*)&res_server, &res_server_size);

            if (res_server.sin_addr.S_un.S_addr != server.sin_addr.S_un.S_addr || res_server.sin_port != server.sin_port) {
                printf("++ invalid reply: wrong server replied\n");
                return 0;
            }

            if (bytes_received == SOCKET_ERROR)
            {
                printf("socket error %d\n", WSAGetLastError());
                return 0;
            };

            ReceiverHeader* rh = (ReceiverHeader*)res_buf;

            if (rh->flags.SYN == 1 && rh->flags.ACK == 1) {
                current_time = clock() - start_time;
                printf("[%.3f] <-- SYN-ACK 0 window %d; setting initial RTO to \n", (float)(current_time / (float)1000), rh->recvWnd);
                return STATUS_OK;
            }
        }
    }
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

    for (int i = 0; i < 3; i++) {
        current_time = clock() - start_time;
        printf("[%.3f] --> FIN 0 (attempt %d of %d, RTO) \n", (float)(current_time / (float)1000), i + 1, MAX_ATTEMPTS);

        if (sendto(sock, (char*)ssh, sizeof(SenderSynHeader), 0, (struct sockaddr*)&server, sizeof(server)) == SOCKET_ERROR)
        {
            printf("socket generated error %d\n", WSAGetLastError());
            return 0;
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
            printf("timeout\n");
            continue;
        }

        if (available == SOCKET_ERROR)
        {
            printf("socket generated error %d\n", WSAGetLastError());
            return 0;
        };

        if (available > 0)
        {
            int bytes_received = recvfrom(sock, res_buf, sizeof(ReceiverHeader), 0, (struct sockaddr*)&res_server, &res_server_size);

            if (res_server.sin_addr.S_un.S_addr != server.sin_addr.S_un.S_addr || res_server.sin_port != server.sin_port) {
                printf("++ invalid reply: wrong server replied\n");
                return 0;
            }

            if (bytes_received == SOCKET_ERROR)
            {
                printf("socket error %d\n", WSAGetLastError());
                return 0;
            };

            ReceiverHeader* rh = (ReceiverHeader*)res_buf;

            if (rh->flags.FIN == 1 && rh->flags.ACK == 1) {
                current_time = clock() - start_time;
                printf("[%.3f] <-- FIN-ACK 0 window %d\n", (float)(current_time / (float)1000), rh->recvWnd);
                return STATUS_OK;
            }
        }
    }
}