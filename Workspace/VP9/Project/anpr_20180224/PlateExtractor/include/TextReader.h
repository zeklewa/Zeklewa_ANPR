#ifndef DEEP_LEARNING_H
#define DEEP_LEARNING_H

#include <opencv2/opencv.hpp>
//#include <opencv2/ml.hpp>
// #include "keras_model.h"
#include "caffe_model.h"
struct charWithPro {
	float pro;
	char c;
	float char_pro;
	char char_c;
	float num_pro;
	char num_c;
	float neg_pro;
	char neg_c;
	float cnn_out[33];
};
struct stringWithPro {
	float pro;
	std::string PlateStr;
	int num_char;
	std::string proEachChar;
	std::vector<charWithPro> vector_cwp; 	
};

namespace pr {
	class TextReader {
	public:
		// keras::KerasModel *m;
		// keras::DataChunk *sample;
		CNN* model;
	public:
		// TextReader(std::string dumpfile);
		TextReader(std::string init_model, std::string predict_model);
		std::vector< std::vector<std::vector<float > > > mat2data(cv::Mat img);
		charWithPro test_keras2cpp_with_mat(cv::Mat img);
		stringWithPro GetPlateString(std::vector<cv::Rect> charRects, cv::Mat& img,int plate_mode);
		stringWithPro GetPlateString_square(std::vector<cv::Rect> charRects, cv::Mat& img);
		stringWithPro GetPlateString_long(std::vector<cv::Rect> charRects, cv::Mat& img);
		stringWithPro GetPlateString_square_blue(std::vector<cv::Rect> charRects, cv::Mat& img);
		stringWithPro GetPlateString_long_blue(std::vector<cv::Rect> charRects, cv::Mat& img);
		stringWithPro GetPlateString_square_red(std::vector<cv::Rect> charRects, cv::Mat& img);
		stringWithPro GetPlateString_long_red(std::vector<cv::Rect> charRects, cv::Mat& img);
	};
}

#endif