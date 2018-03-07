#include <queue>
#include <stack>
#include <mutex>
#include <chrono>
#include <string>
#include <thread>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
// #include <Amvideocaptools.h>
#include <fstream>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "VideoReader.h"
#include "wscapture.h"

using namespace pr;
using namespace std;
using namespace cv;

// #define VIDEOCAPDEV "/dev/amvideocap0"
// #define CAP_WIDTH_MAX 1920
// #define CAP_HEIGHT_MAX 1080

// #define ENDIAN_SHIFT 24
// #define LITTLE_ENDIAN (1 << ENDIAN_SHIFT)
// #define FMT_S24_RGB (LITTLE_ENDIAN | 0x00200)  /* 10_00_0_00_0_00 */
// #define FMT_S16_RGB (LITTLE_ENDIAN | 0x00100)  /* 01_00_0_00_0_00 */
// #define FMT_S32_RGBA (LITTLE_ENDIAN | 0x00300) /* 11_00_0_00_0_00 */

// #define COLOR_MAP_SHIFT 20
// #define COLOR_MAP_MASK (0xf << COLOR_MAP_SHIFT)
// /* 16 bit */
// #define COLOR_MAP_RGB565 (5 << COLOR_MAP_SHIFT)
// /* 24 bit */
// #define COLOR_MAP_RGB888 (0 << COLOR_MAP_SHIFT)
// #define COLOR_MAP_BGR888 (5 << COLOR_MAP_SHIFT)
// /* 32 bit */
// #define COLOR_MAP_RGBA8888 (0 << COLOR_MAP_SHIFT)
// #define COLOR_MAP_ARGB8888 (1 << COLOR_MAP_SHIFT)
// #define COLOR_MAP_ABGR8888 (2 << COLOR_MAP_SHIFT)
// #define COLOR_MAP_BGRA8888 (3 << COLOR_MAP_SHIFT)

// /*16 bit*/
// #define FORMAT_S16_RGB_565 (FMT_S16_RGB | COLOR_MAP_RGB565)
// /*24 bit*/
// #define FORMAT_S24_BGR (FMT_S24_RGB | COLOR_MAP_BGR888)
// #define FORMAT_S24_RGB (FMT_S24_RGB | COLOR_MAP_RGB888)
// /*32 bit*/
// #define FORMAT_S32_ARGB (FMT_S32_RGBA | COLOR_MAP_ARGB8888)
// #define FORMAT_S32_ABGR (FMT_S32_RGBA | COLOR_MAP_ABGR8888)
// #define FORMAT_S32_BGRA (FMT_S32_RGBA | COLOR_MAP_BGRA8888)
// #define FORMAT_S32_RGBA (FMT_S32_RGBA | COLOR_MAP_RGBA8888)

string urlsearchip;
int FrameIgnored1=atoi(IOData::GetCongfigData("FrameIgnored:").c_str());
std::mutex m_mtx;
bool cal_cropFrame=true;
Rect cropFrame;

// int fd;
// int ret_cap = 0;
// int init_capframe(int *w, int *h, int fmt)
// {
//     int ret_init = 0;
//     fd = open(VIDEOCAPDEV, O_RDONLY);
//     printf("fd= %d \n", fd);
//     if (fd < 0)
//     {
//         return -1;
//     }

//     if (w != NULL && *w > 0)
//     {
//         ret_init = ioctl(fd, AMVIDEOCAP_IOW_SET_WANTFRAME_WIDTH, *w);
//         //printf("ret width= %d \n", ret_init);
//     }
//     if (h != NULL && *h > 0)
//     {
//         ret_init = ioctl(fd, AMVIDEOCAP_IOW_SET_WANTFRAME_HEIGHT, *h);
//         //printf("ret height = %d \n", ret_init);
//     }
//     if (fmt)
//     {
//         ret_init = ioctl(fd, AMVIDEOCAP_IOW_SET_WANTFRAME_FORMAT, fmt);
//     }

//     return ret_init;
// }

// void close_capframe(void)
// {
//     close(fd);
// }

// int amvideocap_capframe(char *buf, int size, int *w, int *h, int *ret_size)
// {
//     fd = open(VIDEOCAPDEV, O_RDONLY);
//     if (fd < 0)
//     {
//         return -1;
//     }
//     *ret_size = read(fd, buf, size);
//     close(fd);
//     return ret_cap;
// }

