#ifndef CROPPING_V2
#define CROPPING_V2

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

    float croping_v2(Mat img_src,Point &cn1, Point &cn2, Point &cn3, Point &cn4,float ratio);
#endif