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
#include "crop_longplate_corner.h"

using namespace cv;
using namespace std;

float check_score_Horizol(Vec2f can_vl,Vec2f can_vr,Vec2f can_hu,Vec2f can_hd)
{
	Point pt11,pt12,pt21,pt22,pt31,pt32,pt41,pt42; //the cordinate of 4 lines_end points
	double a,b,x0,y0;
	float v_distance=0; // the distance of angle between 2 vertical lines
	float h_distance=0; // the distance of angle between 2 horizontal lines
	float ratio_distance=0;// the distance in ration (in compare with the standard plate
	float angle_vl=can_vl[1];
	float angle_vr=can_vr[1];
	float angle_hd=can_hd[1];
	float angle_hu=can_hu[1];
	float rho_vl,rho_vr,rho_hu,rho_hd;
	rho_vl=can_vl[0];
	rho_vr=can_vr[0];
	rho_hd=can_hd[0];
	rho_hu=can_hu[0];
	//------determine the candidate of line_end_point
	a = cos(angle_vl); b = sin(angle_vl);
	x0 = a*rho_vl; y0 = b*rho_vl;
	pt11.x = cvRound(x0 + 1000*(-b));
	pt11.y = cvRound(y0 + 1000*(a));
	pt12.x = cvRound(x0 - 1000*(-b));
	pt12.y = cvRound(y0 - 1000*(a));

	a = cos(angle_vr); b = sin(angle_vr);
	x0 = a*rho_vr; y0 = b*rho_vr;
	pt21.x = cvRound(x0 + 1000*(-b));
	pt21.y = cvRound(y0 + 1000*(a));
	pt22.x = cvRound(x0 - 1000*(-b));
	pt22.y = cvRound(y0 - 1000*(a));	

	a = cos(angle_hu); b = sin(angle_hu);
	x0 = a*rho_hu; y0 = b*rho_hu;
	pt31.x = cvRound(x0 + 1000*(-b));
	pt31.y = cvRound(y0 + 1000*(a));
	pt32.x = cvRound(x0 - 1000*(-b));
	pt32.y = cvRound(y0 - 1000*(a));

	a = cos(angle_hd); b = sin(angle_hd);
	x0 = a*rho_hd; y0 = b*rho_hd;
	pt41.x = cvRound(x0 + 1000*(-b));
	pt41.y = cvRound(y0 + 1000*(a));
	pt42.x = cvRound(x0 - 1000*(-b));
	pt42.y = cvRound(y0 - 1000*(a));
	//----------------------------------------------
	Point cn1,cn2,cn3,cn4;
	cn3=intersection(pt31,pt32,pt21,pt22);
	cn4=intersection(pt41,pt42,pt21,pt22);
	cn1=intersection(pt11,pt12,pt31,pt32);
	cn2=intersection(pt11,pt12,pt41,pt42);
	//----------------------------------------------
	if ((angle_vr!=0)||(angle_vl!=0))
		{
		v_distance=abs(angle_vl-angle_vr)/max(abs(angle_vl),abs(angle_vr));
		}
	else
		{
		v_distance=0;
		}
	if ((angle_hu!=0)||(angle_hd!=0))
		{
		h_distance=abs(angle_hu-angle_hd)/max(abs(angle_hu),abs(angle_hd));
		}
	else
		{
		h_distance=0;
		}
	//float ratio_car=1.4;
	//float ratio_bike=1.357;
	float ratio_long=4.27;
	float d_vl=norm(cn1-cn2);//length of vertical left line
	float d_vr=norm(cn3-cn4);//length of vertical right line
	float d_hu=norm(cn1-cn3);//length of horizontal up line
	float d_hd=norm(cn2-cn4);//length of horizontal down line
	float ratio=(d_hu+d_hd)/(d_vl+d_vr);
	ratio_distance=abs(ratio-ratio_long)/ratio_long;
	float total_distance;
	total_distance=v_distance+h_distance+ratio_distance*2;
	//printf("v=%f  h=%f  r=%f final =%f\n",v_distance,h_distance,ratio_distance,total_distance);
	return(total_distance);
}
float croping_Horizol(Mat img_src,Point &cn1, Point &cn2, Point &cn3, Point &cn4,float ratio)
{
	Mat img=thresholding(img_src);
	Mat img_canny;
	Canny(img,img_canny,50,150,3);
	int height=img.size().height;
	int width=img.size().width;
	vector<Vec2f> lines; //vector that stores all the lines detected by Hough Transform
	vector<Vec2f> lines_hu; // vector that stores all the candidates for the upper horizontal line 
	vector<Vec2f> lines_hd;// vector that stores all the candidates for the lower horizontal line
	vector<Vec2f> lines_vl;// vector that stores all the candidates for the left vertical line
	vector<Vec2f> lines_vr;// vector that stores all the candidates for the right vertical line 
	int thres_vote=floor(height/4);// minimum votes for a line (in Hough transform)
	HoughLines(img_canny,lines, 2, CV_PI/180,thres_vote, 0, 0 ); // Hough transform to detect lines whose votes> thres_vote. All lines are saved into vector lines
	Point pt1,pt2,pt3,pt4,pt5,pt6; //pt3,pt4,pt5,pt6 are 4 points at conrner of the image
	pt3.x=0; pt3.y=0; //top left corner;
	pt4.x=0; pt4.y=height-1;// bottom left corner;
	pt5.x=width-1;pt5.y=0;//top right cornenr
	pt6.x=width-1;pt6.y=height-1;//bottom right corner
	float interthres=0.5; //threshold to decide if a lines belong to hu,hd,vl,vr or not 
	//------------Detect line candidates and store them in the correspond vector-------------
	for (int i=0;i<(int) lines.size();i++)
		{
		float rho,theta;
		double a,b,x0,y0; 
		Point int1,int2;
		rho = lines[i][0];
		theta = lines[i][1];
		a = cos(theta);
		b = sin(theta);
		x0 = a*rho;
		y0 = b*rho;
		pt1.x = cvRound(x0 + 1000*(-b));
		pt1.y = cvRound(y0 + 1000*(a));
		pt2.x = cvRound(x0 - 1000*(-b));
		pt2.y = cvRound(y0 - 1000*(a));
		if ( theta>CV_PI/180*160 || theta<CV_PI/180*20)
		{
		interthres=0.3;
		int1=intersection(pt1,pt2,pt3,pt5);
		int2=intersection(pt1,pt2,pt4,pt6);
		if ((int1.x<interthres*width)&&(int2.x<interthres*width))
			{
				lines_vl.push_back(lines[i]);
			}
		else if ((int1.x>(1-interthres)*width)&&(int2.x>(1-interthres)*width))
			{
				lines_vr.push_back(lines[i]);
			}
		}
		else if ( theta>CV_PI/180*70 && theta<CV_PI/180*110)
			{
				interthres=0.5;
			int1=intersection(pt1,pt2,pt3,pt4);
			int2=intersection(pt1,pt2,pt5,pt6);
		if ((int1.y<interthres*height)&&(int2.y<interthres*height))
			{
				lines_hu.push_back(lines[i]);
			}
		else if ((int1.y>(1-interthres)*height)&&(int2.y>(1-interthres)*height))
			{
				lines_hd.push_back(lines[i]);
			}

			}
		}

	//----------------- End of line detection------------------------------------------
	//------------------Preparation to find the best set of lines to form the license plate-----------
	unsigned int max_line=3; // We check "max_line" number of candidate from each vector
	int num_vl; // The actual number of vertical left to be checked, this could be different from max_line if lines_vl.size()<max_line
	int num_vr;
	int num_hu;
	int num_hd;
	int num_lines=0;//Number of lines that can be detected 
	float rho,theta;
	double a,b,x0,y0;
	Point pt11,pt12,pt21,pt22,pt31,pt32,pt41,pt42; //the cordinate of 4 lines_end points
	int type=0; //type =1:miss vl, type =2: miss vr, type =3: miss hu, type =4: miss hd
	max_line=4;
	if (lines_vl.size()>0)
	{
		if (lines_vl.size()>max_line)
			{
			num_vl=max_line;
			}
		else
			{
			num_vl=lines_vl.size();
			}
		rho = lines_vl[0][0]; theta = lines_vl[0][1];
		a = cos(theta); b = sin(theta);
		x0 = a*rho; y0 = b*rho;
		pt11.x = cvRound(x0 + 1000*(-b));
		pt11.y = cvRound(y0 + 1000*(a));
		pt12.x = cvRound(x0 - 1000*(-b));
		pt12.y = cvRound(y0 - 1000*(a));
		num_lines++;
	}
	else
		{
		pt11=pt3;pt12=pt4;
		type=1;
		}
	if (lines_vr.size()>0)
	{
		if (lines_vr.size()>max_line)
			{
			num_vr=max_line;
			}
		else
			{
			num_vr=lines_vr.size();
			}
		rho = lines_vr[0][0]; theta = lines_vr[0][1];
		a = cos(theta); b = sin(theta);
		x0 = a*rho; y0 = b*rho;
		pt21.x = cvRound(x0 + 1000*(-b));
		pt21.y = cvRound(y0 + 1000*(a));
		pt22.x = cvRound(x0 - 1000*(-b));
		pt22.y = cvRound(y0 - 1000*(a));
		num_lines++;
	}
	else
		{
		pt21=pt5;pt22=pt6;
		type=2;
		}
	max_line=3;
	if (lines_hu.size()>0)
	{
		if (lines_hu.size()>max_line)
			{
			num_hu=max_line;
			}
		else
			{
			num_hu=lines_hu.size();
			}
		rho = lines_hu[0][0]; theta = lines_hu[0][1];
		a = cos(theta); b = sin(theta);
		x0 = a*rho; y0 = b*rho;
		pt31.x = cvRound(x0 + 1000*(-b));
		pt31.y = cvRound(y0 + 1000*(a));
		pt32.x = cvRound(x0 - 1000*(-b));
		pt32.y = cvRound(y0 - 1000*(a));
		num_lines++;
	}
	else
		{
		pt31=pt3;pt32=pt5;
		type=3;
		}
	if (lines_hd.size()>0)
	{
		if (lines_hd.size()>max_line)
			{
			num_hd=max_line;
			}
		else
			{
			num_hd=lines_hd.size();
			}
		rho = lines_hd[0][0]; theta = lines_hd[0][1];
		a = cos(theta); b = sin(theta);
		x0 = a*rho; y0 = b*rho;
		pt41.x = cvRound(x0 + 1000*(-b));
		pt41.y = cvRound(y0 + 1000*(a));
		pt42.x = cvRound(x0 - 1000*(-b));
		pt42.y = cvRound(y0 - 1000*(a));
		num_lines++;
	}
	else
		{
		pt41=pt4;pt42=pt6;
		type=4;
		}
	//printf("vl=%d vr=%d hu=%d hd=%d\n",num_vl,num_vr,num_hd,num_hu);
	//--------------Start to find the best set of candidate to form the plate-------------------------------
	int index_vl,index_vr,index_hu,index_hd; //index of chosen line
	float plate_score;
	Vec2f can_vl,can_vr,can_hu,can_hd; // the candidate lines to be check
	float d1,d2,delta_x,delta_y;
	if (num_lines==4)
		{
		plate_score=999; //the score that determine how likely the candidates could form a plate. The smaller score is, the more confident we have
		for (int i=0;i<num_vl;i++)
			{
			for (int j=0;j<num_vr;j++)
				{
				for (int k=0;k<num_hu;k++)
					{
					for (int h=0;h<num_hd;h++)
						{
						can_vl=lines_vl[i];
						can_vr=lines_vr[j];
						can_hu=lines_hu[k];
						can_hd=lines_hd[h];
						float score_tmp=check_score_Horizol(can_vl,can_vr,can_hu,can_hd);
						//printf("score=%f\n",score_tmp);
						if (plate_score>score_tmp)
							{
							plate_score=score_tmp;
							index_vl=i;
							index_vr=j;
							index_hu=k;
							index_hd=h;
							}
						}
					}
				}
			}
		
	can_vl=lines_vl[index_vl];
	can_vr=lines_vr[index_vr];
	can_hu=lines_hu[index_hu];
	can_hd=lines_hd[index_hd];
	//printf("index,vl=%d,vr=%d,hu=%d,hd=%d\n",index_vl,index_vr,index_hu,index_hd);
	float angle_vl=can_vl[1];
	float angle_vr=can_vr[1];
	float angle_hd=can_hd[1];
	float angle_hu=can_hu[1];
	float rho_vl,rho_vr,rho_hu,rho_hd;
	rho_vl=can_vl[0];
	rho_vr=can_vr[0];
	rho_hd=can_hd[0];
	rho_hu=can_hu[0];
	//------determine the candidate of line_end_point
	a = cos(angle_vl); b = sin(angle_vl);
	x0 = a*rho_vl; y0 = b*rho_vl;
	pt11.x = cvRound(x0 + 1000*(-b));
	pt11.y = cvRound(y0 + 1000*(a));
	pt12.x = cvRound(x0 - 1000*(-b));
	pt12.y = cvRound(y0 - 1000*(a));

	a = cos(angle_vr); b = sin(angle_vr);
	x0 = a*rho_vr; y0 = b*rho_vr;
	pt21.x = cvRound(x0 + 1000*(-b));
	pt21.y = cvRound(y0 + 1000*(a));
	pt22.x = cvRound(x0 - 1000*(-b));
	pt22.y = cvRound(y0 - 1000*(a));	

	a = cos(angle_hu); b = sin(angle_hu);
	x0 = a*rho_hu; y0 = b*rho_hu;
	pt31.x = cvRound(x0 + 1000*(-b));
	pt31.y = cvRound(y0 + 1000*(a));
	pt32.x = cvRound(x0 - 1000*(-b));
	pt32.y = cvRound(y0 - 1000*(a));

	a = cos(angle_hd); b = sin(angle_hd);
	x0 = a*rho_hd; y0 = b*rho_hd;
	pt41.x = cvRound(x0 + 1000*(-b));
	pt41.y = cvRound(y0 + 1000*(a));
	pt42.x = cvRound(x0 - 1000*(-b));
	pt42.y = cvRound(y0 - 1000*(a));
	//----------------------------------------------
	cn3=intersection(pt31,pt32,pt21,pt22);
	cn4=intersection(pt41,pt42,pt21,pt22);
	cn1=intersection(pt11,pt12,pt31,pt32);
	cn2=intersection(pt11,pt12,pt41,pt42);
	}

	else if (num_lines==3)
		{
		if (type==1)
			{
			cn3=intersection(pt31,pt32,pt21,pt22);
			cn4=intersection(pt41,pt42,pt21,pt22);
			d1=norm(cn3-cn4); //distance between cn3, cn4
			d2=d1*ratio;
			delta_x=d2/sqrt(1+pow((cn3.y-pt32.y)/(cn3.x-pt32.x),2));
			delta_y=delta_x*(cn3.y-pt32.y)/(cn3.x-pt32.x);
			cn1.x=cn3.x-delta_x;
			cn1.y=cn3.y-delta_y;
			cn2.x=cn4.x-delta_x;
			cn2.y=cn4.y-delta_y;
			}
		else if (type==2)
			{
			cn1=intersection(pt11,pt12,pt31,pt32);
			cn2=intersection(pt11,pt12,pt41,pt42);
			d1=norm(cn1-cn2);
			d2=d1*ratio;
			delta_x=d2/sqrt(1+pow((cn1.y-pt32.y)/(cn1.x-pt32.x),2));
			delta_y=delta_x*(cn1.y-pt32.y)/(cn1.x-pt32.x);
			cn3.x=cn1.x+delta_x;
			cn3.y=cn1.y+delta_y;
			cn4.x=cn2.x+delta_x;
			cn4.y=cn2.y+delta_y;
			}
		else if (type==3)
			{
			cn4=intersection(pt21,pt22,pt41,pt42);
			cn2=intersection(pt11,pt12,pt41,pt42);
			d1=norm(cn4-cn2);
			d2=d1/ratio;
			delta_y=d2/sqrt(1+pow((pt12.x-cn2.x)/(cn2.y-pt12.y),2));
			delta_x=delta_y*(pt12.x-cn2.x)/(pt12.y-cn2.y);
			cn1.x=cn2.x-delta_x;
			cn1.y=cn2.y-delta_y;
			cn3.x=cn4.x-delta_x;
			cn3.y=cn4.y-delta_y;
			}
		else if (type==4)
			{
			cn1=intersection(pt11,pt12,pt31,pt32);
			cn3=intersection(pt31,pt32,pt21,pt22);
			d1=norm(cn1-cn3);
			d2=d1/ratio;
			delta_y=d2/sqrt(1+pow((pt12.x-cn1.x)/(cn1.y-pt12.y),2));
			delta_x=delta_y*(pt12.x-cn1.x)/(pt12.y-cn1.y);
			cn2.x=cn1.x+delta_x;
			cn2.y=cn1.y+delta_y;
			cn4.x=cn3.x+delta_x;
			cn4.y=cn3.y+delta_y;
			}
		plate_score=100;
		}
	else
		{
		plate_score=999;
		}
		//printf("end\n");
	return(plate_score);
}