// void getFrames(int width, int height, queue<FrameData> &frameQueue,Rect cropRect)
// {
//     int w, h;
//     char *buf;
//     int bufsize;
//     int ret_size;
//     int byte_per_pix = 3;
//     cv::Mat frame;
//     w = atoi(IOData::GetCongfigData("sizevideoinput_w:").c_str());
//     h = atoi(IOData::GetCongfigData("sizevideoinput_h:").c_str());
//     bufsize = w * h * byte_per_pix;
//     buf = (char *)malloc(w * h * byte_per_pix);
//     int ret = init_capframe(&w, &h, FORMAT_S24_RGB);
//     ObjectUtils objects;
//     long long id = 0;
//     int nframe = 0;
//     auto start = CLOCK_NOW();
//     int fps = atoi(IOData::GetCongfigData("fps_video_reading:").c_str());
//     int delay = 1000/fps*FrameIgnored1;
//     for (;;)
//     {
//         auto startime = CLOCK_NOW();
//         usleep(1000);
//         ret = amvideocap_capframe(buf, bufsize, &w, &h, &ret_size);
//         FrameData thisFrame;
//         if (ret_size == bufsize)
//         {
//             thisFrame.framecolor = Mat(h, w, CV_8UC3, (uchar *)buf).clone();
//         }
//         id++;
//         if (cal_cropFrame && thisFrame.framecolor.data)
//         {
//             float rate_crop_width = (float)thisFrame.framecolor.cols/(float)width;
//             float rate_crop_height = (float)thisFrame.framecolor.rows/(float)height;
//             cropFrame = cv::Rect(   (int)((float)cropRect.x*rate_crop_width),
//                                     (int)((float)cropRect.y*rate_crop_height),
//                                     (int)((float)cropRect.width*rate_crop_width),
//                                     (int)((float)cropRect.height*rate_crop_height));
//             cal_cropFrame=false;
//         }
//         if (thisFrame.framecolor.data)
//         {
//             cvtColor(thisFrame.framecolor, thisFrame.frame,CV_RGB2GRAY);
//             thisFrame.frame = thisFrame.frame(cropFrame);
//             resize(thisFrame.frame, thisFrame.frame, cv::Size(cropRect.width, cropRect.height));
//             thisFrame.frameID = id;
//             thisFrame.frameTime = objects.getCurrentDateTime_pushSQL();
//             frameQueue.push(thisFrame);
//         }
//         nframe++;
// 		auto end = CLOCK_NOW();
// 		ElapsedTime elapsed = end - start;
// 		if (nframe % 100 == 0)
// 		{
// 			cout << " display fps=" << nframe / elapsed.count() << endl;
// 		}
//         //ToDO: interval for read frame, depend pfs
//         while (1)
//         {
//             auto endtime = CLOCK_NOW();
//             ElapsedTime elapsedtime = endtime - startime;
//             if (elapsedtime.count()*1000 < delay)
//                 this_thread::sleep_for(chrono::milliseconds(1));
//             else break;
//         }
//     }

//     free(buf);
// }

void get_IP_CAM_Address()
{

    string macaddress = IOData::GetCongfigData("MacCameraAddress:").c_str();
    string ipaddress = "";
    urlsearchip = IOData::GetCongfigData("Urlsearchip:").c_str();
    //string url = "rtsp://admin:D9844F@10.12.11.25:554/av0_0";
    remove("log_macadress.txt");

    system("arp -na >> log_macadress.txt");

    string line;
    ifstream fi("log_macadress.txt");
    if (fi.is_open())
    {
        while (getline(fi, line))
        {

            std::size_t found = line.find(macaddress);
            if (found != std::string::npos)
            {
                int n = line.length();
                int l, r;
                for (int i = 0; i < n; i++)
                {
                    if (line[i] == '(')
                        l = i;
                    if (line[i] == ')')
                        r = i;
                }
                for (int i = l + 1; i < r; i++)
                    ipaddress += line[i];
                urlsearchip += ipaddress + ":554/av0_0";
                break;
            }
        }
        fi.close();
    }
}
//========================================

