#include <ctime>
#include <queue>
#include <iostream>
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <curl/curl.h>
#include "base64.cpp"
#include "connectCMS.h"



#define UT_FUNCTION
#define URL_DEFAULT     "http://demo.stmc.vp9.vn:8000/plateInfo"

using namespace std;
using namespace cv;


void postDataToCms(string vehicle_plate,string camera_id,string frametime,string encoded_plate_image, string encoded_vehicle_image,string speed,string location,string sever_address,int mode,int locationX)
{
    pr::ObjectUtils objects;
    CURLcode ret;
    CURL *hnd;
    struct curl_slist *slist1;
    std::time_t timeGetPlate = std::time(nullptr);
    timeGetPlate = timeGetPlate + 60*60*7;

    std::string jsonstr = " {\"vehicle_plate\":\" ";
    jsonstr.append(vehicle_plate);
    jsonstr.append("\",\"camera_id\":\" ");
    jsonstr.append(camera_id);
    jsonstr.append("\",\"frametime\":\" ");
    jsonstr.append(frametime);
    jsonstr.append("\",\"camera_id\":\" ");
    jsonstr.append(camera_id);
    jsonstr.append("\",\"encoded_plate_image\":\" ");
    jsonstr.append(encoded_plate_image);
    jsonstr.append("\",\"encoded_vehicle_image\":\" ");
    jsonstr.append(encoded_vehicle_image);
    jsonstr.append("\",\"speed\":\" ");
    jsonstr.append(speed);
    jsonstr.append("\",\"location\":\" ");
    jsonstr.append(location);
    jsonstr.append("\",\"mode\":\" ");
    jsonstr.append(objects.FloatToStr(mode));
    jsonstr.append("\",\"locationX\":\" ");
    jsonstr.append(objects.FloatToStr(locationX));
    jsonstr.append(" \" }");

    slist1 = NULL;
    slist1 = curl_slist_append(slist1, "Content-Type: application/json");

    hnd = curl_easy_init();
    char url[1000];
    strcpy(url, sever_address.c_str());
    // get plate
    curl_easy_setopt(hnd, CURLOPT_URL, url);
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, jsonstr.c_str());
    curl_easy_setopt(hnd, CURLOPT_USERAGENT, "VP9-ANPR");
    curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
    curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, 50L);
    curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, "POST");
    curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);

    ret = curl_easy_perform(hnd);

    curl_easy_cleanup(hnd);
    hnd = NULL;
    curl_slist_free_all(slist1);
    slist1 = NULL;
    cout << endl;
    //delete hnd;
}
void push_data_to_CMS(string vehicle_plate,string camera_id,string frametime,Mat plate_image, Mat vehicle_image,string speed,string location, string sever_address,int mode,int locationX)
{
    string encoded_plate_image;
    string encoded_vehicle_image;
    vector<uchar> buf;

    imencode(".jpg", plate_image, buf);
    uchar *enc_msg = new uchar[buf.size()];
    for (int i = 0; i < (int)buf.size(); i++)
        enc_msg[i] = buf[i];
    encoded_plate_image = base64_encode(enc_msg, buf.size());
    delete enc_msg;

    imencode(".jpg", vehicle_image, buf);
    uchar *enc_msg1 = new uchar[buf.size()];
    for (int i = 0; i < (int)buf.size(); i++)
        enc_msg1[i] = buf[i];
    encoded_vehicle_image = base64_encode(enc_msg1, buf.size());
    delete enc_msg1;

    postDataToCms(vehicle_plate,camera_id,frametime,encoded_plate_image, encoded_vehicle_image,speed,location,sever_address,mode,locationX);
}

