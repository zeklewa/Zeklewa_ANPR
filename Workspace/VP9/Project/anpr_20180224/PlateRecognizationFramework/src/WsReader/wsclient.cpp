#include "wsclient.h"
#include "libdownload.h"
#include "decoder.h"

#define MAX_PACKAGE_SIZE    49
#define MIN_PACKAGE_SIZE    9

//#define DISABLE_THREAD_DOWNLOADER // Do not create multithread for downloading
#define MIN_DOWNLOAD_THREAD 5
#define MAX_DOWNLOAD_THREAD 12

#define EXAMPLE_RX_BUFFER_BYTES (10)
#define MAX_ITEM (10)  // maximum items are stored

static int bDenyDeflate = 1;
// const char msg[128] = "{ \"action\":\"hello\", \"version\":\"2.0\", \"host_id\":\"a81702c22743\", \"signature\":\"RESERVED\", \"timestamp\":\"1503548717145\" }";
const char msg[128] = "{ \"action\":\"hello\", \"version\":\"2.0\", \"host_id\":\"a81702bf7cbc\", \"signature\":\"RESERVED\", \"timestamp\":\"1503907530054\" }";

static int callback_test(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

bool m_isOpen;
bool bExit;
int  msgIndex = 0;
bool pauseDownloads = false;
bool pauseDecode = false;
int  count_downloading = 0;

int count_downloadedRequest = 0;
int count_decodeLoop = 0;
int count_msgIgnore = 0;
int count_downloadErr = 0;
int count_downloadSuccess = 0;

RTCSocketState socketState = RTCSS_Disconnected;
ws_message latestMassage;
ws_header_infor headerInfo;

// LIST BUFFER
std::deque<ws_message>  listWsMessage;
std::deque<string>      listMsgDownload;
std::map<string, download_info> listDownloadPackage;

void increaseCounter()
{
    pthread_mutex_lock(&mutex);
    count_downloading++;
    pthread_mutex_unlock(&mutex);
}
void descreaseCounter()
{
    pthread_mutex_lock(&mutex);
    max(0, count_downloading--);
    pthread_mutex_unlock(&mutex);
}
int getCounter()
{
    int ret;
    pthread_mutex_lock(&mutex);
    ret = count_downloading;
    pthread_mutex_unlock(&mutex);
    return ret;
}

/************************************************************
 * Cac message tra ve tu server duoc append vao quece
 * Moi message nhan ve => Tao 1 thread de download
 * Sau khi download xong => kill thread(fix leak ram issue)
 * => append downloaded data vao quece
************************************************************/
string getNextWsMessage()
{
    ws_message msg;
    int i_sleep = 5000;
    int timeout = 10000000; // 1 000 000 = 1s

    while (msg.live_id.empty()) {
        pthread_mutex_lock(&mutex);
        if (!listWsMessage.empty()) {
            msg = listWsMessage.front();
            listWsMessage.pop_front();
        }
        pthread_mutex_unlock(&mutex);
        usleep(i_sleep);
        timeout -= i_sleep;
        if(timeout < 0) {
            cout << endl<<endl;
            cout << "*************************************" << endl;
            cout << "Connection timeout" <<endl;
            cout << "*************************************" << endl;
            cout << endl<<endl;
            bExit = true;
            usleep(2000000); // Note *****
            break;
        }
    }
    return msg.live_id;
}
int getBufferSize()
{
    int size;
    pthread_mutex_lock(&mutex);
    size = (int)listWsMessage.size();
    pthread_mutex_unlock(&mutex);
    return size;
}

string getLinkDownload()
{
    string link;
    pthread_mutex_lock(&mutex);
    if (!listMsgDownload.empty()) {
        link = listMsgDownload.front();
        listMsgDownload.pop_front();
    }
    pthread_mutex_unlock(&mutex);
    return link;
}

void *threadDownloader(void*)
{
    string link = getLinkDownload();
    if (!link.empty()) {
        downloadPacket(link);
    }
}

int step = 30;
int maxNum = 7;
// vector<int> networkLevel;
// networkLevel.push_back(step*1);
// networkLevel.push_back(step*2);
// networkLevel.push_back(step*3);
// networkLevel.push_back(step*4);
// networkLevel.push_back(step*5);
// networkLevel.push_back(step*6);
// networkLevel.push_back(step*7);
// networkLevel.push_back(step*7);

// bool calculateNetworkLevel(string pkInfo) // 785 111
// {
//     bool ret;
//     static int curState = 0;
//     int curLevel = 0;
//     static int lastLevel = 0;
//     int waitingMessageNumber;
//     pthread_mutex_lock(&mutex);
//     waitingMessageNumber = (int)listWsMessage.size();
//     pthread_mutex_unlock(&mutex);
//     for (int i=maxNum;i>0;i--) {
//         if (waitingMessageNumber>(step*i)) {
//             curLevel = i;
//         }
//     }
//     if (lastLevel != curLevel) {
//         lastLevel  = curLeve;
//         curState = 0;
//     }
//     else {
//         curState++;
//     }
//     if (curState == curLevel) {
//         ret = true;
//     }
//     else {
//         ret = false;
//     }
//     return ret;
// }

void append_ws_msg(ws_message msg_info)
{
    // printf("[DEBUG_WS] \n");
    // printf("\t MESSAGE : %s\n", msg_info.live_id.c_str());
    // printf("\t Downloading(counter) : %d\n", getCounter());

    // cout <<endl<< "[New MESSAGE] >> downloadingnumber: "<<getCounter()
    // << " Size of downloadedQueue: "<< getBufferSize() <<endl;
    if (!pauseDownloads && getCounter()>MAX_DOWNLOAD_THREAD) {
        printf("PAUSE DOWNLOADS...\n");
        printf("Đang download %d gói tin\n", getCounter());
        printf("Bỏ qua các gói tin mới\n");
        pauseDownloads = true;
    }
    else if(pauseDownloads && getCounter()<MIN_DOWNLOAD_THREAD) {
        printf("RESUME DOWNLOADS...\n");
        printf("Đang download %d gói tin\n", getCounter());
        printf("Tiếp tục download các gói tin mới\n");
        pauseDownloads = false;
    }

    if (!pauseDecode && getBufferSize()>MAX_PACKAGE_SIZE) {
        printf("*****************************\n");
        printf("PAUSE >> MAX_PACKAGE_SIZE: %d\n",getBufferSize());
        printf("Bỏ qua gói tin mới, chờ decoder\n");
        printf("*****************************\n");
        pauseDecode = true;
    }
    else if(pauseDecode && getBufferSize()<MIN_PACKAGE_SIZE) {
        printf("******************************\n");
        printf("RESUME >> MIN_PACKAGE_SIZE: %d\n", getBufferSize());
        printf("Tiếp tục xử lý các gói tin mới\n");
        printf("******************************\n");
        pauseDecode = false;
    }
    // if (getBufferSize()>MAX_PACKAGE_SIZE) {
    //     printf("MaxSize[%d] Ignore this message\n", getBufferSize());
    //     return;
    // }
    if (!pauseDownloads && !pauseDecode) {
        msgIndex++;
#ifdef TTQ_ENABLE_DEBUG
        cout <<endl<< "==============receives new message==============" <<endl;
        pthread_mutex_lock(&mutex);
        printf("[CHECK BUFFER] msgIndex[%d] listWsMessage[%d] listMsgDownload[%d] listDownloadPackage[%d]\n",
               msgIndex, listWsMessage.size(),listMsgDownload.size(),listDownloadPackage.size());
        
        cout << "count_downloadedRequest: "<<count_downloadedRequest<<endl;
        cout << "count_decodeLoop: "<<count_decodeLoop<<endl;
        cout << "count_msgIgnore: "<<count_msgIgnore<<endl;
        cout << "count_downloadErr: "<<count_downloadErr<<endl;
        cout << "count_downloadSuccess: "<<count_downloadSuccess<<endl;

        pthread_mutex_unlock(&mutex);
        // int count_downloadedRequest = 0;
        // int count_decodeLoop = 0;
        // int count_msgIgnore = 0;
        // int count_downloadErr = 0;
        // int count_downloadSuccess = 0;
#endif

        pthread_mutex_lock(&mutex);
        if ((int)listWsMessage.size()<MAX_PACKAGE_SIZE && (int)listMsgDownload.size()<5) {
            download_info newItem;
            newItem.msgIndex = msgIndex;
            newItem.status = MSG_Received;
            listDownloadPackage[msg_info.live_id] = newItem;
            latestMassage = msg_info;
            listWsMessage.push_back(msg_info);
            listMsgDownload.push_back(msg_info.live_id);
        }
        pthread_mutex_unlock(&mutex);
    }

    if (getCounter()<(MAX_DOWNLOAD_THREAD)) {
#ifdef DISABLE_THREAD_DOWNLOADER
        int* x;
        threadDownloader((void*)x);
#else
        // create downloading thread
        pthread_t tid;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&tid, &attr, threadDownloader, (void*)tid);
        pthread_join((pthread_t)tid, NULL);
        pthread_attr_destroy(&attr);
#endif
    }
}

