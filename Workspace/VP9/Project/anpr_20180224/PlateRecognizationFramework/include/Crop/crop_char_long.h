#ifndef CROP_CHAR_LONG
#define CROP_CHAR_LONG

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

    std::vector<cv::Rect> getCharacterRect_LongPlate(cv::Mat& img_rgb);

#endif