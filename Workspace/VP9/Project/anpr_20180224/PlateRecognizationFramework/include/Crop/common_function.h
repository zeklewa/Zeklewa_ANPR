#ifndef COMMON_FUNCTION
#define COMMON_FUNCTION

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;
using namespace std;

const int region_extend=15;
const int plate_extend=10;
const int angle_threshold=7;
const float dis_extend=10; // number of pixel to extend the plate (after plate corner cropping)

struct space 
{
int start_point;
int end_point;
int mid_point;
};
struct line_2point
{
Vec4i lines;  //vector that stores the cordinate of two end points of the line
float angle; // angle of the the line to the horizontal axis
};
struct plate_point
{
Point cn1;
Point cn2;
Point cn3;
Point cn4;
};
struct border
{
int border_left;
int border_right;
int border_up;
int border_down;
int check; //check =1 if we find atleast a border, else check=0
};

    void bwareaopen(cv::Mat& im, double size);
    Point intersection(Point p1,Point p2,Point p3, Point p4);
    //Mat fill_holes(Mat image);
    Mat thresholding (Mat img);
    Mat thresholding_color(Mat img);
    bool check_plate (Mat img);
    int intersection_vertical(line_2point l);
    float angle_horizontal(int p1x,int p1y, int p2x, int p2y);
    int check_character(Mat img,int left, int right, int up, int down);
    int intersection_horizontal(line_2point l);
    border plate_filter (Mat img);

#endif