void downloadPacket(string cur_msg)
{
    download_info cur_packet;
    int downloaded=-1;
    int checked = -2;
    string link = headerInfo.request+cur_msg;
    increaseCounter();
    count_downloadedRequest++;
    checked =  download(link, cur_packet.buffer,downloaded);
    // cout << "QUAN dai ca___VUA DOWN XONG: "<<cur_msg<<endl;
    descreaseCounter();

    std::map<string, download_info>::iterator it;
    std::map<string, download_info>::iterator pEnd;

    pthread_mutex_lock(&mutex);
    pEnd = listDownloadPackage.end();
    it = listDownloadPackage.find(cur_msg);

    if(it!=pEnd) {
        cur_packet.msgIndex = listDownloadPackage[cur_msg].msgIndex;
        if(checked >= 0) {
            count_downloadSuccess++;
            cur_packet.status = MSG_Downloaded;
            listDownloadPackage[cur_msg] = cur_packet;
        }
        else {
            count_downloadErr++;
            cur_packet.status = MSG_Download_error;
            cout << "ERROR ! Download packet error: "<<cur_msg<<endl;
            listDownloadPackage[cur_msg] = cur_packet;
        }
    }
    pthread_mutex_unlock(&mutex);
}

/************************************************************
 * Function get downloaded package
 * Voi moi message nhan tu ws, can check status cua qua trinh download:
 * 1. MSG_Downloaded:
 *      Da down xong => return data
 * 2. MSG_Download_error:
 *      Goi tin loi => xoa(ignore)
 * 3. default:
 *      Dang download => doi goi tin ve
 *      => Truong hop timeout => ignore goi tin
 *
 * Luu y:
 * 1. Can xoa goi tin sau khi xu ly xong, tranh leak ram
 * 2. Timeout doi goi tin dang la 2s. thuat toan nay co the gay delay trong truong hop mang cham
 *    => Can nghien cuu them
************************************************************/
download_info getNextPackage()
{
    count_decodeLoop++;
    int i_sleep = 10000;
    int timeout = 2000000; // 1 000 000 = 1s

    download_info pkg;
    string next_msg = getNextWsMessage();
    while(!bExit && next_msg.empty()) {
        usleep(40000);
        next_msg = getNextWsMessage();
    }
    if(!next_msg.empty()) {
        std::map<string, download_info>::iterator it;
        std::map<string, download_info>::iterator pEnd;

        pthread_mutex_lock(&mutex);
        pEnd = listDownloadPackage.end();
        it = listDownloadPackage.find(next_msg);
        pthread_mutex_unlock(&mutex);

        if(it!=pEnd) {
            while(timeout>=0) {
                pthread_mutex_lock(&mutex);
                WS_MESSAGE_STATUS stt = it->second.status;
                pthread_mutex_unlock(&mutex);
                switch (stt) {
                case MSG_Downloaded:
                    pthread_mutex_lock(&mutex);
                    pkg = it->second;
                    pthread_mutex_unlock(&mutex);
                    timeout = -1; // break
                    break;
                case MSG_Download_error:
                    timeout = -1; // break
                    break;
                default:
                    timeout-=i_sleep; // waiting for download to complete
                    break;
                }
                usleep(i_sleep);
            }
            pthread_mutex_lock(&mutex);
            listDownloadPackage.erase(it);
            pthread_mutex_unlock(&mutex);
        }
        else {
            cout << "[timeout] ERROR !!! Not found in listDownloadPackage"<<endl;
        }
    }
    return pkg;
}

