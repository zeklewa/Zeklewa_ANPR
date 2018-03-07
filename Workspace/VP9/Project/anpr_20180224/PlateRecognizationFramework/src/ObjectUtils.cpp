#include "iostream"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>
#include <ctime>
#include <thread>
#include <chrono>
#include <cstring>
#include <string>
#include <limits>
#include <sys/types.h>
#include <sys/stat.h>
#include <exception>
#include <math.h>
#include <string.h>
#include <time.h>
#include <algorithm>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include <complex>


#include "ObjectUtils.h"
#include "PlateRecognizator.h"
#include "connectCMS.h"

#define DPRINTC(C) printf(#C " = %c\n", (C))
#define DPRINTS(S) printf(#S " = %s\n", (S))
#define DPRINTD(D) printf(#D " = %d\n", (D))
#define DPRINTLLD(LLD) printf(#LLD " = %lld\n", (LLD))
#define DPRINTLF(LF) printf(#LF " = %.5lf\n", (LF))

typedef long long lld;
typedef unsigned long long llu;
using namespace std;
using namespace cv;
using namespace pr;
int match_score, mismatch_score, gap_score;

int frameNum = 0;
int plate_num = 0;
int dp[50][50];

string cam_id = IOData::GetCongfigData("cam_id:").c_str();
string cam_location = IOData::GetCongfigData("cam_location:").c_str();
string sever_address = IOData::GetCongfigData("sever_address:").c_str();
int video_width = atoi(IOData::GetCongfigData("video_width:").c_str());
int video_height =atoi(IOData::GetCongfigData("video_height:").c_str());

std::ofstream ofs;
std::string root_folder = "/home/thanhnn/Desktop/Result";
string plate_folder_result = "/plate/";
string imageVehicle_folder_result = "/vehicle/";

const int Range_Time = 5;
const int MaxMissPlateOfQueue = 15;
const float distance_one_pixcel = 0.00001163; //10m / 480pixcel (tinh theo km)
const int type_plate_vehicle = 5;

vehicle array_result[1001];
bool n_array_result[1001];

queue<FrameData> frameQueue1;

void ObjectUtils::wtfile_demo(string link,string content)
{
	ofstream fout;
	fout.open(link,ios::app);
	assert(!fout.fail());
	fout << content << endl;
	fout.close();
	assert(!fout.fail());
}

string ObjectUtils::FloatToStr(float tt)
{
	stringstream ss; //convert tt to string
	ss << tt;
	string str = ss.str();
	return str;
}
std::string ObjectUtils::getCurrentDate()
{
	time_t rawtime;
	struct tm *timeinfo;
	char buffer[80];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, 80, "%Y_%m_%d", timeinfo);
	std::string str(buffer);
	return str;
}


long ObjectUtils::convert_time(string s)
{
	long hh, mm, ss;
	hh = (int)(s[11] - '0') * 10 + (int)(s[12] - '0');
	mm = (int)(s[14] - '0') * 10 + (int)(s[15] - '0');
	ss = (int)(s[17] - '0') * 10 + (int)(s[18] - '0');
	return hh * 3600 + mm * 60 + ss;
}
//===========================find best result=======================
int ObjectUtils::minResult(int x, int y, int z)
{
	int tmp = min(x, y);
	return min(tmp, z);
}

inline int needleman_wunsch(string A, string B,int length_str1,int length_str2)
{
	int n=length_str1;
	int m=length_str2;
	
    for (int i=0;i<=n;i++) dp[i][0] = dp[0][i] = -i * gap_score;
    for (int i=1;i<=n;i++)
    {
        for (int j=1;j<=m;j++)
        {
            int S = (A[i-1] == B[j-1]) ? match_score : -mismatch_score;
            dp[i][j] = max(dp[i-1][j-1] + S, max(dp[i-1][j] - gap_score, dp[i][j-1] - gap_score));
        }
    }
    return dp[n][m];
}

