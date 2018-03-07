#ifndef PUBLICVALUE
#define PUBLICVALUE

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

static float ratio_crop[4]={1.3,4.2,1.3,4.2};
static const long num_plate_estimate_square_cascade[4] = {150,30,150,30};
static int num_scale_cascade[4] = {3,2,3,2};
static bool showVideo = true;


#endif