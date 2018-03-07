#ifndef CROP
#define CROP

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

struct plate_corner
{
    Point cn1;
    Point cn2;
    Point cn3;
    Point cn4;
    float plate_score;
    Mat plate_extended;
    Rect plate_content;
    bool isplate;
    float blurriness;
    float brightness;
    int type;
    float plate_ratio;
};

    vector<cv::Rect> crop_plate_char(Mat img_rgb, int mode);
    plate_corner crop_plate_corner (Mat img_rgb,int mode,float ratio);



#endif