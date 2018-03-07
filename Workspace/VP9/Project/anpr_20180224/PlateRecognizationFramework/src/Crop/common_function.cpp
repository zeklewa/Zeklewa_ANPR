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
#include "common_function.h"
using namespace cv;
using namespace std;



/*struct plate_corner
{
Point cn1;
Point cn2;
Point cn3;
Point cn4;
float plate_score;
Mat plate_extended;
Rect plate_content;
int type; // type= 2 => plate detected by croping_v2, type=3=> plate detected by croping v3
bool isplate;
};*/
int check_character(Mat img,int left, int right, int up, int down)
{

int result;
int check; //number of blank line
int max;
//int start,end;
result=1;
check=0;
//check 1: check so hang co qua it pixel den
for (int i=up;i<down;i++)
	{
	max=0;
	for (int j=left;j<right;j++)
		{
		if ((img.at<uchar>(i, j)<100))
			{
			max++;
			}
		}
	if (max<4)
		{
		check++;
		}
	}
if (check>0.5*(down-up)) //check gom so hang co qua it pixel den
	{
	result=0;
	}
//check 2: check so hang co toan pixel trang 
check =0;
/*for (int i=up;i<down;i++)
	{
	max=0;
	for (int j=left;j<right;j++)
		{
		if ((img.at<uchar>(i, j)<150))
			{
			max=1;
			break;
			}
		}
	if (max==0)
		{
		check++;
		}
	}*
//printf("check =%d\n",check);
if (check>0.3*(down-up)) //check la so hang gom toan pixel trang
	{
	result=0;
	}*/
//printf("left=%d, right=%d,up=%d,down=%d result=%d\n",left,right,up,down,result);
//result=1;
return (result);
}