inline pair<string, string> get_optimal_alignment(string A, string B, int length_str1, int length_str2)
{
    string retA, retB;
    stack<char> SA, SB;
    int ii = length_str1, jj = length_str2;
    while (ii != 0 || jj != 0)
    {
        if (ii == 0)
        {
            SA.push('-');
            SB.push(B[jj-1]);
            jj--;
        }
        else if (jj == 0)
        {
            SA.push(A[ii-1]);
            SB.push('-');
            ii--;
        }
        else
        {
            int S = (A[ii-1] == B[jj-1]) ? match_score : -mismatch_score;
            if (dp[ii][jj] == dp[ii-1][jj-1] + S)
            {
                SA.push(A[ii-1]);
                SB.push(B[jj-1]);
                ii--; jj--;
            }
            else if (dp[ii-1][jj] > dp[ii][jj-1])
            {
                SA.push(A[ii-1]);
                SB.push('-');
                ii--;
            }
            else
            {
                SA.push('-');
                SB.push(B[jj-1]);
                jj--;
            }
        }
    }
    while (!SA.empty())
    {
        retA += SA.top();
        retB += SB.top();
        SA.pop();
        SB.pop();
    }
    return make_pair(retA, retB);
}
int ObjectUtils::editDist(string str1, string str2, int m, int n)
{
	int dp1[m + 1][n + 1];
	for (int i = 0; i <= m; i++)
	{
		for (int j = 0; j <= n; j++)
		{
			if (i == 0)
				dp1[i][j] = j; // Min. operations = j
			else if (j == 0)
				dp1[i][j] = i; // Min. operations = i
			else if (str1[i - 1] == str2[j - 1])
				dp1[i][j] = dp1[i - 1][j - 1];
			else
				dp1[i][j] = 1 + minResult(dp1[i][j - 1],		// Insert
										 dp1[i - 1][j],		// Remove
										 dp1[i - 1][j - 1]); // Replace
		}
	}
	return dp1[m][n];
}
int Needleman_Dist(stringWithPro str1,stringWithPro str2)
{
	string A=""; // string result of the first plate
	string B=""; // string result of the second plate
	// initialize string A and string B
	match_score = 1, mismatch_score = 1, gap_score = 2;
	int length_str1=0;
	int length_str2=0;
	float v1_pro[30]; // vector that contain the pro of each real-character in A
    float v2_pro[30]; // vector that contain the pro of each real-character in A
	
	for (int i=0;i<(int)str1.vector_cwp.size();i++)
	{
		if ((str1.vector_cwp[i].c!='_')&&(str1.vector_cwp[i].c!='-'))
		{
			A+=str1.vector_cwp[i].c;
			v1_pro[length_str1]=str1.vector_cwp[i].pro;
			length_str1++;
		}
	}
	for (int i=0;i<(int)str2.vector_cwp.size();i++)
	{
		if ((str2.vector_cwp[i].c!='_')&&(str2.vector_cwp[i].c!='-'))
		{
			B+=str2.vector_cwp[i].c;
			v2_pro[length_str2]=str2.vector_cwp[i].pro;
			//cout<<str2.vector_cwp[i].pro<<endl;
			length_str2++;
		}
	}
	needleman_wunsch(A,B,length_str1,length_str2);
	pair<string, string> alignment = get_optimal_alignment(A,B,length_str1,length_str2);
	//printf("string alignment is:\n%s\n%s\n", alignment.first.c_str(), alignment.second.c_str());
	int index=0;
    charWithPro swp;
    vector<charWithPro> string1;
    vector<charWithPro> string2;
    for (int i=0;i<(int)alignment.first.length();i++)
 	{
 		swp.c=alignment.first[i];
 		if (alignment.first[i]=='-')
 		{
 			swp.pro=0;
		 }
		 else
		 {
		 	swp.pro=v1_pro[index];
		 	index++;
		 }	
		 string1.push_back(swp);	 
	 }
	 index=0;
	for (int i=0;i<(int)alignment.second.length();i++)
 	{
 		swp.c=alignment.second[i];
 		if (alignment.second[i]=='-')
 		{
 			swp.pro=0;
		 }
		 else
		 {
		 	swp.pro=v2_pro[index];
		 	index++;
		 }
		 string2.push_back(swp);		 
	 }
	  int dis1=0;
    for (int i=0;i<(int)string1.size();i++)
    {
		//cout<<dis1<<"\t"<<string1[i].c <<"\t"<<string1[i].pro<<"\t"<<string2[i].c <<"\t"<<string2[i].pro<<"\t"<<endl;
    	if ((string1[i].c!='-')&&(string2[i].c!='-'))
    	{
    		if (string1[i].c!=string2[i].c)
    		{
    		dis1++;	
			}
		}
		else if (string2[i].c=='-')
		{
			if (string1[i].pro<0.4)
			{
				dis1++;
			}
		}
		else if (string1[i].c=='-')
		{
			if (string2[i].pro>0.4)
			{
				dis1++;
			}
		}
	}
	return (dis1);

}

