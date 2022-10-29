#pragma once
// SenderSocket.h
#define MAGIC_PORT 22345 // receiver listens on this port
#define MAX_PKT_SIZE (1500-28) // maximum UDP packet size accepted by receiver
// possible status codes from ss.Open, ss.Send, ss.Close
#define STATUS_OK 0 // no error
#define ALREADY_CONNECTED 1 // second call to ss.Open() without closing connection
#define NOT_CONNECTED 2 // call to ss.Send()/Close() without ss.Open()
#define INVALID_NAME 3 // ss.Open() with targetHost that has no DNS entry
#define FAILED_SEND 4 // sendto() failed in kernel
#define TIMEOUT 5 // timeout after all retx attempts are exhausted
#define FAILED_RECV 6 // recvfrom() failed in kernel
#define MAGIC_PROTOCOL 0x8311AA 

#define FORWARD_PATH 0
#define RETURN_PATH 1 

#define MAX_SYN_ATTEMPTS 3
#define MAX_ATTEMPTS 5

#pragma pack(push,1) 
class Flags {
public:
    DWORD reserved : 5; // must be zero
    DWORD SYN : 1;
    DWORD ACK : 1;
    DWORD FIN : 1;
    DWORD magic : 24;
    Flags() { memset(this, 0, sizeof(*this)); magic = MAGIC_PROTOCOL; }
};
class SenderDataHeader {
public:
    Flags flags;
    DWORD seq; // must begin from 0
};
class LinkProperties {
public:
    // transfer parameters
    float RTT; // propagation RTT (in sec)
    float speed; // bottleneck bandwidth (in bits/sec)
    float pLoss[2]; // probability of loss in each direction
    DWORD bufferSize; // buffer size of emulated routers (in packets)
    LinkProperties() { memset(this, 0, sizeof(*this)); }
};
class SenderSynHeader {
public:
    SenderDataHeader sdh;
    LinkProperties lp;
};
class ReceiverHeader {
public:
    Flags flags;
    DWORD recvWnd; // receiver window for flow control (in pkts)
    DWORD ackSeq; // ack value = next expected sequence
};
#pragma pack(pop)

class SenderSocket {
    SOCKET sock;
    float rto;
    struct sockaddr_in server;
    clock_t start_time, current_time;
public:
    SenderSocket();
    int Open(char* host, int port, int senderWindow, LinkProperties* lp);
    int Send();
    int Close(int senderWindow, LinkProperties* lp);
};