void display_image(Mat img)
{
	namedWindow("Display window", WINDOW_AUTOSIZE);// Create a window for display.
	imshow("Display window", img);
	waitKey(0);
	cv::destroyWindow("Display window");
}
void bwareaopen(cv::Mat& im, double size)
{
    // Only accept CV_8UC1
    if (im.channels() != 1 || im.type() != CV_8U)
        return;

  //  cv::bitwise_not(im, im);
    // Find all contours
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(im.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    for (int i = 0; i <(int) contours.size(); i++)
    {
        // Calculate contour area
        double area = cv::contourArea(contours[i]);

        // Remove small objects by drawing the contour with black color
        if (area > 0 && area <= size)
            cv::drawContours(im, contours, i, CV_RGB(0, 0, 0), -1);
    }
  //  cv::bitwise_not(im, im);
}
Point intersection(Point p1,Point p2,Point p3, Point p4) // the intersection of two lines, 12 and 34
{
Point pt;
if (((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x))!=0)
	{
	pt.x=((p1.x*p2.y-p1.y*p2.x)*(p3.x-p4.x)-(p1.x-p2.x)*(p3.x*p4.y-p3.y*p4.x))/((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
	pt.y=((p1.x*p2.y-p1.y*p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x*p4.y-p3.y*p4.x))/((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
	}
else
	{
	pt.x=0;
	pt.y=0;
	}

return (pt);
}
Mat fill_holes(Mat image)
{
	//cv::Mat image = cv::imread("image.jpg", 0);

	cv::Mat image_thresh;
	cv::threshold(image, image_thresh, 125, 255, cv::THRESH_BINARY);

	// Loop through the border pixels and if they're black, floodFill from there
	cv::Mat mask;
	image_thresh.copyTo(mask);
	for (int i = 0; i < mask.cols; i++) {
	    if (mask.at<char>(0, i) == 0) {
	        cv::floodFill(mask, cv::Point(i, 0), 255, 0, 10, 10);
	    }
	    if (mask.at<char>(mask.rows-1, i) == 0) {
	        cv::floodFill(mask, cv::Point(i, mask.rows-1), 255, 0, 10, 10);
	    }
	}
	for (int i = 0; i < mask.rows; i++) {
	    if (mask.at<char>(i, 0) == 0) {
	        cv::floodFill(mask, cv::Point(0, i), 255, 0, 10, 10);
	    }
	    if (mask.at<char>(i, mask.cols-1) == 0) {
	        cv::floodFill(mask, cv::Point(mask.cols-1, i), 255, 0, 10, 10);
	    }
	}


	// Compare mask with original.
	cv::Mat newImage;
	image.copyTo(newImage);
	for (int row = 0; row < mask.rows; ++row) {
	    for (int col = 0; col < mask.cols; ++col) {
	        if (mask.at<char>(row, col) == 0) {
	            newImage.at<char>(row, col) = 255;
	        }
	    }
	}
	return newImage;
}  

Mat thresholding (Mat img)
{
	Mat img_t1; //image after thresholding, hole filling, closing,...

	adaptiveThreshold(img, img_t1, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 5,0);

	int n=img.size().height;
	int m=img.size().width;	
	int dx[4]={-1,0,1,0};
	int dy[4]={0,1,0,-1};
	queue <Point> myqueue;
	int a[200][200],check[1000][1000];
	for(int j=0;j<n;j++)
	    	for (int k=0;k<m;k++)
	    	{
	    		a[j][k]=(int)img_t1.at<uchar>(j,k);
	    		check[j][k]=1;
	        }
			 Point p,q;
			 p.x = n/2;
			 p.y = m/2;
			 int eps_range=5;
			for (int j=p.x-eps_range;j<=p.x+eps_range;j++)
			{
				for (int k=p.y-eps_range;k<=p.y+eps_range;k++)
				{
					if (a[j][k]==255)
					{
					q.x=j;q.y=k;
					check[j][k]=0;
					myqueue.push(q);
					}
				}
			}

			while (!myqueue.empty())
			{
			q=myqueue.front();
			myqueue.pop();
			for (int j=0;j<4;j++)
			{
				Point td;
				td.x = q.x+dx[j];
				td.y = q.y+dy[j];
				if ((check[td.x][td.y]==1) && (a[td.x][td.y]==255))
				{
					myqueue.push(td);
					check[td.x][td.y]=0;
				}
			}
			}
for (int j=0;j<n;j++)
		{
			for (int k=0;k<m;k++)
			{
				if (check[j][k]==0)
					img_t1.at<uchar>(j,k) = 255;
				else img_t1.at<uchar>(j,k) = 0;}
		}
		img_t1=fill_holes(img_t1);

		Mat const structure_elem = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
		morphologyEx(img_t1, img_t1,cv::MORPH_CLOSE, structure_elem);
		img_t1=fill_holes(img_t1); 

return (img_t1);
}
Mat thresholding_color(Mat img)
{
Mat img_t1; //image after thresholding, hole filling, closing,...
adaptiveThreshold(img, img_t1, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 5,-1);
//display_image(img_t1);
	int n=img.size().height;
	int m=img.size().width;	
	int dx[4]={-1,0,1,0};
	int dy[4]={0,1,0,-1};
	queue <Point> myqueue;
	int a[200][200],check[1000][1000];
	for(int j=0;j<n;j++)
	    	for (int k=0;k<m;k++)
	    	{
	    		a[j][k]=(int)img_t1.at<uchar>(j,k);
	    		check[j][k]=1;
	        }
			 Point p,q;
			 p.x = n/2;
			 p.y = m/2;
			 int eps_range=5;
			for (int j=p.x-eps_range;j<=p.x+eps_range;j++)
			{
				for (int k=p.y-eps_range;k<=p.y+eps_range;k++)
				{
					if (a[j][k]==255)
					{
					q.x=j;q.y=k;
					check[j][k]=0;
					myqueue.push(q);
					}
				}
			}

			while (!myqueue.empty())
			{
			q=myqueue.front();
			myqueue.pop();
			for (int j=0;j<4;j++)
			{
				Point td;
				td.x = q.x+dx[j];
				td.y = q.y+dy[j];
				if ((check[td.x][td.y]==1) && (a[td.x][td.y]==255))
				{
					myqueue.push(td);
					check[td.x][td.y]=0;
				}
			}
			}
for (int j=0;j<n;j++)
		{
			for (int k=0;k<m;k++)
			{
				if (check[j][k]==0)
					img_t1.at<uchar>(j,k) = 255;
				else img_t1.at<uchar>(j,k) = 0;}
		}
		img_t1=fill_holes(img_t1);

		Mat const structure_elem = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
		morphologyEx(img_t1, img_t1,cv::MORPH_CLOSE, structure_elem);
		img_t1=fill_holes(img_t1); 

return (img_t1);
}
bool check_plate (Mat img)
{
bool check;// check =1=>plate; check =0=> not plate
check =1;
int width;
int height;
width=img.size().width;
height=img.size().height;
//printf("width=%d, height=%d \n",width,height);
//display_image(img);
Mat img_bin;
Mat img_bin2,img_canny;
bitwise_not(img,img);
adaptiveThreshold(img, img_bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,-5);
img_bin2=img_bin;
bitwise_not(img_bin,img_bin);
//display_image(img_canny);
//display_image(img_bin);
int mid_index;
vector<Vec4i> lines;
//-------------Check Horizontal Direction---------------
int hist_horizontal[300];
for (int i=(floor(height/2)-7);i<(floor(height/2)+7);i++)
	{
	hist_horizontal[i]=0;
	}
int max=0;
mid_index=0;
for (int i=(floor(height/2)-7);i<(floor(height/2)+7);i++)
	{
	for (int j=0;j<width;j++)
		{
		if (img_bin.at<uchar>(i, j)>200)
			{
			hist_horizontal[i]=hist_horizontal[i]+1;
			}
		}
	if (max<hist_horizontal[i])
		{
		max=hist_horizontal[i];
		mid_index=i;
		}
	}
if (max<width*0.84)
	{
	check=0;
	}
int hist_vertical_up[300];
int hist_vertical_down[300];

if (check==1)// check vertical direction
	{
//	printf("mid_index=%d\n",mid_index);
	for (int i=0;i<width;i++)
		{
		hist_vertical_up[i]=0;
		hist_vertical_down[i]=0;
		}
	for (int i=0;i<width;i++)
		{
		for (int j=0;j<mid_index;j++)
			{
			if (img_bin.at<uchar>(j, i)>200)
				{
				hist_vertical_up[i]=hist_vertical_up[i]+1;
				}
			}
		for (int j=mid_index;j<height;j++)
			{
			if (img_bin.at<uchar>(j, i)>200)
				{
				hist_vertical_down[i]=hist_vertical_down[i]+1;
				}

			}
		}
	int count=0;
	for (int i=0;i<width;i++)
		{
		if (hist_vertical_up[i]>mid_index*0.9)
			{
			hist_vertical_up[i]=1;
			count++;
			}
		}
	//printf("count_up=%d \n",count);

	if (count<10)
	{check=0;}
	count=0;
	for (int i=0;i<width;i++)
		{
		if (hist_vertical_down[i]>(height-mid_index)*0.9)
			{
			hist_vertical_down[i]=1;
			count++;
			}
		}
	//printf("count_down=%d \n",count);
	if (count<10)
	{check=0;}
}
//-------Check vertical line-------------
/*HoughLinesP(img_canny, lines, 1, CV_PI/180, height/4, height/4, height/6);
double angle_v = 0;
unsigned nb_lines_v = 0;
double angle_h = 0;
unsigned nb_lines_h = 0;
Mat cdst;
cdst=img;
cvtColor(cdst, cdst, CV_GRAY2BGR);
for (int i=0;i<lines.size();i++)
	{
	line( cdst, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 1, CV_AA);
	float angle_tmp=angle_horizontal(lines[i][0],lines[i][1],lines[i][2],lines[i][3]);
	if ((angle_tmp>50)||(angle_tmp<-50))
		{
		angle_h=angle_h+abs(angle_tmp);
		nb_lines_h++;
		}
	else if ((angle_tmp<20)&&(angle_tmp>-20))
		{
		angle_v=angle_v+abs(angle_tmp);
		nb_lines_v++;
		}
	}
if (nb_lines_h!=0)
	{
	angle_h=angle_h/nb_lines_h;
	}
else
	{
	angle_h=0;
	}
if (nb_lines_v!=0)
	{
	angle_v=angle_v/nb_lines_v;
	}
else
	{
	angle_v=0;
	}
if (angle_h<82)
	{
	check=0;
	}*/
//printf("max=%d width=%d check=%d \n",max,width,check);
//display_image(img_bin2);
//display_image(cdst);

return check;
}
int min_aTung (int x1,int x2,int x3, int x4, int x5, int x6, int x7, int x8)
{
int min_aTung;
min_aTung=min(min(min(x1,x2),min(x3,x4)),min(min(x5,x6),min(x7,x8)));
return (min_aTung);
}
int intersection_vertical(line_2point l)  // find the intersection of a line to the vertical axis
{
int intersection;
Point p1,p2,p3,p4;
p1.x=l.lines[0];
p1.y=l.lines[1];
p2.x=l.lines[2];
p2.y=l.lines[3];
p3.x=0;
p3.y=0;
p4.x=0;
p4.y=100;
intersection=((p1.x*p2.y-p1.y*p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x*p4.y-p3.y*p4.x))/((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
return (intersection);
}
float angle_horizontal(int p1x,int p1y, int p2x, int p2y)
{
double x1 = (float)(p1x - p2x);
double y1 = (float)(p1y - p2y);
double angle;
if (x1 != 0)
angle = atan(y1/x1);
else
angle = 3.14159 / 2.0; // 90 degrees
angle = angle * 180.0 / 3.14159;
//printf("angle=%f \n",angle);
return angle;
}
int intersection_horizontal(line_2point l)  // find the intersection of a line to the horizontal axis
{
int intersection;
Point p1,p2,p3,p4;
p1.x=l.lines[0];
p1.y=l.lines[1];
p2.x=l.lines[2];
p2.y=l.lines[3];
p3.x=0;
p3.y=0;
p4.x=100;
p4.y=0;
intersection=((p1.x*p2.y-p1.y*p2.x)*(p3.x-p4.x)-(p1.x-p2.x)*(p3.x*p4.y-p3.y*p4.x))/((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
return (intersection);
}
border plate_filter (Mat img)
{
int max_index=0;
border tmp;
Mat img_bin;
bitwise_not(img,img);
adaptiveThreshold(img, img_bin, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,-5);
bitwise_not(img_bin,img_bin);
//imwrite("E:\\Plate 1\\Crop4gocOutput\\tmp.jpg",img_bin);
//display_image(img_bin);
int height=img.size().height;
int width=img.size().width;
tmp.border_down=height-1;
tmp.border_up=0;
tmp.border_left=0;
tmp.border_right=width-1;
tmp.check=0;
int*  hist_v=new int[width];
int* hist_h=new int[height];
for (int i=0;i<width;i++)
	{
	hist_v[i]=0;
	for (int j=0;j<height;j++)
		{
		if (img_bin.at<uchar>(j, i)>200)
			{
			hist_v[i]++;
			}
		}
	//printf("hist[%d]=%d height%d\n",i,hist_v[i],height);
	}
for (int i=0;i<height;i++)
	{
	hist_h[i]=0;
	for (int j=0;j<width;j++)
		{
		if (img_bin.at<uchar>(i, j)>200)
			{
			hist_h[i]++;
			}
		}
	}
int g_max=0;
int max_check_1,max_check_2;
if (height>31)
	{
	max_check_1=(int)(height/2-15);
	max_check_2=(int)(height/2+15);
	}
else
	{
	max_check_1=0;
	max_check_2=height;
	}
for (int i=max_check_1;i<max_check_2;i++)
	{
		if (g_max<hist_h[i])
			{
			g_max=hist_h[i];
			max_index=i;
			}
	}
//printf("max_index=%d\n",max_index);
for (int i=0;i<width;i++)
	{
	if ((hist_v[i]<height*0.25)&&(img_bin.at<uchar>(max_index, i)<50))
		{
		hist_v[i]=0;
		}
	else
		{
		hist_v[i]=1;
		}
	//printf("hist[%d]=%d, pixel value=%d \n",i,hist_v[i],img_bin.at<uchar>(max_index, i));
	}
for (int i=(int) 2*width/3;i<width;i++)
	{
	if (hist_v[i]==0)
		{
		tmp.border_right=i;
		//printf("tmp.border_right=%d, %d\n",tmp.border_right,2*width/3);
		break;
		}
	}
for (int i=(int) width/3;i>0;i--)
	{
	if (hist_v[i]==0)
		{
		tmp.border_left=i;
		break;
		}
	}
for (int i=0;i<height;i++)
	{
	if (hist_h[i]<width*0.25)
		{
		hist_h[i]=0;
		}
	else
		{
		hist_h[i]=1;
		}
	}
for (int i=(int) height/3;i>0;i--)
	{
	if (hist_h[i]==0)
		{
		tmp.border_up=i;
		break;
		}
	}
for (int i=(int)2*height/3;i<height;i++)
	{
	if (hist_h[i]==0)
		{
		tmp.border_down=i;
		break;
		}
	}
tmp.border_right=width-1-tmp.border_right;
tmp.border_down=height-1-tmp.border_down;
if ((tmp.border_left!=0)||(tmp.border_right!=0)||(tmp.border_down!=0)||(tmp.border_up!=0))
	{
	tmp.check=1;
	}
else
	{
	tmp.check=0;
	}
//printf("left=%d, right=%d, up=%d, down=%d\n",tmp.border_left,tmp.border_right,tmp.border_up,tmp.border_down);
delete hist_h;
delete hist_v;
return (tmp);
}