vehicle ObjectUtils::Find_best_result(vehicle plateprocess[maxplateprocess],int num_plates)
{
	vehicle a[num_plates];
	for (int i=0;i<num_plates;i++)
		a[i] = plateprocess[i];
	for (int i=0;i<num_plates;i++)
		a[i].speed = plateprocess[num_plates-1].speed;
	int n = num_plates;
	int b[n + 1];
	memset(b, 0, sizeof(b));
	int min_dist = numeric_limits<int>::max(), vt_min_dist = 0;
	float min_pro = 0;
	// for (int i=0;i<n;i++)
	// {

	// 	cout<<i <<"sring  is: "<<a[i].vehiclePlate_ht.PlateStr<<endl;
	// 	for (int k=0;k<a[i].vehiclePlate_ht.vector_cwp.size();k++)
	// 	{
	// 		printf("char=%c, pro=%f \n",a[i].vehiclePlate_ht.vector_cwp[k].c,a[i].vehiclePlate_ht.vector_cwp[k].pro);
	// 	}
	// }
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			b[i] += Needleman_Dist(a[i].vehiclePlate_ht, a[j].vehiclePlate_ht);
			//cout <<i <<j << "distance is: "<<b[i]<<endl;
		}
		
		if (b[i] < min_dist || (b[i] == min_dist && a[i].pro > min_pro))
		{
			min_dist = b[i];
			vt_min_dist = i;
			min_pro = a[i].pro;
		}
	}
	return a[vt_min_dist];
}

float ObjectUtils::calcBlurriness(const Mat &src)
{
	Mat Gx, Gy;
	Sobel(src, Gx, CV_32F, 1, 0);
	Sobel(src, Gy, CV_32F, 0, 1);
	double normGx = norm(Gx);
	double normGy = norm(Gy);
	double sumSq = normGx * normGx + normGy * normGy;
	return static_cast<float>(1000000. / (sumSq / src.size().area() + 1e-6));
}



bool ObjectUtils::filter_result(vehicle a)
{
    int n = 100;
    //loai ket qua sau 1 phut
    for (int i=1;i<=n;i++)
        if (array_result[i].vehiclePlate!="@")
            if (abs(a.time - array_result[i].time) > 10)
                array_result[i].vehiclePlate="@";
    for (int i=1;i<=n;i++)
        if (array_result[i].vehiclePlate!="@")
            if (editDist(a.vehiclePlate,array_result[i].vehiclePlate,a.vehiclePlate.length(),array_result[i].vehiclePlate.length())<=2)
            {
                array_result[i].time = a.time;
                return false;
            }
    for (int i=1;i<=n;i++)
        if (array_result[i].vehiclePlate=="@")
        {
            array_result[i].vehiclePlate = a.vehiclePlate;
            array_result[i].time = a.time;
            break;
        }
    return true;
}


