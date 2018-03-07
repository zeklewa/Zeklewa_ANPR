#include "decoder.h"
#include "wsclient.h"
#include "cvBufferCapture.h"

#define MB_NAL_TYPE 0x1F //|00011111|
#define MB_FU_S     0x80
#define MB_FU_E     0x40
#define MB_FU_R     0x20

#define HEADER_LENGTH       12
#define OFSET_TIMESTAMP     1262304000000 // 40 years

pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
struct frame_packet m_frame;
CvCapture_FFMPEG cap;
bool bRunning;

vector<char>  IDRPicture;
vector<char>  NALUStartCode = {0,0,0,1};
vector<char>  IDR_NAL_TYPE  = {101}; //0x65
bool IDRPictureStarted = false;
bool checkIFrameFirst = false;
timestampType curTimeStamp;

// LIST BUFFER
std::deque<frame_packet> listFrame;

bool isRunning()
{
    return bRunning;
}
void SetRunningStt(bool stt)
{
    cout << "SetRunningStt: "<<stt<<endl;
    bRunning = stt;
}
struct frame_packet getFrame()
{
    frame_packet lastItem;
    while (isRunning() && lastItem.frame.empty()) {
        pthread_mutex_lock(&mutex2);
        if (!listFrame.empty()) {
            // cout << "getFrame: "<<endl;
            lastItem = listFrame.front();
            listFrame.pop_front();
            #ifdef DISPLAY_VIDEO
            if (!lastItem.frame.empty()) {
                imshow("myFunction()",lastItem.frame);
                waitKey(1);
            }
            #endif
        }
        pthread_mutex_unlock(&mutex2);
        usleep(1000);  // waiting for new frame - 1ms
    }
    return lastItem;
}
void appendFrame(frame_packet item) {
    pthread_mutex_lock(&mutex2);
    listFrame.push_back(item);
    pthread_mutex_unlock(&mutex2);
}

