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
#include "crop_char_square.h"

using namespace cv;
using namespace std;

std::vector<cv::Rect> getCharacterRect_Square(cv::Mat img_rgb) 
{	std::vector<cv::Rect> charRegions;
	Mat img, img_t1,img_t2,img_bw; // img: gray image; img_t1: binary image
	if (img_rgb.channels()==3)
	{
		cvtColor(img_rgb, img,CV_RGB2GRAY);
	}
	else
	{
		img=img_rgb;
	}
	int height_ori=img.size().height; // height of the original image
	int width_ori=img.size().width;	// width of the original image
	//prepare another binay image for filter out the non-character REC
	img_t2=img;
	cv::resize(img_t2, img_t2, cv::Size(216,160), 0, 0, CV_INTER_LINEAR);
	equalizeHist( img_t2, img_t2);
	bitwise_not(img_t2,img_t2);
	adaptiveThreshold(img_t2, img_bw, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,-15);
	bwareaopen(img_bw,200);
	bitwise_not(img_bw,img_bw);
	//---Pre processing the image with resize, equalize---
	cv::resize(img, img, cv::Size(216,160), 0, 0, CV_INTER_LINEAR);
	equalizeHist( img, img);
	bitwise_not(img,img);
	adaptiveThreshold(img, img_t1, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,1);
	bwareaopen(img_t1,500); // Delete small object
	bitwise_not(img_t1,img_t1);
	//imwrite("E:\\Plate 1\\crop_char_output\\tmp.jpg",img_t1);
	//--------------------Begin to process the image------------------------------------------
	int height=img.size().height;  // height of the resized image (160)
	int width=img.size().width;		// widthe of the resized image (216)
	int hist_h[300]; // projection of all pixel values on the horizontal direction
	int hist_hh[300];// sum over 3 pixels of hist_h;
	int project[300]; // projection of all pixel values on the vertical direction 
//	float cx=0.7;
	float cw=0.83;
	int g_max=0; // the maximum value of projection, this is used as a threshold to define if a pixel is lie on a space or a character
	for (int i=1;i<300;i++)
			{
			hist_h[i]=0;
			hist_hh[i]=0;
			}
	for (int i=(floor(height/2)-16);i<(floor(height/2)+16);i++)
		{
		for (int j=0;j<width;j++)
			{
			hist_h[i]=hist_h[i]+img_t1.at<uchar>(i, j);
			}
		}
	for (int i=(floor(height/2)-15);i<(floor(height/2)+15);i++)
		{
		hist_hh[i]=hist_h[i-1]+hist_h[i]+hist_h[i+1];
		}
	int max,index_line;  // index_line is the position where we seperate the plate into upper and lower regions
	max=0;
	index_line=0;
	for (int i=(floor(height/2)-15);i<(floor(height/2)+15);i++)
		{
		if (max<hist_hh[i])
			{
			max=hist_hh[i];
			index_line=i;
			}
		}
	int index_line_ori; // the index_line in the original image
	index_line_ori=floor (index_line*height_ori/height);
	float ratio_width=(float) width_ori/width;
	//---------------------------crop character of the upper regions---------------------------------------------------------------------------------
		for (int i=0;i<width;i++)
			{
			project[i]=0;
			}
		for (int i=0;i<width;i++)
			{
			for (int j=0;j<index_line;j++)
				{
				project[i]=project[i]+img_t1.at<uchar>(j, i);
				}
			}
	vector <space> v_space;
	//int max_index=0;
	max=0;
	for (int i=0;i<width;i++)
		{
			if (g_max<project[i])
				{
				g_max=project[i];
				}
			} 
	//	printf("index_line=%d ,gmax=%d \n",index_line,g_max);

	//-------------Calculate the potential space--------------
	int positive[300];
	int num_positive;
	positive[0]=8;//the position of the first space candidate
	num_positive=1; 
	for (int i=9;i<width-8;i++)
		{
		if (project[i]>cw*g_max)
			{
			positive[num_positive]=i;
			num_positive++;
			}
		}
	positive[num_positive]=width-8; //the position of the last space candidate
	num_positive++;
	int enlarge;	
for (int i=1;i<num_positive;i++)
{
	space tmp;
	enlarge =5; 
	if (positive[i]-positive[i-1]>35) // if the character width is bigger than a threshold => big character (not 1 or half character)
	{
		if ((positive[i-1]-enlarge)>0)
        {
        tmp.start_point=positive[i-1]-enlarge;
        }
        else
        {
        tmp.start_point=0;
        }
        if ((positive[i]+enlarge)<width-1)
        {
        tmp.end_point=positive[i]+enlarge;
        }
        else
        {
        tmp.end_point=width-1;
        }
		v_space.push_back(tmp);
	}
	else if (positive[i]-positive[i-1]>12) // if it is small character (1 or a character being cut into 2 )
	{
		if (v_space.size()==0) // if it is the first character being dectected => add anyway
		{
			if ((positive[i-1]-enlarge)>0)
			{
			tmp.start_point=positive[i-1]-enlarge;
			}
			else
			{
			tmp.start_point=0;
			}
			if ((positive[i]+enlarge)<width-1)
			{
			tmp.end_point=positive[i]+enlarge;
			}
			else
			{
			tmp.end_point=width-1;
			}
			v_space.push_back(tmp);
		}
		else if ((v_space[v_space.size()-1].end_point-v_space[v_space.size()-1].start_point)>35) // if the character before it is a big character => it cannot be a 1/2 character => add into queue
		{
			if ((positive[i-1]-enlarge)>0)
			{
			tmp.start_point=positive[i-1]-enlarge;
			}
			else
			{
			tmp.start_point=0;
			}
			if ((positive[i]+enlarge)<width-1)
			{
			tmp.end_point=positive[i]+enlarge;
			}
			else
			{
			tmp.end_point=width-1;
			}
			v_space.push_back(tmp);
		}
		else// the character before it is a small character=> check if it's number 1 or a half of character. if it is 1=> add, if it is half of character => merge into 1
		{
			int check =0; //check =0 => there is no space between current candidate and the previous character => should be merge into 1
			for (int j=v_space[v_space.size()-1].end_point-enlarge;j<positive[i-1];j++)
			{
				if (project[j]>0.95*g_max)// => there is  a space
				{
					check =1; 
					break;
				}
			}
			if (check==1) // there is a space => add new character
			{
				if ((positive[i-1]-enlarge)>0)
				{
				tmp.start_point=positive[i-1]-enlarge;
				}
				else
				{
				tmp.start_point=0;
				}
				if ((positive[i]+enlarge)<width-1)
				{
				tmp.end_point=positive[i]+enlarge;
				}
				else
				{
				tmp.end_point=width-1;
				}
				v_space.push_back(tmp);
			}
			else // check =0 => merge 2 character into 1
			{
				if ((positive[i]+enlarge)<width-1)
				{
					v_space[v_space.size()-1].end_point=positive[i]+enlarge;
				}
				else
				{
					v_space[v_space.size()-1].end_point=width-1;
				}
			}
		}
			
	}
	else if (positive[i]-positive[i-1]>3)
      {
      positive[i]=positive[i-1];
      }
}
/*	for (int i=1;i<num_positive;i++)
		{
		space tmp;
		//printf("i=%d positive[i-1] =%d\n",i,positive[i-1]);
		//int width_thres;
		if  ((positive[i-1]>width*0.5)&&(positive[i-1]<width*0.7)) // the position of the character in the upper region, we use bigger threshold for character (it can not be number '1')
			{
			enlarge=5;
			if ((positive[i]-positive[i-1])>36)
				{
				//printf("i=%d positive[i-1] =%d,positive[i]=%d\n",i,positive[i-1],positive[i]);
					if ((positive[i-1]-enlarge)>0)
					{
					tmp.start_point=positive[i-1]-enlarge;
					}
					else
					{
					tmp.start_point=0;
					}
					if ((positive[i]+enlarge)<width-1)
					{
					tmp.end_point=positive[i]+enlarge;
					}
					else
					{
					tmp.end_point=width-1;
					}
				v_space.push_back(tmp);
			}
				else if (positive[i]-positive[i-1]>3)
				{
				positive[i]=positive[i-1];
				}
			}

		else 
			{
			if ((positive[i]-positive[i-1])>3)
				{
		//		printf("i=%d positive[i-1] =%d,positive[i]=%d\n",i,positive[i-1],positive[i]);

					enlarge=5;
				if ((positive[i-1]-enlarge)>12)
					{
					tmp.start_point=positive[i-1]-enlarge;
					}
					else
					{
					tmp.start_point=12;
					}
					if ((positive[i]+enlarge)<width-12)
					{
					tmp.end_point=positive[i]+enlarge;
					}
					else
					{
					tmp.end_point=width-12;
					}
				v_space.push_back(tmp);
			}
			}
		}
*/
	int start_point,end_point;// position of the character
	for (int i=0;i<(int) v_space.size();i++)
		{
		//printf("number of character is %d\n",v_space.size());
		int num_region=0;
		if ((v_space[i].end_point-v_space[i].start_point)<60)
			{
			num_region=1;
			}
		else if ((v_space[i].end_point-v_space[i].start_point)<100)
			{
			num_region=2;
			}
		else if((v_space[i].end_point-v_space[i].start_point)<145)
			{
			num_region=3;
			}
		else if((v_space[i].end_point-v_space[i].start_point)<186)
			{
			num_region=4;
			}
		else
			num_region=5;
		if (num_region==1) // xu li cho 1 region
			{
			if (check_character(img_bw,v_space[i].start_point,v_space[i].end_point,0,index_line)==1)
				{
				start_point=(int) (ratio_width*v_space[i].start_point);
				end_point=(int) (ratio_width*v_space[i].end_point);
				Rect Rec(start_point, 0, end_point-start_point,index_line_ori);
				charRegions.push_back(Rec);
				}
			}
		else 
			{
			//printf("num_region=%d\n",num_region);
			int cut_point[6];
			cut_point[0]=v_space[i].start_point;
			cut_point[num_region]=v_space[i].end_point;
			for (int j=1;j<num_region;j++)
				{
				int mid_point=v_space[i].start_point+j*floor((v_space[i].end_point-v_space[i].start_point)/num_region);
				max=0;
				for (int k=mid_point-10;k<mid_point+10;k++)
					{
					if (max<project[k])
						{
							max=project[k];
							cut_point[j]=k;
						}
					}
				}
			for (int j=0;j<num_region;j++)
				{
			//	printf("region %d\n",j);
				if (check_character(img_bw,cut_point[j],cut_point[j+1],0,index_line)==1)
					{
				//	printf("start=%d, end=%d \n",cut_point[j],cut_point[j+1]);
					start_point=(int) (ratio_width*cut_point[j]);
					end_point=(int) (ratio_width*cut_point[j+1]);
					Rect Rec(start_point, 0, end_point-start_point,index_line_ori);
					charRegions.push_back(Rec);
					}
				}
			}
		}
	v_space.clear();
	//printf("2\n");
	Rect Rec_line(0,0,0,0);
	charRegions.push_back(Rec_line);
	//---------------------------crop character of the lower regions---------------------------------------------------------------------------------
	for (int i=0;i<width;i++)
			{
			project[i]=0;
			}
	for (int i=0;i<width;i++)
		{
		for (int j=index_line;j<height;j++)
			{
			project[i]=project[i]+img_t1.at<uchar>(j, i);
			}
		}
	max=0;
	for (int i=0;i<width;i++)
		{
		if (g_max<project[i])
			{
			g_max=project[i];
			}
		} 
	positive[0]=3;//the position of the first space candidate
	num_positive=1; 
	for (int i=1;i<width-1;i++)
		{
		if (project[i]>cw*g_max)
			{
			positive[num_positive]=i;
			num_positive++;
			}
		}
	positive[num_positive]=width-3; //the position of the last space candidate
	num_positive++;
for (int i=1;i<num_positive;i++)
{
	space tmp;
	enlarge =5; 
	if (positive[i]-positive[i-1]>35) // if the character width is bigger than a threshold => big character (not 1 or half character)
	{
		if ((positive[i-1]-enlarge)>0)
        {
        tmp.start_point=positive[i-1]-enlarge;
        }
        else
        {
        tmp.start_point=0;
        }
        if ((positive[i]+enlarge)<width-1)
        {
        tmp.end_point=positive[i]+enlarge;
        }
        else
        {
        tmp.end_point=width-1;
        }
		v_space.push_back(tmp);
	}
	else if (positive[i]-positive[i-1]>12) // if it is small character (1 or a character being cut into 2 )
	{
		if (v_space.size()==0) // if it is the first character being dectected => add anyway
		{
			if ((positive[i-1]-enlarge)>0)
			{
			tmp.start_point=positive[i-1]-enlarge;
			}
			else
			{
			tmp.start_point=0;
			}
			if ((positive[i]+enlarge)<width-1)
			{
			tmp.end_point=positive[i]+enlarge;
			}
			else
			{
			tmp.end_point=width-1;
			}
			v_space.push_back(tmp);
		}
		else if ((v_space[v_space.size()-1].end_point-v_space[v_space.size()-1].start_point)>35) // if the character before it is a big character => it cannot be a 1/2 character => add into queue
		{
			if ((positive[i-1]-enlarge)>0)
			{
			tmp.start_point=positive[i-1]-enlarge;
			}
			else
			{
			tmp.start_point=0;
			}
			if ((positive[i]+enlarge)<width-1)
			{
			tmp.end_point=positive[i]+enlarge;
			}
			else
			{
			tmp.end_point=width-1;
			}
			v_space.push_back(tmp);
		}
		else// the character before it is a small character=> check if it's number 1 or a half of character. if it is 1=> add, if it is half of character => merge into 1
		{
			int check =0; //check =0 => there is no space between current candidate and the previous character => should be merge into 1
			for (int j=v_space[v_space.size()-1].end_point-enlarge;j<positive[i-1];j++)
			{
				if (project[j]>0.95*g_max)// => there is  a space
				{
					check =1; 
					break;
				}
			}
			if (check==1) // there is a space => add new character
			{
				if ((positive[i-1]-enlarge)>0)
				{
				tmp.start_point=positive[i-1]-enlarge;
				}
				else
				{
				tmp.start_point=0;
				}
				if ((positive[i]+enlarge)<width-1)
				{
				tmp.end_point=positive[i]+enlarge;
				}
				else
				{
				tmp.end_point=width-1;
				}
				v_space.push_back(tmp);
			}
			else // check =0 => merge 2 character into 1
			{
				if ((positive[i]+enlarge)<width-1)
				{
					v_space[v_space.size()-1].end_point=positive[i]+enlarge;
				}
				else
				{
					v_space[v_space.size()-1].end_point=width-1;
				}
			}
		}
			
	}
	else if (positive[i]-positive[i-1]>3)
      {
      positive[i]=positive[i-1];
      }
}
/*	for (int i=1;i<num_positive;i++)
		{
		space tmp;
		if ((positive[i-1]<6)||(positive[i]>width-6))
			{
			if ((positive[i]-positive[i-1])>22)
				{
					if ((positive[i-1]-enlarge)>0)
					{
					tmp.start_point=positive[i-1]-enlarge;
					}
					else
					{
					tmp.start_point=0;
					}
					if ((positive[i]+enlarge)<width-1)
					{
					tmp.end_point=positive[i]+enlarge;
					}
					else
					{
					tmp.end_point=width-1;
					}
				v_space.push_back(tmp);
			}	
			}
		else
			{
			if ((positive[i]-positive[i-1])>12)
				{
					if ((positive[i-1]-enlarge)>0)
					{
					tmp.start_point=positive[i-1]-enlarge;
					}
					else
					{
					tmp.start_point=0;
					}
					if ((positive[i]+enlarge)<width-1)
					{
					tmp.end_point=positive[i]+enlarge;
					}
					else
					{
					tmp.end_point=width-1;
					}
				v_space.push_back(tmp);
			}
			}	
		} */
	for (int i=0;i<(int) v_space.size();i++)
		{
		//printf("v_space.size()=%d\n",v_space.size());
		int num_region=0;
		if ((v_space[i].end_point-v_space[i].start_point)<65)
			{
			num_region=1;
			}
		else if ((v_space[i].end_point-v_space[i].start_point)<100)
			{
			num_region=2;
			}
		else if((v_space[i].end_point-v_space[i].start_point)<145)
			{
			num_region=3;
			}
		else if((v_space[i].end_point-v_space[i].start_point)<186)
			{
			num_region=4;
			}
		else
			num_region=5;
		if (num_region==1) // xu li cho 1 region
			{
			if (check_character(img_bw,v_space[i].start_point,v_space[i].end_point,index_line,height)==1)
				{
				start_point=(int) (ratio_width*v_space[i].start_point);
				end_point=(int) (ratio_width*v_space[i].end_point);
				Rect Rec(start_point, index_line_ori, end_point-start_point,height_ori - index_line_ori - 1);
				//printf("ratio_width=%f, start_point=%d, end_point=%d,index_line_ori=%d\n",ratio_width,start_point,end_point,index_line_ori);
				charRegions.push_back(Rec);
				}
			}
		else 
			{
		//	printf("num_region=%d\n",num_region);
			int cut_point[6];
			cut_point[0]=v_space[i].start_point;
			cut_point[num_region]=v_space[i].end_point;
			for (int j=1;j<num_region;j++)
				{
				int mid_point=v_space[i].start_point+j*floor((v_space[i].end_point-v_space[i].start_point)/num_region);
				max=0;
				for (int k=mid_point-10;k<mid_point+10;k++)
					{
					if (max<project[k])
						{
							max=project[k];
							cut_point[j]=k;
						}
					}
				}
			for (int j=0;j<num_region;j++)
				{
				//printf("region %d\n",j);
				if (check_character(img_bw,cut_point[j],cut_point[j+1],index_line,height)==1)
					{
					//printf("start=%d, end=%d \n",cut_point[j],cut_point[j+1]);
					start_point=(int) (ratio_width*cut_point[j]);
					end_point=(int) (ratio_width*cut_point[j+1]);
					Rect Rec(start_point, index_line_ori, end_point-start_point,height_ori - index_line_ori - 1);
					charRegions.push_back(Rec);
					}
				}
			}
		}
	v_space.clear();

	return charRegions;
}