//end find best result
bool init_array_result = true;
void ObjectUtils::printf_result(vehicle plateprocess[maxplateprocess],int num_plates, Rect cropRect,int mode)
{
	this_thread::sleep_for(chrono::milliseconds(10));
	if (init_array_result==true)
	{
		for (int i=1;i<=100;i++) 
		{
			array_result[i].vehiclePlate = "@";
			array_result[i].time=0;
		}
		init_array_result=false;
	}

	vehicle result;
	result = Find_best_result(plateprocess,num_plates);
	//Cap nhat thong tin cho result
	//cv::rectangle(result.plate_croped_detail.plate_extended, result.plate_croped_detail.plate_content, cv::Scalar(0, 0, 255), 1, 8, 0);
	
	result.plate = result.plate_croped_detail.plate_extended;

	Rect RectReal = plateprocess[0].square_cascade;
	RectReal.x += cropRect.x;
	RectReal.y += cropRect.y;
	
	Mat ImageResult = plateprocess[0].vehicleImage.clone();
	RectReal.x = max(0,(int)((float)RectReal.x*(float)ImageResult.cols/(float)video_width)-10);
	RectReal.y = max(0,(int)((float)RectReal.y*(float)ImageResult.rows/(float)video_height)-10);
	RectReal.width = min(ImageResult.cols-1,(int)((float)RectReal.width*(float)ImageResult.cols/(float)video_width)+20);
	RectReal.height = min(ImageResult.rows-1,(int)((float)RectReal.height*(float)ImageResult.rows/(float)video_height)+20);
	// result.plate = ImageResult(RectReal).clone();
	cv::rectangle(ImageResult, RectReal, cv::Scalar(0, 255, 0), 2, 8, 0);
	result.speed = ceilf(result.speed);
	// putText(ImageResult, FloatToStr(result.speed) + "km/h", cvPoint(RectReal.x, RectReal.y-20), FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0, 0, 0), 20, CV_AA);
	// putText(ImageResult, FloatToStr(result.speed) + "km/h", cvPoint(RectReal.x, RectReal.y-20), FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(255, 255, 255), 2, CV_AA);

	//TODO: crop vehicle image
    int carX = std::max(RectReal.x - 400, 0);
    int carY = std::max(RectReal.y - 400, 0);
    Rect RectVehicle = cv::Rect(
        carX, carY,
        std::min(RectReal.width + 800, ImageResult.cols - carX-1),
        std::min(RectReal.height + 800, ImageResult.rows - carY-1));
	result.vehicleImage = ImageResult(RectVehicle);
	result.vehicleImageName = "vehicle_"+result.CurrentDateTime+"_"+result.vehiclePlate+".jpg";
	result.link = "plate_"+result.CurrentDateTime+"_"+result.vehiclePlate+".jpg";
	//cout << "result:" << result.vehiclePlate << "\t" << result.pro << "\t" << result.direction << endl;
	//cout << "----------------------------------"<<endl;
	int locationX = RectReal.x;
	if (result.vehiclePlate.length() >= 1 && filter_result(result) && result.pro>1)
	{
		// char filename[100];
		// sprintf(filename,"/home/tonlh/Desktop/Test_Result/2017_10_23_06_37_00/%s_%d.jpg",result.vehiclePlate.c_str(),result.frameID);
		// cout<<filename<<"\t"<<result.vehiclePlate<<endl;
		// imwrite(filename,result.plate);
		cout << "result:" << result.vehiclePlate << "\t" << result.pro << "\t" << result.direction<<"\t"<<locationX << endl;
		try
		{
			push_data_to_CMS(result.vehiclePlate,cam_id,result.CurrentDateTime,result.plate, result.vehicleImage,"0",cam_location,sever_address,mode,locationX);
		}
		catch (exception& e)
		{
		  cout << "Standard exception: " << e.what() << endl;
		}
		

	}
}



linear_equation ObjectUtils::calculation_line_equa(int x1, int y1, int x2, int y2)
{
	//a*x + b*y + c = 0
	linear_equation line;
    line.a = (float)y1-(float)y2;
    line.b = (float)x2-(float)x1;
    line.c = (float)x1*(float)y2-(float)x2*(float)y1;
	if (line.b==0)
	{
		line.a = -1; 
        line.b = 0;
        line.c = (float)x1;
	}
	else if (line.a ==0)
	{
		line.a = 0; 
        line.b = -1;
        line.c = (float)y1;
	}
    return line;
}

float ObjectUtils::distance_point_to_line(linear_equation line, int x, int y)
{
	float result;
	float x0 = x; float y0=y;
	result = abs(line.a*x0+line.b*y0+line.c)/sqrt(pow(line.a,2)+pow(line.b,2));
	return result;
}

std::string ObjectUtils::getCurrentDateTime()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%d-%m-%Y_%I-%M-%S", timeinfo);
    std::string str(buffer);
    return str;
}


std::string ObjectUtils::getCurrentDateTime_pushSQL()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
    std::string str(buffer);
    return str;
}

string ObjectUtils::kindcascade(int mode)
{
	string kindcascade = "car_cascade:";
	if (mode==0)
		kindcascade = "car_cascade:";
	if (mode==1)
		kindcascade = "car_horizontal_cascade:";
	if (mode==2)
		kindcascade = "car_redblue_cascade:";
	if (mode==3)
		kindcascade = "car_redblue_horizontal_cascade:";
	return kindcascade;
}
int ObjectUtils::kindsizecascade(int mode)
{
	int kindsizecascade = 0;
	if (mode==0)
		kindsizecascade = 0;
	if (mode==1)
		kindsizecascade = 1;
	if (mode==2)
		kindsizecascade = 0;
	if (mode==3)
		kindsizecascade = 1;
	return kindsizecascade;	
}