#ifndef CONNECT_CMS
#define CONNECT_CMS

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include "ObjectUtils.h"

using namespace cv;
using namespace std;



void push_data_to_CMS(string vehicle_plate,string camera_id,string frametime,Mat plate_image, Mat vehicle_image,string speed,string location,string sever_address,int mode,int locationX);

#endif