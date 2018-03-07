#ifndef CROP_CHAR_SQUARE
#define CROP_CHAR_SQUARE

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>

using namespace cv;
using namespace std;

    std::vector<cv::Rect> getCharacterRect_Square(cv::Mat img_rgb);
#endif