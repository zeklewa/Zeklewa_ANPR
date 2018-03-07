#ifndef CROP_LONGPLATE_CORNER
#define CROP_LONGPLATE_CORNER

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

    float croping_Horizol(Mat img_src,Point &cn1, Point &cn2, Point &cn3, Point &cn4,float ratio);
#endif