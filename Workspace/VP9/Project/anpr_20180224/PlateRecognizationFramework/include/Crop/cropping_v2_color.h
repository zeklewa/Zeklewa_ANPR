#ifndef CROPPING_V2_COLOR
#define CROPPING_V2_COLOR

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

    float croping_v2_color(Mat img_src,Point &cn1, Point &cn2, Point &cn3, Point &cn4,float ratio);
#endif