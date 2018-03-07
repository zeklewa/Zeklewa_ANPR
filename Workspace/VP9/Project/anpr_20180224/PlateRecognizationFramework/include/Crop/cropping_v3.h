#ifndef CROPPING_V3
#define CROPPING_V3

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

float croping_v3(Mat img,Point &corner1, Point &corner2, Point &corner3, Point &corner4,float ratio);
#endif