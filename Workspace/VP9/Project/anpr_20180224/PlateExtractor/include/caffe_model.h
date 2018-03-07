#ifndef __SIMPLE_CNN_HPP
#define __SIMPLE_CNN_HPP
#include <iostream>

#include <vector>
#include <string>
#include <caffe2/core/init.h>
#include <caffe2/core/predictor.h>
#include <caffe2/utils/proto_utils.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

class CNN
{
private:
  caffe2::NetDef init_net, predict_net;
  caffe2::Predictor *predictor;
  caffe2::TensorCPU mat2data(const cv::Mat &img);
  int width, height;
public:
  CNN(std::string init_mode, std::string predict_mode, int width = 28, int height = 28);
  std::vector<float> predict(const cv::Mat &img);
};

// Reads a model graph definition from disk, and creates a session object you
// can use to run it.

#endif // __SIMPLE_CNN_HPP