/******************************************************
 * Cac functions tao connection den ws server
 * Init => connect ws => send signature key
 *
 * Sau khi connect thanh cong
 * Data tu server tra ve trong ham callback_test | case: LWS_CALLBACK_CLIENT_RECEIVE
******************************************************/
std::vector<string> split(const string &s, char delim)
{
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
ws_header_infor parse_ws_header(string header)
{
    ws_header_infor hd_info;
    size_t start =  header.find_first_of("//");
    string sub_str = header.substr(start+2,header.size()-1);
    start =  sub_str.find_first_of("/");
    hd_info.host = sub_str.substr(0,start);
    start = sub_str.find_last_of("/");
    hd_info.cam = sub_str.substr(start+1,sub_str.size()-1);
    sub_str = sub_str.substr(0,start);
    start = sub_str.find_last_of("/");
    hd_info.mac = sub_str.substr(start+1,sub_str.size()-1);
    hd_info.request = "http://"+hd_info.host+"/live/g/"+hd_info.cam+"/";
    return hd_info;
}
ws_message parse_ws_message(char* msg_recived)
{
    ws_message msg_info;
    std::string str_std(msg_recived);
    //Split "[" and "]"
    size_t start =  str_std.find_first_of("[");
    size_t end =  str_std.find_first_of("]");

    //String for split
    string str_std_2 = str_std.substr(start+1,end-start-1);

    //Extract id and size
    vector<string> x = split(str_std_2,' ');
    if(x.size() > 2)
    {
        //        msg_info.live_id = (unsigned long) stol(x[0]);
        msg_info.live_id = (x[0]);
        msg_info.size = (unsigned int) stoi(x[2]);
    }
    return msg_info;
}
static struct lws_protocols protocols[] = {
{
    "test-protocol", // Protocol name
    callback_test,   // Protocol callback
    0,               // Data size per session (can be left empty)
    512,             // Receive buffer size (can be left empty)

},
{ NULL, NULL, 0 } // Always needed at the end
};
static const struct lws_extension extensions[] = {
{
    "permessage-deflate",
    lws_extension_callback_pm_deflate,
    "permessage-deflate; client_max_window_bits"
},
{
    "deflate-frame",
    lws_extension_callback_pm_deflate,
    "deflate_frame"
},
{ NULL, NULL, NULL } // Always needed at the end
};

static int callback_test(struct lws* wsi, enum lws_callback_reasons reason, void *user, void* in, size_t len)
{
    switch (reason)
    {
    case LWS_CALLBACK_CLOSED:
        printf("[Test Protocol] Connection closed.\n");
        bExit = true;
        break;
    case LWS_CALLBACK_CLIENT_RECEIVE:
    {
        // cout << "..........................\n";
        switch (socketState) {
        case RTCSS_SentSignature: {
            socketState = RTCSS_ReceivedSignature;
        } break;
        case RTCSS_ReceivedSignature: {
            socketState = RTCSS_ReceivedInfomation;
        } break;
        case RTCSS_ReceivedInfomation: {
            ws_message newMsg = parse_ws_message((char*)in);
            append_ws_msg(newMsg);
        } break;
        default:
            break;
        }
    }
        break;

        // Here the server tries to confirm if a certain extension is supported by the server
    case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:
        if (strcmp((char*)in, "deflate-stream") == 0)
        {
            if (bDenyDeflate)
            {
                printf("[Test Protocol] Denied deflate-stream extension\n");
                return 1;
            }
        }
        break;

        // The connection was successfully established
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
    {
        printf("[Test Protocol] Connection to server established.\n");

        // NOTICE: data which is sent always needs to have a certain amount of memory (LWS_PRE) preserved for headers
        unsigned char buf[LWS_PRE + 128];
        // Allocating the memory for the buffer, and copying the message to it
        memset(&buf[LWS_PRE], 0, 128);
        string msg = "{ \"action\":\"hello\", \"version\":\"2.0\", \"host_id\":\""
                + headerInfo.mac
                // +"\", \"signature\":\"RESERVED\", \"timestamp\":\"1503907530054\" }";
                +"\", \"signature\":\"RESERVED\", \"timestamp\":\"1503907530054\" }";
        cout << "CHECK: "<< msg << endl;
        strncpy((char*)buf + LWS_PRE, msg.c_str(), 128);

        // Write the buffer from the LWS_PRE index + 128 (the buffer size)
        lws_write(wsi, &buf[LWS_PRE], msg.length(), LWS_WRITE_TEXT);
        cout << "Send signature. Waiting for reply from server......\n" <<endl;
        socketState = RTCSS_SentSignature;
        break;
    }
    case LWS_CALLBACK_CLIENT_WRITEABLE:
    {
//        printf("[Test Protocol] The client is able to write.\n");
        break;
    }
    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        printf("[Test Protocol] There was a connection error: %s\n", in ? (char*)in : "(no error information)");
        break;
    default:
        break;
    }
    return 0;
}
struct lws_context* init_socket(string wslink)
{
    initializeDecoder();
    lws_set_log_level(LLL_ERR | LLL_WARN, lwsl_emit_syslog); // We don't need to see the notice messages

    // Connection info
    int inputPort = 80;
    const char *urlProtocol, *urlTempPath; // the protocol of the URL, and a temporary pointer to the path
    char urlPath[300]; // The final path string
    headerInfo = parse_ws_header(wslink);

    struct lws_context_creation_info ctxCreationInfo; // Context creation info
    struct lws_client_connect_info clientConnectInfo; // Client creation info
    struct lws_context *ctx; // The context to use

    // Set both information to empty and allocate it's memory
    memset(&ctxCreationInfo, 0, sizeof(ctxCreationInfo));
    memset(&clientConnectInfo, 0, sizeof(clientConnectInfo));

    clientConnectInfo.port = inputPort; // Set the client info's port to the input port

    if (lws_parse_uri((char*)wslink.c_str(), &urlProtocol, &clientConnectInfo.address, &clientConnectInfo.port, &urlTempPath))
    {
        printf("Couldn't parse URL\n");
    }
    // Fix up the urlPath by adding a / at the beginning, copy the temp path, and add a \0 at the end
    urlPath[0] = '/';
    strncpy(urlPath + 1, urlTempPath, sizeof(urlPath) - 2);
    urlPath[sizeof(urlPath) - 1] = '\0';

    clientConnectInfo.path = urlPath; // Set the info's path to the fixed up url path

    // Set up the context creation info
    ctxCreationInfo.port = CONTEXT_PORT_NO_LISTEN; // We don't want this client to listen
    ctxCreationInfo.protocols = protocols; // Use our protocol list
    ctxCreationInfo.gid = -1; // Set the gid and uid to -1, isn't used much
    ctxCreationInfo.uid = -1;
    ctxCreationInfo.extensions = extensions; // Use our extensions list

    // Create the context with the info
    ctx = lws_create_context(&ctxCreationInfo);
    if (ctx == NULL)
    {
        printf("Error creating context\n");
        return NULL;
    }

    // Set up the client creation info
    clientConnectInfo.context = ctx; // Use our created context
    clientConnectInfo.ssl_connection = 0; // Don't use SSL for this test
    clientConnectInfo.host = clientConnectInfo.address; // Set the connections host to the address
    clientConnectInfo.origin = clientConnectInfo.address; // Set the conntections origin to the address
    clientConnectInfo.ietf_version_or_minus_one = -1; // IETF version is -1 (the latest one)
    clientConnectInfo.protocol = protocols[PROTOCOL_TEST].name; // We use our test protocol

    printf("Connecting to %s://%s:%d%s \n\n", urlProtocol, clientConnectInfo.address, clientConnectInfo.port, urlPath);

    // Connect with the client info
    lws_client_connect_via_info(&clientConnectInfo);
    m_isOpen = true;
    bExit = false;
    return ctx;
}

int deinit_socket(struct lws_context* ctx)
{
    m_isOpen = false;
    bExit = true;
    lws_context_destroy(ctx);
    return 0;
}
int ws_ReadHandler(struct lws_context* ctx)
{
    while (!bExit && NULL != ctx)
    {
        lws_service(ctx, 50);
    }
    m_isOpen = false;
    return 0;
}
bool isOpen()
{
    return m_isOpen;
}
void SetExitStt(bool stt)
{
    cout << "SetExitStt " << stt<<endl;
    bExit = stt;
}

