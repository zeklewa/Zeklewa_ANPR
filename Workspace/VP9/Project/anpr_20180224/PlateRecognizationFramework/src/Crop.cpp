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
#include "Crop.h"
#include"common_function.h"
#include"cropping_v2.h"
#include"cropping_v3.h"
#include"crop_longplate_corner.h"
#include"cropping_v2_color.h"
#include"cropping_v3_color.h"
#include"crop_char_square.h"
#include"crop_char_long.h"
#include "blurriness.h"

using namespace cv;
using namespace std;
Point intersection_general(float a1,float b1, float c1, float a2, float b2, float c2)
{
Point pt;
pt.x=(int)((c1*b2-b1*c2)/(b1*a2-a1*b2));
pt.y=(int)((a1*c2-c1*a2)/(b1*a2-a1*b2));
return (pt);
}
void point_extend (Point &cn1, Point &cn2, Point &cn3, Point &cn4,float dis)
{
// The equation off 4 line , in form of ax+by+c=0;
float a1=(float) (cn1.y-cn3.y); // up line
float b1=(float)(cn3.x-cn1.x);
float c1=(float)(cn1.x*cn3.y-cn3.x*cn1.y);
float a2=(float)(cn2.y-cn4.y);// down line
float b2=(float)(cn4.x-cn2.x);
float c2=(float)(cn2.x*cn4.y-cn4.x*cn2.y);
float a3=(float)(cn1.y-cn2.y);// left line
float b3=(float)(cn2.x-cn1.x);
float c3=(float)(cn1.x*cn2.y-cn2.x*cn1.y);
float a4=(float)(cn3.y-cn4.y); // right
float b4=(float)(cn4.x-cn3.x);
float c4=(float)(cn3.x*cn4.y-cn4.x*cn3.y);
float c11=c1+sqrt(a1*a1+b1*b1)*dis;
float c22=c2-sqrt(a2*a2+b2*b2)*dis;
float c33=c3-sqrt(a3*a3+b3*b3)*dis;
float c44=c4+sqrt(a4*a4+b4*b4)*dis;
cn1=intersection_general(a1,b1,c11,a3,b3,c33);
cn2=intersection_general(a2,b2,c22,a3,b3,c33);
cn3=intersection_general(a1,b1,c11,a4,b4,c44);
cn4=intersection_general(a2,b2,c22,a4,b4,c44);
}
void point_extend_border (Point &cn1, Point &cn2, Point &cn3, Point &cn4,int left, int right, int up,int down)
{
// The equation off 4 line , in form of ax+by+c=0;
float a1=(float) (cn1.y-cn3.y); // up line
float b1=(float)(cn3.x-cn1.x);
float c1=(float)(cn1.x*cn3.y-cn3.x*cn1.y);
float a2=(float)(cn2.y-cn4.y);// down line
float b2=(float)(cn4.x-cn2.x);
float c2=(float)(cn2.x*cn4.y-cn4.x*cn2.y);
float a3=(float)(cn1.y-cn2.y);// left line
float b3=(float)(cn2.x-cn1.x);
float c3=(float)(cn1.x*cn2.y-cn2.x*cn1.y);
float a4=(float)(cn3.y-cn4.y); // right
float b4=(float)(cn4.x-cn3.x);
float c4=(float)(cn3.x*cn4.y-cn4.x*cn3.y);
float c11=c1+sqrt(a1*a1+b1*b1)*up;
float c22=c2-sqrt(a2*a2+b2*b2)*down;
float c33=c3-sqrt(a3*a3+b3*b3)*left;
float c44=c4+sqrt(a4*a4+b4*b4)*right;
cn1=intersection_general(a1,b1,c11,a3,b3,c33);
cn2=intersection_general(a2,b2,c22,a3,b3,c33);
cn3=intersection_general(a1,b1,c11,a4,b4,c44);
cn4=intersection_general(a2,b2,c22,a4,b4,c44);
}
plate_corner crop_square_white_plate_corner (Mat img_rgb,float ratio)
{
	Mat img;
	if (img_rgb.channels()==3)
	{
		cvtColor(img_rgb, img,CV_RGB2GRAY);
	}
	else
	{
	img=img_rgb;
	}
	plate_corner tmp;
	Mat img_src=img;
	int width=img.size().width;
	int height=img.size().height;
	Point cn1, cn2, cn3, cn4;  // To be done :initialize the value of cn1, cn2,cn3, cn4
	float result=9999; 
	//int check;
	result= croping_v2(img,cn1,cn2,cn3,cn4,ratio);
	//checkcheck=0;
	if (result<99)
		{
		tmp.cn1=cn1;
		tmp.cn2=cn2;
		tmp.cn3=cn3;
		tmp.cn4=cn4;
		tmp.plate_score=result;
		tmp.type=2;
		}
	else
		{
		//printf("cropping v3\n");
		result=croping_v3(img,cn1,cn2,cn3,cn4,ratio);
		if (result<100)
			{
			tmp.cn1=cn1;
			tmp.cn2=cn2;
			tmp.cn3=cn3;
			tmp.cn4=cn4;
			tmp.plate_score=result;
			tmp.type=3;
			}
		else
			{
			tmp.plate_score=9999;
			}

		}
	if (tmp.plate_score<99)
		{
		tmp.isplate=true;
		Mat plate_extended;
		Rect rect_small;
		float d_vl=norm(tmp.cn1-tmp.cn2);//length of vertical left line
		float d_vr=norm(tmp.cn3-tmp.cn4);//length of vertical right line
		float d_hu=norm(tmp.cn1-tmp.cn3);//length of horizontal up line
		float d_hd=norm(tmp.cn2-tmp.cn4);//length of horizontal down line
		float ratio_i=(d_hu+d_hd)/(d_vl+d_vr);
		tmp.plate_ratio=ratio_i;
		//border plate_border;
		//plate_border.check=1;
		//int iteration=0;
		/*while((plate_border.check==1)&&(iteration<4))
			{
			//printf("111\n");
			int width_plate=norm (cn2-cn4);
			int height_plate=norm (cn1-cn2);
			Point2f a(0, 0), b(width, 0), c(width, height),d(0,height);
			cv::Point2f src[] = {cn1, cn3, cn4,cn2};
			cv::Point2f dst[] = {a, b, c,d};
			float ratio_i=(float) width_plate/width;
			Mat warpMat = getPerspectiveTransform(src, dst);
			warpPerspective(img,plate_extended, warpMat,img.size());
			//display_image(plate_extended);	
			plate_border=plate_filter(plate_extended);
			int left,right,up,down;
			left=0-plate_border.border_left;
			right=0-plate_border.border_right;
			up=0-plate_border.border_up;
			down=0-plate_border.border_down;
			left=(int) left*ratio_i;
			right=(int) right*ratio_i;
			up=(int) up*ratio_i;
			down=(int) down *ratio_i;
			point_extend_border (cn1, cn2, cn3, cn4,left,right, up,down);
			//display_image(plate_extended);	
			iteration++;
			}*/
		Point2f a2(0, 0), b2(width, 0), c2(width, height),d2(0,height);
		point_extend (cn1, cn2, cn3, cn4,dis_extend);
		float dis2=norm(cn2-cn4);
		if (dis2==0)
			{
			dis2=width;
			}
		dis2=dis_extend*width/dis2;
		int extend=(int) (dis2);
		rect_small.x=min(extend,width-1);
		rect_small.y=min(extend,height-1);
		rect_small.width=max(width-2*extend,0);
		rect_small.height=max(height-2*extend,0);
		tmp.plate_content= rect_small;
		cv::Point2f src[] = {cn1, cn3, cn4,cn2};
		cv::Point2f dst[] = {a2, b2, c2,d2};
		cv::Mat warpMat = cv::getAffineTransform(src, dst);
		cv::warpAffine(img_rgb,plate_extended, warpMat, img.size());
		tmp.plate_extended=plate_extended;
		//display_image(tmp.plate_extended);
		}
	
	else
		{
		tmp.isplate=false;
		}

	return(tmp);
}
plate_corner crop_long_white_plate_corner (Mat img_rgb,float ratio)
{
	Mat img;
	if (img_rgb.channels()==3)
	{
		cvtColor(img_rgb, img,CV_RGB2GRAY);
	}
	else
	{
	img=img_rgb;
	}
	plate_corner tmp;
	Mat img_src=img;
	int width=img.size().width;
	int height=img.size().height;
	Point cn1, cn2, cn3, cn4;  // To be done :initialize the value of cn1, cn2,cn3, cn4
	float result=9999; 
	//int check;
	result= croping_Horizol(img,cn1,cn2,cn3,cn4,ratio);
	//check=0;
	if (result<99)
		{
		tmp.cn1=cn1;
		tmp.cn2=cn2;
		tmp.cn3=cn3;
		tmp.cn4=cn4;
		tmp.plate_score=result;
		tmp.isplate=true;
		Mat plate_extended;
		Rect rect_small;
		float d_vl=norm(tmp.cn1-tmp.cn2);//length of vertical left line
		float d_vr=norm(tmp.cn3-tmp.cn4);//length of vertical right line
		float d_hu=norm(tmp.cn1-tmp.cn3);//length of horizontal up line
		float d_hd=norm(tmp.cn2-tmp.cn4);//length of horizontal down line
		float ratio_i=(d_hu+d_hd)/(d_vl+d_vr);
		tmp.plate_ratio=ratio_i;
		//border plate_border;
		//plate_border.check=1;
		//int iteration=0;
	/*	while((plate_border.check==1)&&(iteration<4))
			{
			//printf("111\n");
			int width_plate=norm (cn2-cn4);
			int height_plate=norm (cn1-cn2);
			Point2f a(0, 0), b(width, 0), c(width, height),d(0,height);
			cv::Point2f src[] = {cn1, cn3, cn4,cn2};
			cv::Point2f dst[] = {a, b, c,d};
			float ratio_i=(float) width_plate/width;
			Mat warpMat = getPerspectiveTransform(src, dst);
			warpPerspective(img,plate_extended, warpMat,img.size());
			//display_image(plate_extended);	
			plate_border=plate_filter(plate_extended);
			int left,right,up,down;
			left=0-plate_border.border_left;
			right=0-plate_border.border_right;
			up=0-plate_border.border_up;
			down=0-plate_border.border_down;
			left=(int) left*ratio_i;
			right=(int) right*ratio_i;
			up=(int) up*ratio_i;
			down=(int) down *ratio_i;
			point_extend_border (cn1, cn2, cn3, cn4,left,right, up,down);
			//display_image(plate_extended);	
			iteration++;
			}*/
		//printf("height=%d width=%d\n",height,width);
		//width=(int) (height*47/11);
		//printf("height=%d width=%d\n",height,width);	
		Point2f a2(0, 0), b2(width, 0), c2(width, height),d2(0,height);

		point_extend (cn1, cn2, cn3, cn4,dis_extend);
		float dis2=norm(cn2-cn4);
		if (dis2==0)
			{
			dis2=width;
			}
		dis2=dis_extend*width/dis2;
		int extend=(int) (dis2);

		rect_small.x=min(extend,width-1);
		rect_small.y=min(extend,height-1);
		rect_small.width=max(width-2*extend,0);
		rect_small.height=max(height-2*extend,0);
		tmp.plate_content= rect_small;
		/*point_extend (cn1, cn2, cn3, cn4,dis_extend);
		float dis2=norm(cn2-cn4);
		if (dis2==0)
			{
			dis2=width;
			}
		dis2=dis_extend*width/dis2;
		int extend=(int) (dis2);
		Mat plate_extended;
		Rect rect_small;
		rect_small.x=min(extend,width-1);
		rect_small.y=min(extend,height-1);
		rect_small.width=max(width-2*extend,0);
		rect_small.height=max(height-2*extend,0);
		tmp.plate_content= rect_small;*/
		cv::Point2f src[] = {cn1, cn3, cn4,cn2};
		cv::Point2f dst[] = {a2, b2, c2,d2};
		//cv::Mat warpMat = cv::getAffineTransform(src, dst);
		//cv::warpAffine(img,plate_extended, warpMat, img.size());
		Mat warpMat = getPerspectiveTransform(src, dst);
		Size s1=Size(width,height);
		warpPerspective(img_rgb,plate_extended, warpMat,s1);
		tmp.plate_extended=plate_extended;
		tmp.type=3;
		//display_image(tmp.plate_extended);
	  }
	
	else
		{
		tmp.isplate=false;
		}

	return(tmp);
}
plate_corner crop_square_color_plate_corner (Mat img_rgb,float ratio)
{
	//cout<<"start"<<endl;
	Mat img;
	if (img_rgb.channels()==3)
	{
		cvtColor(img_rgb, img,CV_RGB2GRAY);
	}
	else
	{
	img=img_rgb;
	}
	bitwise_not(img,img);
	plate_corner tmp;
	Mat img_src=img;
	int width=img.size().width;
	int height=img.size().height;
	Point cn1, cn2, cn3, cn4;  // To be done :initialize the value of cn1, cn2,cn3, cn4
	float result=9999; 
	//int check;
	//cout<<"11111\n"<<endl;
	result= croping_v2_color(img,cn1,cn2,cn3,cn4,ratio);
	//check=0;
	tmp.type=0;
	//cout<<"22222222"<<endl;
	//printf("result v2=%f\n",result);
	if (result<99)
		{
		tmp.cn1=cn1;
		tmp.cn2=cn2;
		tmp.cn3=cn3;
		tmp.cn4=cn4;
		tmp.plate_score=result;
		tmp.type=2;
		}

	else
		{
		//printf("cropping v3\n");
		result=croping_v3_color(img,cn1,cn2,cn3,cn4,ratio);
		if (result<100)
			{
			tmp.cn1=cn1;
			tmp.cn2=cn2;
			tmp.cn3=cn3;
			tmp.cn4=cn4;
			tmp.plate_score=result;
			tmp.type=3;
			}
	
		else
			{
			tmp.plate_score=9999;
			}
		//printf("result v3=%f \n",result);
		}
	if (result<99)
		{
		tmp.isplate=true;
		Mat plate_extended;
		Rect rect_small;
		border plate_border;
		plate_border.check=1;
		int iteration=0;
		while((plate_border.check==1)&&(iteration<4))
			{
			//printf("111\n");
			int width_plate=norm (cn2-cn4);
			//int height_plate=norm (cn1-cn2);
			Point2f a(0, 0), b(width, 0), c(width, height),d(0,height);
			cv::Point2f src[] = {cn1, cn3, cn4,cn2};
			cv::Point2f dst[] = {a, b, c,d};
			float ratio_i=(float) width_plate/width;
			Mat warpMat = getPerspectiveTransform(src, dst);
			warpPerspective(img,plate_extended, warpMat,img.size());
			//display_image(plate_extended);	
			plate_border=plate_filter(plate_extended);
			int left,right,up,down;
			left=0-plate_border.border_left;
			right=0-plate_border.border_right;
			up=0-plate_border.border_up;
			down=0-plate_border.border_down;
			left=(int) left*ratio_i;
			right=(int) right*ratio_i;
			up=(int) up*ratio_i;
			down=(int) down *ratio_i;
			point_extend_border (cn1, cn2, cn3, cn4,left,right, up,down);
			//display_image(plate_extended);	
			iteration++;
			}
		//display_image(plate_extended);	
		point_extend (cn1, cn2, cn3, cn4,dis_extend);
		float dis2=norm(cn2-cn4);
		if (dis2==0)
			{
			dis2=width;
			}
		dis2=dis_extend*width/dis2;
		int extend=(int) (dis2);
		rect_small.x=min(extend,width-1);
		rect_small.y=min(extend,height-1);
		rect_small.width=max(width-2*extend,0);
		rect_small.height=max(height-2*extend,0);
		tmp.plate_content= rect_small;

		Point2f a2(0, 0), b2(width, 0), c2(width, height),d2(0,height);
		Point2f src[] = {cn1, cn3, cn4,cn2}; 
		Point2f dst[] = {a2, b2, c2,d2};
		//cv::Mat warpMat = cv::getAffineTransform(src, dst);
		//cv::warpAffine(img,plate_extended, warpMat, img.size());
		Mat warpMat = getPerspectiveTransform(src, dst);
		warpPerspective(img_rgb,plate_extended, warpMat,img.size()); 
		tmp.plate_extended=plate_extended;
		//display_image(tmp.plate_extended);
		}
	
	else
		{
		tmp.isplate=false;
		}
		//cout<<"done"<<endl;

	return(tmp);
}
plate_corner crop_long_color_plate_corner (Mat img_rgb,float ratio)
{
	Mat img;
	if (img_rgb.channels()==3)
	{
		cvtColor(img_rgb, img,CV_RGB2GRAY);
	}
	else
	{
	img=img_rgb;
	}
	bitwise_not(img,img);
	plate_corner tmp;
	Mat img_src=img;
	int width=img.size().width;
	int height=img.size().height;
	Point cn1, cn2, cn3, cn4;  // To be done :initialize the value of cn1, cn2,cn3, cn4
	float result=9999; 
	//int check;
	result= croping_Horizol(img,cn1,cn2,cn3,cn4,ratio);
	//check=0;
	if (result<99)
		{
		tmp.cn1=cn1;
		tmp.cn2=cn2;
		tmp.cn3=cn3;
		tmp.cn4=cn4;
		tmp.plate_score=result;
		tmp.isplate=true;
		Mat plate_extended;
		Rect rect_small;
		//border plate_border;
		//plate_border.check=1;
		//int iteration=0;
	/*	while((plate_border.check==1)&&(iteration<4))
			{
			//printf("111\n");
			int width_plate=norm (cn2-cn4);
			int height_plate=norm (cn1-cn2);
			Point2f a(0, 0), b(width, 0), c(width, height),d(0,height);
			cv::Point2f src[] = {cn1, cn3, cn4,cn2};
			cv::Point2f dst[] = {a, b, c,d};
			float ratio_i=(float) width_plate/width;
			Mat warpMat = getPerspectiveTransform(src, dst);
			warpPerspective(img,plate_extended, warpMat,img.size());
			//display_image(plate_extended);	
			plate_border=plate_filter(plate_extended);
			int left,right,up,down;
			left=0-plate_border.border_left;
			right=0-plate_border.border_right;
			up=0-plate_border.border_up;
			down=0-plate_border.border_down;
			left=(int) left*ratio_i;
			right=(int) right*ratio_i;
			up=(int) up*ratio_i;
			down=(int) down *ratio_i;
			point_extend_border (cn1, cn2, cn3, cn4,left,right, up,down);
			//display_image(plate_extended);	
			iteration++;
			}*/
		//printf("height=%d width=%d\n",height,width);
		//width=(int) (height*47/11);
		//printf("height=%d width=%d\n",height,width);	
		Point2f a2(0, 0), b2(width, 0), c2(width, height),d2(0,height);

		point_extend (cn1, cn2, cn3, cn4,dis_extend);
		float dis2=norm(cn2-cn4);
		if (dis2==0)
			{
			dis2=width;
			}
		dis2=dis_extend*width/dis2;
		int extend=(int) (dis2);

		rect_small.x=min(extend,width-1);
		rect_small.y=min(extend,height-1);
		rect_small.width=max(width-2*extend,0);
		rect_small.height=max(height-2*extend,0);
		tmp.plate_content= rect_small;
		/*point_extend (cn1, cn2, cn3, cn4,dis_extend);
		float dis2=norm(cn2-cn4);
		if (dis2==0)
			{
			dis2=width;
			}
		dis2=dis_extend*width/dis2;
		int extend=(int) (dis2);
		Mat plate_extended;
		Rect rect_small;
		rect_small.x=min(extend,width-1);
		rect_small.y=min(extend,height-1);
		rect_small.width=max(width-2*extend,0);
		rect_small.height=max(height-2*extend,0);
		tmp.plate_content= rect_small;*/
		cv::Point2f src[] = {cn1, cn3, cn4,cn2};
		cv::Point2f dst[] = {a2, b2, c2,d2};
		//cv::Mat warpMat = cv::getAffineTransform(src, dst);
		//cv::warpAffine(img,plate_extended, warpMat, img.size());
		Mat warpMat = getPerspectiveTransform(src, dst);
		Size s1=Size(width,height);
		warpPerspective(img_rgb,plate_extended, warpMat,s1);
		tmp.plate_extended=plate_extended;
		tmp.type=3;
		//display_image(tmp.plate_extended);
	  }
	
	else
		{
		tmp.isplate=false;
		}

	return(tmp);
}
plate_corner crop_plate_corner (Mat img_rgb,int mode,float ratio)
{
plate_corner plate_tmp;
//printf("fasdfasd mode =%d ratio=%f \n",mode,ratio);
switch(mode)
{
	case 0: // bien vuong trang
	plate_tmp=crop_square_white_plate_corner(img_rgb,ratio);
	if (plate_tmp.isplate)
	{
	plate_tmp.blurriness = calBlurriness_Laplacian1(plate_tmp.plate_extended(plate_tmp.plate_content)); 
	plate_tmp.brightness = cal_brighness(plate_tmp.plate_extended(plate_tmp.plate_content));
	}
	break;
case 1: // bien dai trang
	plate_tmp=crop_long_white_plate_corner(img_rgb,ratio);
	if (plate_tmp.isplate)
	{
	plate_tmp.blurriness = calBlurriness_Laplacian1(plate_tmp.plate_extended(plate_tmp.plate_content)); 
	plate_tmp.brightness = cal_brighness(plate_tmp.plate_extended(plate_tmp.plate_content));
	}
	break;
case 2: // bien xanh/do vuong
	plate_tmp=crop_square_color_plate_corner(img_rgb, ratio);
	if (plate_tmp.isplate)
	{
	plate_tmp.blurriness = calBlurriness_Laplacian1(plate_tmp.plate_extended(plate_tmp.plate_content));
	plate_tmp.brightness = cal_brighness(plate_tmp.plate_extended(plate_tmp.plate_content)); 
	}
	break;
case 3: // bien xanh/do dai
	plate_tmp=crop_long_color_plate_corner(img_rgb, ratio);
	if (plate_tmp.isplate)
	{
	plate_tmp.blurriness = calBlurriness_Laplacian1(plate_tmp.plate_extended(plate_tmp.plate_content)); 
	plate_tmp.brightness = cal_brighness(plate_tmp.plate_extended(plate_tmp.plate_content));
	}
	break;
}
return (plate_tmp);
}
vector<cv::Rect> crop_plate_char(Mat img_rgb, int mode)
{
	vector<cv::Rect> tmp;
	Mat img;
	if (img_rgb.channels()==3)
	{
		cvtColor(img_rgb, img,CV_RGB2GRAY);
	}
	else
	{
	img=img_rgb;
	}
	switch (mode)
	{
	case 0: // bien trang vuong
		tmp=getCharacterRect_Square(img);
		break;
	case 1: // bien trang dai
			tmp=getCharacterRect_LongPlate(img);
		break;
	case 2: // bien xanh/do vuong
		bitwise_not(img,img);
		tmp=getCharacterRect_Square(img);
		break;
	case 3: // bien xanh/do dai
		bitwise_not(img,img);
		tmp=getCharacterRect_LongPlate(img);
		break;
	}
return (tmp);
}

