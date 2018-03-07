#ifndef OJECT_UTILS_DATA_H
#define OJECT_UTILS_DATA_H

#include <iostream>
#include <vector>
#include "PlateRecognizator.h"
#include "IOData.h"

using namespace cv;
using namespace std;

const int maxplateprocess = MaxPlateProcessCNN*2;

#define CLOCK_NOW std::chrono::system_clock::now
namespace pr
{
class ObjectUtils
{
private:
  Mat img;
  string test;  

public:
  
  void wtfile_demo(string link,string content);
  void printf_result(vehicle plateprocess[maxplateprocess],int num_plates, Rect cropRect,int mode);
  string FloatToStr(float tt);
  string getCurrentDate();
  string getCurrentDateTime();
  string getCurrentDateTime_pushSQL();
  bool isplate(cv::Mat img);
  long convert_time(string s);
  int minResult(int x, int y, int z);
  int editDist(string str1, string str2, int m, int n);
  vehicle Find_best_result(vehicle plateprocess[maxplateprocess],int num_plates);
  float calcBlurriness(const Mat &src);
  linear_equation calculation_line_equa(int x1, int y1, int x2, int y2);
  float distance_point_to_line(linear_equation line, int x, int y);
  bool filter_result(vehicle a);
  string kindcascade(int mode);
  int kindsizecascade(int mode);
};
}
#endif
