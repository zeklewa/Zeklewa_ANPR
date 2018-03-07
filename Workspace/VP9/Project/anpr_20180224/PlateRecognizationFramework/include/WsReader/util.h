#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdlib>

#include <deque>
#include <map>

// #define TTQ_ENABLE_DEBUG

using namespace std;
enum protocolList {
    PROTOCOL_TEST,
    PROTOCOL_LIST_COUNT // Needed
};
struct ws_message {
//    unsigned long live_id;
    string live_id;
    unsigned int size;
};

enum WS_MESSAGE_STATUS {
    MSG_Nodata          = 0,
    MSG_Received        = 1,
    MSG_Downloading     = 2,
    MSG_Downloaded      = 3,
    MSG_Download_error  = 4
};
struct download_info {
    WS_MESSAGE_STATUS status;
    vector<char> buffer;
    int msgIndex;
};

struct ws_header_infor {
    string request;
    string host;
    string mac;
    string cam;
};

enum RTCSocketState {
  RTCSS_Disconnected,
  RTCSS_Connected,
  RTCSS_SentSignature,
  RTCSS_ReceivedSignature,
  RTCSS_ReceivedInfomation,
  RTCSS_ReceivedDownloadLink
};

//struct ws_message parse_ws_message(char* );
//ws_header_infor parse_ws_header(string header);
//std::vector<std::string> split(const std::string &s, char delim);

void print_hex_memory(void *mem, int pos, int n);
void exportVectorToFile(string url, vector<char>);

template <typename T>
void print_hex_vector(vector<T> p, int pos, int n)
{
    int i;
    for (i=pos;i<(n+pos);i++) {
        printf("0x%02x ", p[i] & 0xff);
        if (((i-pos)%16==0) && (i!=pos))
            printf("\n");
    }
    printf("\n");
}

template <typename T>
std::vector<T> operator+(const std::vector<T> &A, const std::vector<T> &B)
{
    std::vector<T> AB;
    AB.reserve( A.size() + B.size() );                // preallocate memory
    AB.insert( AB.end(), A.begin(), A.end() );        // add A;
    AB.insert( AB.end(), B.begin(), B.end() );        // add B;
    return AB;
}

template <typename T>
std::vector<T> &operator+=(const std::vector<T> &A, const std::vector<T> &B)
{
    A.reserve( A.size() + B.size() );                // preallocate memory without erase original data
    A.insert( A.end(), B.begin(), B.end() );         // add B;
    return A;                                        // here A could be named AB
}

template <typename T>
std::vector<T> shift(std::vector<T> &A, int x)
{
    std::vector<T> AB(A.begin()+2,A.end());
    return AB;
}

template <typename T>
std::vector<T> mid(std::vector<T> &A, int pos, int length)
{
    if (-1 == length) {
        length = A.size();
    }
    std::vector<T> AB(A.begin()+pos,A.begin()+pos+length);
    return AB;
}
