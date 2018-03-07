#include "wscapture.h"
#include "wsclient.h"
#include "decoder.h"
#include <thread>

#define COHAGNN_1 "ws://echo.websocket.org";
#define COHAGNN_2 "ws://2c1.vcam.viettel.vn";
#define COHAGNN_3 "ws://2c1.vcam.viettel.vn/evup/a81702bf7cbc/d02212d8bb69xyz5816"
#define COHAGNN_4 "ws://2c1.vcam.viettel.vn/evup/a81702c22743/d02212d8b925xyz7714"
#define COHAGNN_5 "ws://2c1.vcam.viettel.vn/evup/a81702bccc58/d02212d8c193xyz6464"
#define COHAGNN_6 "ws://2c1.vcam.viettel.vn/evup/a81702c22907/d02212d8b943xyz9054"

#define WS_MAIN_01 "ws://2c1.vcam.viettel.vn/evup/a81702c063f7/d02212d812a9xyz9332"
#define WS_MAIN_02 "ws://2c1.vcam.viettel.vn/evup/a81702c063f2/d02212d8bc14xyz9288"
#define WS_MAIN_03 "ws://2c1.vcam.viettel.vn/evup/a81702c063f7/d02212d8bd7fxyz9324"
#define WS_MAIN_04 "ws://2c1.vcam.viettel.vn/evup/010203040506/d02212d8b9d3xyz5808"

#define WS_MAIN_05 "ws://2c1.vcam.viettel.vn/evup/a81702bccbeb/d02212d8be1exyz6118"//Cam sau
#define WS_MAIN_06 "ws://2c1.vcam.viettel.vn/evup/000000000001/d02212d98452xyz18527"//Cam trc
#define WS_MAIN_07_CAMAU "ws://2c1.vcam.viettel.vn/evup/0015181701c5/d02212d98348xyz16301"

// #define WS_NEW_1    "ws://2c1.vcam.viettel.vn/evup/001518170182/d02212d98452xyz19774"
#define WS_NEW_1    "ws://2c1.vcam.viettel.vn/evup/001518170102/d02212d98452xyz19774"
#define WS_NEW_2    "ws://2c1.vcam.viettel.vn/evup/010203040506/d02212d98448xyz20466"

// #define DISPLAY_VIDEO

struct lws_context* ctx;
bool isRunning = true;
string m_url;
using namespace vws;

// void onSigInt(int sig);
void WebsocketProcess();
void DecodeProcess();

void vws::open(string url)
{
    if (url.compare("") == 0) {
        url = WS_NEW_1;
    }
    else if (url.compare("test") == 0) {
        url = WS_NEW_2;
    }

    // signal(SIGINT, onSigInt); // Register the SIGINT handler
    m_url = url;
    ctx = init_socket(m_url);
    if (NULL != ctx) {
        thread tid1(WebsocketProcess);
        thread tid2(DecodeProcess);
        tid1.detach();
        tid2.detach();

        #ifdef DISPLAY_VIDEO
        thread tid3(test);
        tid3.detach();
        #endif
    }
    else {
        cout << "cannot connect to websocket"<<endl;
    }
}

frame_packet vws::getImageInfo()
{
    return getFrame();
}
bool vws::isOpened()
{
    return isOpen();
}
// void onSigInt(int sig)
// {
//     isRunning = false;
//     SetExitStt(true);
//     SetRunningStt(false);
// }
void reconnectSocket()
{
    cout << "Reconnect to websocket..." <<endl;
    deinit_socket(ctx);
    ctx = init_socket(m_url);
}
void WebsocketProcess()
{
    while(true) {
        if (isOpened()) {
            ws_ReadHandler(ctx);
        }
        usleep(1000000);
        reconnectSocket();
    }
}
void DecodeProcess()
{
    while(true) {
        decodeProcess();
        usleep(1000000);
    }
}