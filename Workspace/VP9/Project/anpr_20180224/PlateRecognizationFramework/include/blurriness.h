#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <numeric>
#include <string>
#include <sstream>
#include <functional>
#include <queue>
using namespace cv;
using namespace std;

float calcBlurriness_Sobel1( Mat src )
{
    Mat Gx, Gy;
    Sobel( src, Gx, CV_32F, 1, 0 );
    Sobel( src, Gy, CV_32F, 0, 1 );
    double normGx = norm( Gx );
    double normGy = norm( Gy );
    double sumSq = normGx * normGx + normGy * normGy;
    return static_cast<float>( 10000 / ( sumSq / src.size().area() + 1e-6 ));
}

float calBlurriness_Laplacian1(Mat src)
{
	Mat img;
    if (src.channels()==3)
    {
	cvtColor(src, img,CV_RGB2GRAY);
    }
   else
    {
	img=src;
    } 
    cv::Mat M = (Mat_<double>(3, 1) << -1, 2, -1);
    cv::Mat G = cv::getGaussianKernel(3, -1, CV_64F);

    cv::Mat Lx;
    cv::sepFilter2D(img, Lx, CV_64F, M, G);

    cv::Mat Ly;
    cv::sepFilter2D(img, Ly, CV_64F, G, M);

    cv::Mat FM = cv::abs(Lx) + cv::abs(Ly);

    float focusMeasure = cv::mean(FM).val[0];
    return focusMeasure;
}

float calBlurriness_Laplacian2(Mat src)
{
    Mat img;
    if (src.channels()==3)
    {
	cvtColor(src, img,CV_RGB2GRAY);
    }
   else
    {
	img=src;
    }   
    cv::Mat lap;
    cv::Laplacian(img, lap, CV_64F);

    cv::Scalar mu, sigma;
    cv::meanStdDev(img, mu, sigma);

    float focusMeasure = sigma.val[0]*sigma.val[0];
    return focusMeasure;
}

float calBlurriness_GLVN(Mat src)
{
    cv::Scalar mu, sigma;
    cv::meanStdDev(src, mu, sigma);

    float focusMeasure = (sigma.val[0]*sigma.val[0]) / mu.val[0];
    return focusMeasure;
}
float cal_contrast(Mat src)
{
	Mat img;
	if (src.channels()==3)
	{
		cvtColor(src, img,CV_RGB2GRAY);
	}
	else
	{
	img=src;
	}
	float mean_I=0;
	int width=img.size().width;
	int height=img.size().height;
	for (int i=0;i<width;i++)
	{
		for (int j=0;j<height;j++)
		{
			mean_I=mean_I+img.at<uchar>(j, i);
		}
	}
	mean_I= mean_I/(height*width);
	float contrast=0;
		for (int i=0;i<width;i++)
	{
		for (int j=0;j<height;j++)
		{
			contrast=contrast+pow((img.at<uchar>(j, i)-mean_I),2);
		}
	}
	contrast= sqrt(contrast/(height*width));
	return (contrast);
}

float cal_brighness(Mat src)
{
	Mat img;
	if (src.channels()==3)
	{
		cvtColor(src, img,CV_RGB2GRAY);
	}
	else
	{
	img=src;
	}
	float mean_I=0;
	int width=img.size().width;
	int height=img.size().height;
	for (int i=0;i<width;i++)
	{
		for (int j=0;j<height;j++)
		{
			mean_I=mean_I+img.at<uchar>(j, i);
		}
	}
	mean_I= mean_I/(height*width);
	
	return (mean_I);
}