void decodeProcess()
{
    struct timeval t0;
    struct timeval t1;
    struct timeval t2;
    download_info curPackage;
    while (isRunning()) {
        gettimeofday(&t0, NULL);
        curPackage = getNextPackage();
        gettimeofday(&t1, NULL);
        float val = (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
        // printf("______________CHECKTIME[getNextPackage]  %f\n", val);
        // printf("decodeProcess msgIndex[%d]\n",curPackage.msgIndex);
        if (!curPackage.buffer.empty() && curPackage.status == MSG_Downloaded) {
            splitData(curPackage.buffer);
            gettimeofday(&t2, NULL);
            float val2 = (t2.tv_sec - t1.tv_sec) * 1000.0f + (t2.tv_usec - t1.tv_usec) / 1000.0f;
            // printf("______________CHECKTIME[splitData]  %f\n", val2);
//            splitMainRtpPackage(curPackage.buffer);
            usleep(1000);
        }
        else {
            std::cout << "ERROR! Nodata for decodeProcess" << std::endl;
            usleep(50000);
        }
    }
}

void initializeDecoder() {
    bRunning = true;
    cap.init();
}

void checkingFUApackage(vector<char> h264Raw) {
    unsigned int nalType = h264Raw[4] & MB_NAL_TYPE;
    switch (nalType) {
    case 7:  // SPS Type
    case 8:  // PPS Type
    case 5:  // IDR Typ
    case 1:  // P-Frame  Pictur Type
        decodeRTPPackage(NALUStartCode + h264Raw);
        break;
    default:
        break;
    }
}

void decodeRTPPackage(vector<char> rtpPackage) {
    // return;
    struct timeval x0;
    struct timeval x1;

    gettimeofday(&x0, NULL);
    int rtpSize = rtpPackage.size();
#ifdef TTQ_DEBUG_SPLIT
    cout << "decodeRTPPackage "<<rtpPackage.size() <<endl;
    print_hex_vector(rtpPackage,0,rtpSize>10?10:rtpSize);
#endif
#ifdef H264_FILE
    exportVectorToFile(H264_FILE,rtpPackage);
#endif
    uint8_t *data = new uint8_t[rtpSize];
    memcpy(data, rtpPackage.data(), rtpSize);

    cap.grabFrame(data,rtpSize);

    IplImage* img2 = cap.retrieveFrame();

    if (img2) {
        cv::Mat image2;
        image2 = cvarrToMat(img2);
//        image2 = cap.retrieveMatFrame();
        m_frame.frame = image2.clone();
        m_frame.index++;
        m_frame.timestamp = curTimeStamp;
        appendFrame(m_frame);
    }
    gettimeofday(&x1, NULL);
    float val = (x1.tv_sec - x0.tv_sec) * 1000.0f + (x1.tv_usec - x0.tv_usec) / 1000.0f;
    // printf("__________________________________________CHECKTIME[decodeRTPPackage]  %f\n", val);
    rtpPackage.clear();
    free(data);
}

void splitData(vector<char> rawData) {
    struct timeval t0;
    struct timeval t1;
    struct timeval t2;
    gettimeofday(&t0, NULL);

    if (!rawData.empty()) {
        long passedBytes = 0;
        bool rtpIsOk = true;
        while (isRunning() && passedBytes < rawData.size() && rtpIsOk) {
            // Parse raw data
            int payloadSize = (rawData[passedBytes+1] & 0xFF) | ((rawData[passedBytes] & 0xFF) << 8);
            // Phần data h264 bị ngắn hơn chiều dài.
            int lengthOfH264RawData = payloadSize - 10;
#ifdef TTQ_DEBUG_SPLIT
            printf("\n\n");
            print_hex_vector(rawData,passedBytes,14);
            printf("passedBytes[%d] lengthOfH264RawData[%d] payloadSize[%d] \n",
                   passedBytes, lengthOfH264RawData,payloadSize);
#endif
            if (passedBytes + HEADER_LENGTH + lengthOfH264RawData > rawData.size()) {
                rtpIsOk = false;
                cout << " something went wrong... in RTP passedBytes + HEADER_LENGTH + lengthOfH264RawData="
                     << (passedBytes + HEADER_LENGTH + lengthOfH264RawData) << "/" << rawData.size() <<endl;
                break;
            }
            vector<char> h264Raw = mid(rawData,passedBytes+HEADER_LENGTH,lengthOfH264RawData);
            vector<char> timeData = mid(rawData,passedBytes+2,8);
            passedBytes = passedBytes + payloadSize + 2;
            
            uint64_t mtime = 0;
            for (int i =0;i<8;i++){
                mtime += timeData[i];
                if (i!=7)   mtime = mtime << 8; 
            }
            double timestamp = reinterpret_cast <double&>(mtime); 
            // memcpy(&mtime, b, sizeof(double));
            long timestampLong = (long)timestamp;
            curTimeStamp = timestampLong;

            unsigned int nalType = h264Raw[0] & MB_NAL_TYPE;
            switch (nalType) {
            case 7:  // SPS Type
            case 8:  // PPS Type
            case 1:  // P-Frame  Pictur Type
                decodeRTPPackage(NALUStartCode + h264Raw);
                break;
            case 5:  // IDR Type
                decodeRTPPackage(NALUStartCode + h264Raw);
                checkIFrameFirst = true;
                break;
            case 28: {  // IDR Picture Type MULTIPLE PART
                char FU_A_byte = h264Raw[1];
                if (FU_A_byte & MB_FU_S) {
                    IDRPicture = IDR_NAL_TYPE + shift(h264Raw, 2);
                    IDRPictureStarted = true;
                }
                else if (FU_A_byte & MB_FU_E) {  // end of Fragment Unit
                    if (IDRPictureStarted == false) {
                        rtpIsOk = false;
                        break;
                    }
                    IDRPicture = IDRPicture + shift(h264Raw, 2);
                    checkingFUApackage(NALUStartCode + IDRPicture);
                    checkIFrameFirst = true;
                    IDRPictureStarted = false;  // end of IDR Picture
                }
                else {
                    // printf("FU_A_byte No: 0x%2x \n",h264Raw[1]);
                    if (IDRPictureStarted == false) {
                        rtpIsOk = false;
                        break;
                    }
                    IDRPicture = IDRPicture + mid(h264Raw, 2, payloadSize - HEADER_LENGTH);
                }
            } break;

            case 0: // End Of 10 minute type
                break;
            default:
                cout << " something went wrong... in RTP passedBytes=" << passedBytes << "/" << rawData.size() << " nalType=" << nalType <<endl;
                break;
            }
        }
    }
    gettimeofday(&t1, NULL);
    
    float val = (t1.tv_sec - t0.tv_sec) * 1000.0f + (t1.tv_usec - t0.tv_usec) / 1000.0f;
    // printf("______________CHECKTIME[JUST]  %f\n", val);
}