VideoReader::VideoReader(
    string url, 
    int width, 
    int height,
    Rect cropRect, 
    queue<FrameData> &queue_)
    : frameQueue(queue_)
{
    this->url = url;
    this->width = width;
    this->height = height;
    this->cropRect = cropRect;
}
void videoreader_http(string url,int width, int height, queue<FrameData> &frameQueue,Rect cropRect)
{
    ObjectUtils objects;
    VideoCapture cap;
    int nframe = 0;
    int nframe_bu=0;
	auto start = CLOCK_NOW();
    while (1)
    {
        this_thread::sleep_for(chrono::milliseconds(1));
        urlsearchip = url;
        int kn = atoi(IOData::GetCongfigData("UsedURL:").c_str());
        if (kn!=1)
            get_IP_CAM_Address();
        cout << urlsearchip<<endl;
        cap.open(urlsearchip);
        int fps = atoi(IOData::GetCongfigData("fps_video_reading:").c_str());
	    int delay = 1000/fps;
        if (cap.isOpened())
        {
            long long id = 0;
            while (1)
            {
                this_thread::sleep_for(chrono::milliseconds(1));
                auto startime = CLOCK_NOW();
                id++;
                FrameData thisFrame;
                cap >> thisFrame.framecolor; 
                if (cal_cropFrame && thisFrame.framecolor.data)
                {

                    float rate_crop_width = (float)thisFrame.framecolor.cols/(float)width;
                    float rate_crop_height = (float)thisFrame.framecolor.rows/(float)height;
                    cropFrame = cv::Rect(   (int)((float)cropRect.x*rate_crop_width),
                                            (int)((float)cropRect.y*rate_crop_height),
                                            (int)((float)cropRect.width*rate_crop_width),
                                            (int)((float)cropRect.height*rate_crop_height));
                    cal_cropFrame=false;
                }
                if (thisFrame.framecolor.data)
                {
                    if  (id%FrameIgnored1==0)
                    {
                        cvtColor(thisFrame.framecolor, thisFrame.frame,CV_RGB2GRAY);
                        thisFrame.frame = thisFrame.frame(cropFrame);
                        resize(thisFrame.frame, thisFrame.frame, cv::Size(cropRect.width, cropRect.height));
                        thisFrame.frameID = id;
                        thisFrame.frameTime = objects.getCurrentDateTime_pushSQL();
                        frameQueue.push(thisFrame);
                    }
                }
                else
                    break;

                //ToDO: printf Framefps
                nframe++;
                auto end = CLOCK_NOW();
                ElapsedTime elapsed = end - start;
                start = end;
                if (nframe % 100 == 0)
                {
                    cout << " display fps=" << (nframe-nframe_bu) / elapsed.count() << endl;
                }
                nframe_bu = nframe;
                //ToDO: interval for read frame, depend pfs
                while (1)
                {
                    auto endtime = CLOCK_NOW();
                    ElapsedTime elapsedtime = endtime - startime;
                    if (elapsedtime.count()*1000 < delay)
                        this_thread::sleep_for(chrono::milliseconds(1));
                    else break;
                }
            }
        }
        else
        {
            cout << "\nLost connect!";
            this_thread::sleep_for(chrono::milliseconds(5000));
        }
    }
}

void videoreader_ws(string url,int width, int height, queue<FrameData> &frameQueue,int FrameIgnored1,Rect cropRect)
{
    vws::open(url);
    
    ObjectUtils objects;
    long long id = 0;
    while(1)
    {
        this_thread::sleep_for(chrono::milliseconds(1));
        id++;
        FrameData thisFrame;
        thisFrame.framecolor = vws::getImageInfo().frame;
        thisFrame.timestamp = vws::getImageInfo().timestamp;
        if (cal_cropFrame && thisFrame.framecolor.data)
        {
            float rate_crop_width = (float)thisFrame.framecolor.cols/(float)width;
            float rate_crop_height = (float)thisFrame.framecolor.rows/(float)height;
            cropFrame = cv::Rect(   (int)((float)cropRect.x*rate_crop_width),
                                    (int)((float)cropRect.y*rate_crop_height),
                                    (int)((float)cropRect.width*rate_crop_width),
                                    (int)((float)cropRect.height*rate_crop_height));
            cal_cropFrame=false;
        }
        if (thisFrame.framecolor.data)
        {
            if  (id%FrameIgnored1==0)
            {
                cvtColor(thisFrame.framecolor, thisFrame.frame,CV_RGB2GRAY);
                thisFrame.frame = thisFrame.frame(cropFrame);
                resize(thisFrame.frame, thisFrame.frame, cv::Size(cropRect.width, cropRect.height));
                thisFrame.frameID = id;
                thisFrame.frameTime = objects.getCurrentDateTime_pushSQL();
                m_mtx.lock();
                frameQueue.push(thisFrame);
                m_mtx.unlock();
            }
        }
    }
}

void VideoReader::operator()()
{
    string kindurl = IOData::GetCongfigData("urlkind:").c_str();
    
    if (kindurl=="http" || kindurl=="rtsp")
    {
        videoreader_http(url,width, height, frameQueue,cropRect);
    }
    if (kindurl=="ws")
    {
        videoreader_ws(url,width, height, frameQueue,FrameIgnored1,cropRect);
    }
    // if (kindurl=="ave")
    // {
    //     getFrames(width, height, frameQueue,cropRect);
    // }

}

