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
#include "cropping_v3.h"
using namespace cv;
using namespace std;



float croping_v3(Mat img,Point &corner1, Point &corner2, Point &corner3, Point &corner4,float ratio)
{
//printf("v3\n");
Mat img_canny;
Mat img_sobel,grad;
Mat img1=thresholding(img); 
Mat img_t1;
//adaptiveThreshold(img, img_t1, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 9,0 );
//display_image(img_t1);
Canny(img1,img_canny,50,150,3);
//display_image(img_canny);


//-----------Edge Sobel detection--------------------
int scale = 1;
int delta = 0;
int ddepth = CV_16S;
GaussianBlur( img, img, Size(3,3), 0, 0, BORDER_DEFAULT );
Mat grad_x, grad_y;
Mat abs_grad_x, abs_grad_y;
Sobel( img, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
convertScaleAbs( grad_x, abs_grad_x );
Sobel( img, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
convertScaleAbs( grad_y, abs_grad_y );
addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad );
//display_image(grad);

threshold( grad, img_sobel, 90,255,THRESH_BINARY );

int height=img.size().height;
int width=img.size().width;
//-------------Lines detection with hough transform-------------------
vector<Vec4i> lines_canny;
vector<Vec4i>lines_sobel;
vector<line_2point> lines;
vector<line_2point> lines_vl;
vector<line_2point> lines_vr;
vector<line_2point> lines_hu;
vector<line_2point> lines_hd;
Mat dst, cdst;
cvtColor(img, cdst, CV_GRAY2BGR);
int height_approx,width_approx; // the approximate size of the plate (size before region extend)
height_approx=height-2*region_extend;
width_approx=width-2*region_extend;
/*//-------------------Laplacian Edge detection--------------------------
 int kernel_size = 3;
GaussianBlur( img, img, Size(3,3), 0, 0, BORDER_DEFAULT );
   Mat abs_dst;

  Laplacian( img, dst, ddepth, kernel_size, scale, delta, BORDER_DEFAULT );
  convertScaleAbs( dst, abs_dst );
  threshold( abs_dst, abs_dst, 60,255,THRESH_BINARY );
  display_image( abs_dst);
//------------------ End of Laplacian Edge Detection*/

//printf("height=%d width=%d height_approx=%d, width_approx=%d\n",height,width,height_approx,width_approx);
HoughLinesP(img_sobel, lines_sobel, 2, CV_PI/180, height_approx/2, height_approx/2, height_approx);// line detected by sobel edge detection
HoughLinesP(img_canny, lines_canny, 2, CV_PI/180,height_approx/2, height_approx/2, height_approx); // line detected by canny edge detection
for (int i=0;i<(int) lines_sobel.size();i++)
	{
	line_2point tmp;
	Vec4i l = lines_sobel[i];
	tmp.lines=lines_sobel[i];
	tmp.angle=angle_horizontal(l[0],l[1],l[2],l[3]);
	lines.push_back(tmp);
	}
for (int i=0;i<(int) lines_canny.size();i++)
	{
	line_2point tmp;
	Vec4i l = lines_canny[i];
	tmp.lines=lines_canny[i];
	tmp.angle=angle_horizontal(l[0],l[1],l[2],l[3]);
	lines.push_back(tmp);
	}
for (int i=0;i<(int) lines.size();i++)
	{
	if ((lines[i].angle<20)&&(lines[i].angle>-20))
		{
		if ((lines[i].lines[1]<height/2)&&(lines[i].lines[3]<height/2))
			{
			//check if there exist duplicated line 
			int check=1;
			int cross=intersection_vertical(lines[i]);
			//printf("hu cross=%d, angle=%f\n",cross,lines[i].angle);
			for (int j=0;j<(int) lines_hu.size();j++)
				{
				if ((abs(abs(lines[i].angle)-abs(lines_hu[j].angle))<2)&&(abs(cross-intersection_vertical(lines_hu[j]))<3))
					{
					check=0;
					break;
					}
				}
			if (check==1)
				{
				lines_hu.push_back(lines[i]);
				line( cdst, Point(lines[i].lines[0], lines[i].lines[1]), Point(lines[i].lines[2], lines[i].lines[3]), Scalar(0,0,255), 1, CV_AA);
				//display_image(cdst);
				}
			}
		else if ((lines[i].lines[1]>height/2)&&(lines[i].lines[3]>height/2))
			{
			//check if there exist duplicated line 
			int check=1;
			int cross=intersection_vertical(lines[i]);
			//printf("hd cross=%d, angle=%f\n",cross,lines[i].angle);
			for (int j=0;j<(int) lines_hd.size();j++)
				{
				if ((abs(abs(lines[i].angle)-abs(lines_hd[j].angle))<2)&&(abs(cross-intersection_vertical(lines_hd[j]))<3))
					{
					check=0;
					break;
					}
				}
			if (check==1)
				{
				lines_hd.push_back(lines[i]);
				line( cdst, Point(lines[i].lines[0], lines[i].lines[1]), Point(lines[i].lines[2], lines[i].lines[3]), Scalar(0,0,255), 1, CV_AA);
				//display_image(cdst);
				}
			}
		}
	if ((lines[i].angle>70)||(lines[i].angle<-70))
		{
		if ((lines[i].lines[0]<width/3)&&(lines[i].lines[2]<width/3))
			{
			//check if there exist duplicated line 
			int check=1;
			int cross=intersection_horizontal(lines[i]);
			//printf("vl cross=%d, angle=%f\n",cross,lines[i].angle);
			for (int j=0;j<(int) lines_vl.size();j++)
				{
				if ((abs(abs(lines[i].angle)-abs(lines_vl[j].angle))<2)&&(abs(cross-intersection_horizontal(lines_vl[j]))<3))
					{
					check=0;
					break;
					}
				}
			if (check==1)
				{
				lines_vl.push_back(lines[i]);
				line( cdst, Point(lines[i].lines[0], lines[i].lines[1]), Point(lines[i].lines[2], lines[i].lines[3]), Scalar(0,0,255), 1, CV_AA);
				//display_image(cdst);
				}
			}
		else if ((lines[i].lines[0]>2*width/3)&&(lines[i].lines[2]>2*width/3))
			{
			//check if there exist duplicated line 
			int check=1;
			int cross=intersection_horizontal(lines[i]);
			//printf("vr cross=%d, angle=%f\n",cross,lines[i].angle);
			for (int j=0;j<(int) lines_vr.size();j++)
				{
				if ((abs(abs(lines[i].angle)-abs(lines_vr[j].angle))<2)&&(abs(cross-intersection_horizontal(lines_vr[j]))<3))
					{
					check=0;
					break;
					}
				}
			if (check==1)
				{
				lines_vr.push_back(lines[i]);
				line( cdst, Point(lines[i].lines[0], lines[i].lines[1]), Point(lines[i].lines[2], lines[i].lines[3]), Scalar(0,0,255), 1, CV_AA);
				//display_image(cdst);
				}
			}
		}
	}
//printf("number of vl=%d, vr=%d, hu=%d, hd=%d \n",lines_vl.size(),lines_vr.size(),lines_hu.size(),lines_hd.size());
//int type=0;//type =1:miss vl, type =2: miss vr, type =3: miss hu, type =4: miss hd
//------------erase duplicate lines--------------------------------------
// if (lines_vl.size()>0)
// 	{
// 	num_lines++;
// 	}
// else 
// 	{
// 	type=1;
// 	}
// if (lines_vr.size()>0)
// 	{
// 	num_lines++;
// 	}
// else
// 	{
// 	type=2;
// 	}
// if (lines_hu.size()>0)
// 	{
// 	num_lines++;
// 	}
// else 
// 	{
// 	type=3;
// 	}
// if (lines_hd.size()>0)
// 	{
// 	num_lines++;
// 	}
// else 
// 	{
// 	type=4;
// 	}
//printf("lines.size=%d \n",lines.size());
//cout<<"type of line detection is "<<type<<endl;
//printf("angle_threshold=%d\n",angle_threshold);

//line( cdst, Point(lines[i].lines[0], lines[i].lines[1]), Point(lines[i].lines[2], lines[i].lines[3]), Scalar(0,0,255), 1, CV_AA);
//display_image(img_sobel);
//display_image(img_canny);
//display_image(cdst);

vector<Vec2i> vertical_pair; //candidate for vertical pairs of lines
vector<Vec2i> horizontal_pair; //candidate for horizontal pairs of lines
for (int i=0;i<(int) lines_vl.size();i++)
	{
	for (int j=0;j<(int) lines_vr.size();j++)
		{
		//printf("angle[vl]=%f, angle[vr]=%f\n",lines_vl[i].angle,lines_vr[j].angle);
		if (abs(abs(lines_vl[i].angle)-abs(lines_vr[j].angle))<angle_threshold)
			{
			Vec2i tmp;
			tmp[0]=i;
			tmp[1]=j;
			vertical_pair.push_back(tmp);
			//cout<<"ver angle1="<<lines_vl[i].angle<<"\t"<<"angle2="<<lines_vr[j].angle<<endl;
			}
		}
	}
for (int i=0;i<(int) lines_hu.size();i++)
	{
	for (int j=0;j<(int) lines_hd.size();j++)
		{
		if (abs(abs(lines_hu[i].angle)-abs(lines_hd[j].angle))<angle_threshold)
			{
			Vec2i tmp;
			tmp[0]=i;
			tmp[1]=j;
			horizontal_pair.push_back(tmp);
			//cout<<" ho angle1="<<lines_hu[i].angle<<"\t"<<"angle2="<<lines_hd[j].angle<<endl;
			}
		}
	}
int count=0;
Mat img_clone;
float min_distance=9999;
plate_point plate_tmp;

//printf("number of horizontal pair=%d, vertical pair=%d \n",horizontal_pair.size(),vertical_pair.size());
for (int i=0;i<(int) vertical_pair.size();i++)
//for (int i=0;i<1;i++)
	{
	for (int j=0;j<(int) horizontal_pair.size();j++)
//for (int j=2;j<3;j++)
		{
		
		Point p11,p12,p21,p22,p31,p32,p41,p42;
		Point cn1,cn2,cn3,cn4;
		p11.x=lines_vl[vertical_pair[i][0]].lines[0];
		p11.y=lines_vl[vertical_pair[i][0]].lines[1];
		p12.x=lines_vl[vertical_pair[i][0]].lines[2];
		p12.y=lines_vl[vertical_pair[i][0]].lines[3];
		p31.x=lines_vr[vertical_pair[i][1]].lines[0];
		p31.y=lines_vr[vertical_pair[i][1]].lines[1];
		p32.x=lines_vr[vertical_pair[i][1]].lines[2];
		p32.y=lines_vr[vertical_pair[i][1]].lines[3];
		p21.x=lines_hd[horizontal_pair[j][1]].lines[0];
		p21.y=lines_hd[horizontal_pair[j][1]].lines[1];
		p22.x=lines_hd[horizontal_pair[j][1]].lines[2];
		p22.y=lines_hd[horizontal_pair[j][1]].lines[3];
		p41.x=lines_hu[horizontal_pair[j][0]].lines[0];
		p41.y=lines_hu[horizontal_pair[j][0]].lines[1];
		p42.x=lines_hu[horizontal_pair[j][0]].lines[2];
		p42.y=lines_hu[horizontal_pair[j][0]].lines[3];
		cn1=intersection(p11,p12,p41,p42);
		cn2=intersection(p11,p12,p21,p22);
		cn3=intersection(p21,p22,p31,p32);
		cn4=intersection(p31,p32,p41,p42);
		float length_vl,length_hu,ratio_i,length_hd,length_vr;
		float angle_vl,angle_vr,angle_hu,angle_hd;
		length_vl=norm(cn1-cn2);
		length_hu=norm(cn1-cn4);
		length_hd=norm(cn2-cn3);
		length_vr=norm(cn3-cn4);
		float ratio_vertical,ratio_horizontal;
		if (max(length_vl,length_vr)!=0)
			{
			ratio_vertical=min(length_vr,length_vl)/max(length_vr,length_vl);
			}
		else
			{
			ratio_vertical=0;
			}
		if (max(length_hu,length_hd)!=0)
			{
			ratio_horizontal=min(length_hu,length_hd)/max(length_hu,length_hd);
			}
		else
			{
			ratio_horizontal=0;
			}
		if ((length_vl+length_vr)!=0)
			{
			ratio_i=(length_hu+length_hd)/(length_vl+length_vr);
			}
		else 
			{
			ratio_i=0;
			}
		//printf("vl=%f, vr=%f,hu=%f,hd=%f,height=%d, width=%d, ratio=%f,ratio_vertical=%f, ratio_hori=%f \n",length_vl,length_vr,length_hu,length_hd,height_approx,width_approx,ratio,ratio_vertical,ratio_horizontal);
		if ((length_vl<1.2*height_approx)&&(length_vl>0.5*height_approx)&&(length_hu<1.2*width_approx)&&(length_hu>0.5*width_approx)&&(length_hd<1.2*width_approx)&&(length_hd>0.5*width_approx)&&(length_vr<1.2*height_approx)&&(length_vr>0.5*height_approx)&&(ratio_i<1.65)&&(ratio_i>1)&&(ratio_vertical>0.85)&&(ratio_horizontal>0.85))
		//if ((length_vl<(height-plate_extend))&&(length_vl>0.6*height_approx)&&(length_hu<(width-plate_extend))&&(length_hu>0.6*width_approx)&&(length_hd<(width-plate_extend))&&(length_hd>0.6*width_approx)&&(length_vr<(height-plate_extend))&&(length_vr>0.6*height_approx)&&(ratio<1.55)&&(ratio>1.25))
			{
			//img_clone=cdst;
			Mat img_affine;
			/*line( img_clone, cn1, cn2, Scalar(0,0,255), 1, CV_AA);
			line( img_clone, cn2, cn3, Scalar(0,0,255), 1, CV_AA);
			line( img_clone, cn3, cn4, Scalar(0,0,255), 1, CV_AA);
			line( img_clone, cn4, cn1, Scalar(0,0,255), 1, CV_AA);*/
			cv::Point2f a2(0, 0), b2(width, 0), c2(width, height),d2(0,height);
			cv::Point2f src_point[] = {cn1, cn2,cn3,cn4};
			cv::Point2f dst_point[] = {a2, d2, c2,b2};
			cv::Mat warpMat = getAffineTransform(src_point, dst_point);
			cv::warpAffine(img,img_affine, warpMat, img.size());
			//display_image(img_clone);
			//if (check_plate(img_affine))
			if (check_plate(img_affine))			
				{
				angle_vl=lines_vl[vertical_pair[i][0]].angle;
				angle_vr=lines_vr[vertical_pair[i][1]].angle;
				angle_hu=lines_hu[horizontal_pair[j][0]].angle;
				angle_hd=lines_hd[horizontal_pair[j][1]].angle;
				float v_distance=0; // the distance of angle between 2 vertical lines
				float h_distance=0; 
				if ((angle_vr!=0)||(angle_vl!=0))
					{
					v_distance=abs(abs(angle_vl)-abs(angle_vr))/angle_threshold;
					}
				else
					{
					v_distance=0;
					}
				if ((angle_hu!=0)||(angle_hd!=0))
					{
					h_distance=abs(abs(angle_hu)-abs(angle_hd))/angle_threshold;
					}
				else
					{
					h_distance=0;
					}
				float ratio_distance;
				ratio_distance=abs(ratio_i-ratio)/ratio;
				float total_distance;
				total_distance=v_distance+h_distance+ratio_distance;
				//printf("distance_v=%f, distance_h=%f, ratio=%f total_distance=%f\n",v_distance,h_distance,ratio,total_distance);
				if (total_distance<min_distance)
					{
					min_distance=total_distance;
					plate_tmp.cn1=cn1;
					plate_tmp.cn2=cn2;
					plate_tmp.cn3=cn3;
					plate_tmp.cn4=cn4;
					}
				count++;
				}
			}
		}
	}
//printf("count= %d min_distance=%f \n",count,min_distance);
img_clone=cdst;
/*line( img_clone, plate_tmp.cn1, plate_tmp.cn2, Scalar(0,0,255), 1, CV_AA);
line( img_clone, plate_tmp.cn2, plate_tmp.cn3, Scalar(0,0,255), 1, CV_AA);
line( img_clone, plate_tmp.cn3, plate_tmp.cn4, Scalar(0,0,255), 1, CV_AA);
line( img_clone, plate_tmp.cn4, plate_tmp.cn1, Scalar(0,0,255), 1, CV_AA);
display_image(img_clone);*/
corner1=plate_tmp.cn1;
corner2=plate_tmp.cn2;
corner3=plate_tmp.cn4;
corner4=plate_tmp.cn3;
return (min_distance);
}