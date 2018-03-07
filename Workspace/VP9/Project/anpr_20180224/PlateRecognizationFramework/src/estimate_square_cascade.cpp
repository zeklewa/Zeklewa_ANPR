#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <queue>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "IOData.h"
#include "estimate_square_cascade.h"
#include "publicValue.h"



using namespace std;
using namespace cv;


cascade_input estimate_square_cascade(queue<long> car_estimate_square_cascade, int mode)
{
	float size_quare_cascade_x, size_quare_cascade_y;
	if (mode == 0 || mode == 2)
	{
		size_quare_cascade_x = atoi(IOData::GetCongfigData("size_quare_cascade_x:").c_str());
		size_quare_cascade_y = atoi(IOData::GetCongfigData("size_quare_cascade_y:").c_str());
	}
	if (mode == 1 || mode == 3)
	{
		size_quare_cascade_x = atoi(IOData::GetCongfigData("size_long_cascade_x:").c_str());
		size_quare_cascade_y = atoi(IOData::GetCongfigData("size_long_cascade_y:").c_str());
	}
	
	float a[1000];
	int num_of_rang=20;
	float ratio = 2.1;
	int n=0;
	while (!car_estimate_square_cascade.empty())
	{
		n++;
		a[n] = car_estimate_square_cascade.front();
		car_estimate_square_cascade.pop();
	}
	float mina=a[1],maxa=a[1];
	float sum=0;
	for (int i=1;i<=n;i++)
	{
		if (mina > a[i]) mina = a[i];
		if (maxa < a[i]) maxa = a[i];
		sum+=a[i];
	}
	//avg = sum/n;
	// float range = (maxa - mina)/(num_of_rang-1);
	// int d[num_of_rang];
	// float gtd[num_of_rang];
	// for (int i=0;i<num_of_rang;i++)
	// {
	// 	d[i]=0;
	// 	gtd[i]=mina+range*i/2;
	// }
	// for (int i=1;i<=n;i++)
	// 	d[(int)((a[i]-mina)/range)]++;
	// for (int i=0;i<num_of_rang;i++)
	// 	cout << d[i]<<" ";
	// cout << endl;
	// for (int i=0;i<num_of_rang;i++)
	// 	cout << gtd[i]<<" ";
	// cout << endl;
	// int left=0,right=num_of_rang-1;
	// float peak_left = gtd[left+1];
	// float peak_right = gtd[right-1];

	
	float peak_left = mina;
	float peak_right = maxa;
	peak_left = sqrt(peak_left/size_quare_cascade_x/size_quare_cascade_y)*size_quare_cascade_x;
	peak_right = sqrt(peak_right/size_quare_cascade_x/size_quare_cascade_y)*size_quare_cascade_x;
	float step = 0.001;
	float p=1.001;
	
	float mu = 0;
	float range_peak[num_scale_cascade[mode]+1];
	for (int i=1;i<=num_scale_cascade[mode]-1;i++)
		range_peak[i]=(1/(num_scale_cascade[mode]+1-i+1))*(peak_right-peak_left)+peak_left;
	range_peak[num_scale_cascade[mode]]=peak_right;
	float min_range=10000;
	while (p<=2)
	{
		p+=step;
		float q = size_quare_cascade_x;
		int t=0;
		while (q*p<=peak_right)
		{
			q = q*p;
			// cout << q << "\t";
			if (q>peak_left && q<peak_right)
			{
				t++;
			}
		}
		float min_range_step=0;
		if (t==num_scale_cascade[mode])
		{
			q = size_quare_cascade_x;
			while (q*p<peak_left)
				q = q*p;
			for (int i=1;i<=num_scale_cascade[mode];i++)
			{
				min_range_step+=abs(q-range_peak[i]);
				q=q*p;
			}
			if (min_range_step<min_range)
			{
				mu = p;
				min_range = min_range_step;
			}
		}	
	}

	// cout << "aaaaaaaaaaaaaaaaaaaaa"<<mu<<endl;

	// float k = num_scale_cascade[mode];
	// float x = pow(peak_right/peak_left,(float)1/(k-1));
	// float mu = (log(peak_left)-log(size_quare_cascade_x))/log(x);
	// cout << k << "\t"<<x<<"\t"<<mu<<endl;
	cascade_input result;
	if (mu!=0)
	{
		
		result.detectScale = mu;
		result.carMin_x = (int)(peak_left);
		result.carMin_y = (int)(peak_left/size_quare_cascade_x*size_quare_cascade_y);
		result.carMax_x = (int)(peak_right+2);
		result.carMax_y = (int)(peak_right/size_quare_cascade_x*size_quare_cascade_y+2);
		cout <<mode<<"\t"<<peak_left<<"\t"<<peak_right<<"\t"<< result.carMin_x <<"\t"<<result.carMin_y <<"\t"<<result.carMax_x <<"\t"<<result.carMax_y <<"\t"<<result.detectScale <<"\t"<<endl;
	}
	else
	{
		// cout << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"<<endl;
		mu = (peak_left+peak_right)/2/size_quare_cascade_x;
		result.detectScale = mu;
		result.carMin_x = (int)(peak_left);
		result.carMin_y = (int)(peak_left/size_quare_cascade_x*size_quare_cascade_y);
		result.carMax_x = (int)(peak_right+2);
		result.carMax_y = (int)(peak_right/size_quare_cascade_x*size_quare_cascade_y+2);
		cout <<mode<<"\t"<<peak_left<<"\t"<<peak_right<<"\t"<< result.carMin_x <<"\t"<<result.carMin_y <<"\t"<<result.carMax_x <<"\t"<<result.carMax_y <<"\t"<<result.detectScale <<"\t"<<endl;
	}
	return result;
}