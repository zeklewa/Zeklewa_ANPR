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
#include "crop_char_long.h"
std::vector<cv::Rect> getCharacterRect_LongPlate(cv::Mat& img_rgb)
{
std::vector<cv::Rect> charRegions;
Mat img, img_t1,img_t2,img_bw; // img: gray image; img_t1: binary image
//cvtColor(img_rgb, img,CV_RGB2GRAY);
int height_ori=img_rgb.size().height; // height of the original image
int width_ori=img_rgb.size().width;  // width of the original image
//prepare another binay image for filter out the non-character REC
img_t2=img_rgb;
img=img_rgb;
cv::resize(img_t2, img_t2, cv::Size(376,88), 0, 0, CV_INTER_LINEAR);
equalizeHist( img_t2, img_t2);
bitwise_not(img_t2,img_t2);
adaptiveThreshold(img_t2, img_bw, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,-15);
//bwareaopen(img_bw,200);
bitwise_not(img_bw,img_bw);
//---Pre processing the image with resize, equalize---
cv::resize(img, img, cv::Size(376,88), 0, 0, CV_INTER_LINEAR);
equalizeHist( img, img);
bitwise_not(img,img);
adaptiveThreshold(img, img_t1, 255, CV_ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 21,1);
bwareaopen(img_t1,500); // Delete small object
bitwise_not(img_t1,img_t1);
//imwrite("E:\\Plate 1\\crop_char_output\\tmp.jpg",img_t1);
//display_image(img_t1);
//--------------------Begin to process the image------------------------------------------
int height=img.size().height;  // height of the resized image (160)
int width=img.size().width;    // widthe of the resized image (216)
//int hist_h[400]; // projection of all pixel values on the horizontal direction
//int hist_hh[400];// sum over 3 pixels of hist_h;
int project[400]; // projection of all pixel values on the vertical direction 
float cw=0.83;
int g_max=0; // the maximum value of projection, this is used as a threshold to define if a pixel is lie on a space or a character
float ratio_width=(float) width_ori/width;
//---------------------------crop character of the upper regions---------------------------------------------------------------------------------
  for (int i=0;i<width;i++)
    {
    project[i]=0;
    }
  for (int i=0;i<width;i++)
    {
    for (int j=0;j<height;j++)
      {
      project[i]=project[i]+img_t1.at<uchar>(j, i);
      }
    }
vector <space> v_space;
//int max_index=0;
int max=0;
for (int i=0;i<width;i++)
{
   if (g_max<project[i])
   {
   g_max=project[i];
   }
} 
//printf("gmax=%d, %f \n",g_max,cw*g_max);
//-------------Calculate the potential space------------�
int positive[400];
int num_positive;
positive[0]=0;//the position of the first space candidate
num_positive=1; 
for (int i=1;i<width-1;i++)
  {
  if (project[i]>cw*g_max)
    {
    positive[num_positive]=i;
    num_positive++;
	//printf("project[%d]=%d ,11111\n",i,project[i]);
    }
  else
	{
	//printf("project[%d]=%d ,00000\n",i,project[i]);
	}
  }
positive[num_positive]=width-1; //the position of the last space candidate
num_positive++;
int enlarge;
for (int i=1;i<num_positive;i++)
{
	space tmp;
	enlarge =5; 
	if (positive[i]-positive[i-1]>28) // if the character width is bigger than a threshold => big character (not 1 or half character)
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
		else if ((v_space[v_space.size()-1].end_point-v_space[v_space.size()-1].start_point)>37) // if the character before it is a big character => it cannot be a 1/2 character => add into queue
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
/*for (int i=1;i<num_positive;i++)
  {
  space tmp;
  //printf("i=%d positive[i-1] =%d\n",i,positive[i-1]);
  int width_thres;
  if ((positive[i-1]>width*0.212)&&(positive[i-1]<width*0.346)) //if this is the position of the third charactor=> it must be a charactor, canot be number, we use a bigger threshold
    {
    enlarge=5;
    if ((positive[i]-positive[i-1])>25)
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

  else // we use threshold of 18 for a number
    {
    if ((positive[i]-positive[i-1])>18)
      {
  //    printf("i=%d positive[i-1] =%d,positive[i]=%d\n",i,positive[i-1],positive[i]);

        enlarge=5;
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
  }*/

int start_point,end_point;// position of the character
for (int i=0;i<(int) v_space.size();i++)
  {
   //printf("end=%d,start=%d \n",v_space[i].start_point,v_space[i].end_point);
  //printf("number of character is %d\n",v_space.size());
  int num_region=0;
  if ((v_space[i].end_point-v_space[i].start_point)<68)
    {
    num_region=1;
    }
  else if ((v_space[i].end_point-v_space[i].start_point)<105)
    {
    num_region=2;
    }
  else if((v_space[i].end_point-v_space[i].start_point)<150)
    {
    num_region=3;
    }
  else if((v_space[i].end_point-v_space[i].start_point)<195)
    {
    num_region=4;
    }
  else
    num_region=5;
 // num_region=1;
  if (num_region==1) // xu li cho 1 region
    {
    if (check_character(img_bw,v_space[i].start_point,v_space[i].end_point,0,height)==1)
      {
      start_point=(int) (ratio_width*v_space[i].start_point);
      end_point=(int) (ratio_width*v_space[i].end_point);
	//  printf("end=%d, start=%d\n",end_point,start_point);
      Rect Rec(start_point, 0, end_point-start_point,height_ori);
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
      for (int k=mid_point-20;k<mid_point+20;k++)
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
    //  printf("region %d\n",j);
      if (check_character(img_bw,cut_point[j],cut_point[j+1],0,height)==1)
        {
   //     printf("start=%d, end=%d \n",cut_point[j],cut_point[j+1]);
        start_point=(int) (ratio_width*cut_point[j]);
        end_point=(int) (ratio_width*cut_point[j+1]);
        Rect Rec(start_point, 0, end_point-start_point,height_ori-1);
        charRegions.push_back(Rec);
        }
      }
    }
  }
v_space.clear();

return charRegions;
}