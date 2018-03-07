#include "TextReader.h"
#include <opencv2/opencv.hpp>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"


using namespace pr;
using namespace std;
// using namespace keras;
using namespace cv;

char ch[33] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F','G','H','K','L','M','N','P','R','S','T','U','V','X','Y','Z','_'};
const int WIDTH = 28;
const int HEIGHT = 28;
struct plate_special // to check the special case of license plate(MD,AA, NG, NN)
{
	int char_num; // number of posible charater to be check
	int index[10];// index of character to be check A=10, B=11, C=12
};
plate_special check_special(char c)
{
	plate_special ps;
	switch (c)
	{
		case 'A':
			ps.char_num=1;
			ps.index[0]=10; //A
			break;
		case 'M':
			ps.char_num=2;
			ps.index[0]=13; //D
			ps.index[1]=18; //K
			break;
		case 'N':
			ps.char_num=2;
			ps.index[0]=21; //N
			ps.index[1]=16; //G
			break;
		case 'L':
			ps.char_num=1;
			ps.index[0]=13; //D
			break;
		case 'K':
			ps.char_num=1;
			ps.index[0]=25; //T
			break;
		case 'T':
			ps.char_num=1;
			ps.index[0]=13; //D
			break;
		case 'D':
			ps.char_num=1;
			ps.index[0]=11; //A
			break;
		case 'H':
			ps.char_num=1;
			ps.index[0]=12; //C
			break;
		case 'F':
			ps.char_num=1;
			ps.index[0]=12; //C
			break;

		default:
			ps.char_num=0;// it is not a special case
			ps.index[0]=32;
	}
	return (ps);
}

plate_special check_special_previous(char c)
{
	plate_special ps;
	switch (c)
	{
		case 'A':
			ps.char_num=2;
			ps.index[0]=10; //A
			ps.index[1]=13; //D
			break;
		case 'N':
			ps.char_num=1;
			ps.index[0]=21; //N
			break;
		case 'D':
			ps.char_num=3;
			ps.index[0]=19; //L
			ps.index[1]=20;  //M
			ps.index[2]=25; //T
			break;
		case 'G':
			ps.char_num=1;
			ps.index[0]=21; //N
			break;
		case 'T':
			ps.char_num=1;
			ps.index[0]=18; //K
			break;
		case 'C':
			ps.char_num=2;
			ps.index[0]=17; //H
			ps.index[1]=15; //F
			break;	
		case 'K':
			ps.char_num=1;
			ps.index[0]=20; //M
			break;			
		default:
			ps.char_num=0;//N
	}
	return (ps);
}
//"../data/DeepLearningdata/cnn-2pool-20170221.dump"
TextReader::TextReader(string init_model, string predict_model){
	model = new CNN(init_model, predict_model, WIDTH, HEIGHT);
}

// TextReader::TextReader(string dumpfile)
// {
// 	m = new KerasModel(dumpfile, true);
// 	sample = new DataChunk2D();

// }
std::vector< std::vector<std::vector<float > > > TextReader::mat2data(cv::Mat img) {
	cv::Mat img_resize;
	cv::resize(img, img_resize, cv::Size(WIDTH, HEIGHT));
	vector< vector<vector<float > > > data;
	vector<vector<float> > mat;
	for (int i = 0; i < img_resize.rows; i++) {
		vector<float> row;
		for (int j = 0; j < img_resize.cols; j++) {
			row.push_back((float)img_resize.at<uchar>(i, j) / 255);
		}
		mat.push_back(row);
	}
	data.push_back(mat);
	return data;
}

charWithPro TextReader::test_keras2cpp_with_mat(cv::Mat img)
 {
	charWithPro cwp;
	//Keras CNN
	if (img.channels() != 1) 
		cv::cvtColor(img, img, CV_BGR2GRAY);
	if (img.channels() == 1) 
		cv::cvtColor(img, img, cv::COLOR_GRAY2BGR);
	//Caffe CNN
	//sample->set_data(mat2data(img));
	char c = ch[0];
	
	//mode: 1 = Num + Word + Negative; 	2 = Num + Word; 
	//		3 = Num + Negative; 		4 = Num; 
	//		5 = Word + Negative; 		6 = Word; 	
	// auto res = m->compute_output(sample);
	cv::resize(img, img, cv::Size(28, 28), 0, 0, cv::INTER_AREA);
	auto res = model->predict(img);
	float max = 1e-5;
	for (int i=0;i<res.size();i++)
	{
		cwp.cnn_out[i]=res[i];
	}
	for (int k = 0; k < 10; k++) 
	{
		if (res[k] > max) 
		{
			max = res[k];
			c = ch[k];
		}
	}
	cwp.num_c=c;
	cwp.num_pro=max;
	c = ch[10];
	max=1e-5;
	for (int k = 10; k < (int)res.size()-1; k++) 
	{
		if (res[k] > max) 
		{
		max = res[k];
		c = ch[k];
		}
	}
	cwp.char_c=c;
	cwp.char_pro=max;
	cwp.neg_c='_';
	cwp.neg_pro= res[(int)res.size()-1];
	cwp.c='_';
	cwp.pro=0;
	return cwp;
}
stringWithPro TextReader::GetPlateString_square(std::vector<cv::Rect> charRects, cv::Mat& img)
{
	//printf("start \n");
	stringWithPro swp;
	std::string plateStr = "";
	string proEachChar = "";
	float pro = 0;
	swp.PlateStr = "";
	swp.pro=0;
	swp.num_char=0;
	swp.PlateStr="";
	swp.proEachChar="";
	charWithPro cwp;
	
	//int k=1;
	int location_minus;
	for (int i = 0; i < (int)charRects.size(); i++)
	{
		Rect r1 = charRects[i];
		if (r1.x==0 && r1.y==0 && r1.width==0 && r1.height==0) //tim vi tri dau "-"
			{
			location_minus = i;
			break;
			}
	}
	vector<charWithPro> v_char;
	for (int i=0;i<(int)charRects.size();i++)
	{
		if (i==location_minus) 
		{
			cwp.c='-';
			cwp.pro=1;
			cwp.num_pro=0;
			cwp.char_pro=0;
			cwp.neg_pro=0;
			v_char.push_back(cwp);
		}
		else
		{
			cwp= TextReader::test_keras2cpp_with_mat(img(charRects[i]));
			v_char.push_back(cwp);
		}
	}
	//printf("size =%d \n",v_char.size());
	float max_tmp=0.0;
	int char_location;
	//printf("location_minus= %d \n",location_minus);
	if (location_minus<3)
	{
		for (int i=0;i<(int)v_char.size();i++)
		{
			if (i!=location_minus)
			{
				if ((v_char[i].num_pro>v_char[i].neg_pro)&&(v_char[i].num_pro>v_char[i].char_pro))
				{
					v_char[i].c=v_char[i].num_c;
					v_char[i].pro=v_char[i].num_pro;
				}
				else if ((v_char[i].char_pro>v_char[i].neg_pro)&&(v_char[i].char_pro>v_char[i].num_pro))
				{
					v_char[i].c=v_char[i].char_c;
					v_char[i].pro=v_char[i].char_pro;
				}
				else
				{
					v_char[i].c=v_char[i].neg_c;
					v_char[i].pro=v_char[i].neg_pro;
				}

			}	
		}
		for (int i=0;i<(int) v_char.size();i++)
		{
			plateStr +=v_char[i].c;
			if (v_char[i].c!='_')
			{
				pro += v_char[i].pro;
			}		
			proEachChar += to_string(v_char[i].pro) + "_";
			//printf("char=%c pro=%f\n",v_char[i].c,v_char[i].pro);
		}

		swp.PlateStr = plateStr;
		swp.pro = pro;
		swp.proEachChar = proEachChar;
		swp.num_char=9;
		v_char.clear();
	//	cout << swp.PlateStr  << "\t"<<swp.pro <<endl;
		return (swp);
	}
//	printf("location minus=%d, char_location=%d\n",location_minus,char_location);
	for (int i=2;i<location_minus;i++)
	{
		if (v_char[i].char_pro>max_tmp)
		{
			max_tmp=v_char[i].char_pro;
			char_location=i;
		}
	}
	//printf("char_location=%d\n",char_location);
	// for (int i=0;i<v_char.size();i++)
	// {
	// //	printf("i=%d char=%c pro_char=%f num=%c,pro_num=%f neg=%c,pro_neg=%f\n",i,v_char[i].char_c,v_char[i].char_pro,v_char[i].num_c,v_char[i].num_pro,v_char[i].neg_c,v_char[i].neg_pro);
	// }
	v_char[char_location].c=v_char[char_location].char_c;
	v_char[char_location].pro=v_char[char_location].char_pro;
	if (char_location==2) // truoc day co 2 ki tu, bat buoc phai la so
	{
		for (int i=0;i<char_location;i++)
		{
			v_char[i].c=v_char[i].num_c;
			v_char[i].pro=v_char[i].num_pro;
		}
		if (char_location<location_minus-1)
		{
			plate_special ps;
			ps=check_special (v_char[char_location].c);
			if (ps.char_num==0) // khong phai truong hop bien dac biet
			{
				float max_num_pro=0.0;
				int max_num_index=0;
				for (int j=char_location+1;j<location_minus;j++) // Tim char co kha nang la so nhat
				{
					if (v_char[j].num_pro>max_num_pro)
					{
						max_num_pro=v_char[j].num_pro;
						max_num_index=j;
					}
				}
				for (int j=char_location+1;j<location_minus;j++)
				{
					if (j==max_num_index)
					{
						if (v_char[j].num_pro>v_char[j].neg_pro)
						{
							v_char[j].c=v_char[j].num_c;
							v_char[j].pro=v_char[j].num_pro;
						}
						else
						{
							v_char[j].c=v_char[j].neg_c;
							v_char[j].pro=v_char[j].neg_pro;
						}
					}
					else
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
				}
			}
			
			else if (ps.char_num>0) // truong hop bien dac biet
			{	
				for (int j=char_location+1;j<location_minus;j++)
				{
					if (v_char[j].num_pro>v_char[j].neg_pro)
					{
						v_char[j].c=v_char[j].num_c;
						v_char[j].pro=v_char[j].num_pro;
					}
					else 
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
					for (int k=0;k<ps.char_num;k++)
					{
						if (v_char[j].pro<v_char[j].cnn_out[ps.index[k]])
						{
							v_char[j].pro=v_char[j].cnn_out[ps.index[k]];
							v_char[j].c=ch[ps.index[k]];
						}
					}
				}
			}
		}
	}

	else if (char_location>2)
	{
		plate_special ps;
		ps=check_special_previous (v_char[char_location].c);
		int check =0;// check =1: ki tu truoc day la character, check =0: ki tu truoc day khong phai la character
		// kiem tra xem ki tu truoc day co phai la char khong
	//	printf("ps.char_num=%d \n",ps.char_num);
		if (ps.char_num>0) // co the la truong hop bien dac biet
		{
			float pro_max=0.0;
			int index_max=0;
			for (int j=0;j<ps.char_num;j++)
				{
					if (v_char[char_location-1].cnn_out[ps.index[j]]>pro_max)
					{
						pro_max=v_char[char_location-1].cnn_out[ps.index[j]];
						index_max=j;
					}
				}
			if ((pro_max>v_char[char_location-1].neg_pro)&&(pro_max>v_char[char_location-1].num_pro))
			{
				check=1;
			}
			else
			{
				check=0;
			}
			//printf("check=%d \n",check);
	 	if (check==1) // ki tu truoc day la character
			{
				v_char[char_location-1].c=ch[ps.index[index_max]];
				v_char[char_location-1].pro=pro_max;
							// find the 2 best number before
				float best_num1=0.0;
				int num1_location=0;
				int num2_location=0;
				float best_num2=0.0;
				for (int j=0;j<char_location-1;j++)
				{
					if (v_char[j].num_pro>best_num1)
					{
						best_num1=v_char[j].num_pro;
						num1_location=j;
					}					
				}
				v_char[num1_location].c=v_char[num1_location].num_c;
				v_char[num1_location].pro=v_char[num1_location].num_pro;
				v_char[num1_location].num_pro=0;
				for (int j=0;j<char_location-1;j++)
				{
					if (v_char[j].num_pro>best_num2)
					{
						best_num2=v_char[j].num_pro;
						num2_location=j;
					}					
				}
				v_char[num2_location].c=v_char[num2_location].num_c;
				v_char[num2_location].pro=v_char[num2_location].num_pro;
				for (int j=0;j<char_location-1;j++)
				{
					if ((j!=num1_location)&&(j!=num2_location))
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
				}
				for (int j=char_location+1;j<location_minus;j++)
				{
				if (v_char[j].num_pro>v_char[j].neg_pro)
					{
						v_char[j].c=v_char[j].num_c;
						v_char[j].pro=v_char[j].num_pro;
					}
					else 
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
				}
			}
			else
			{
				//	printf("asfasdfd\n");
					// find the 2 best number before
					//printf("asdfasdfadfasdfasdf\n");
					float best_num1=0.0;
					int num1_location=0;
					int num2_location=0;
					float best_num2=0.0;
					for (int j=0;j<char_location;j++)
					{
						if (v_char[j].num_pro>best_num1)
						{
							best_num1=v_char[j].num_pro;
							num1_location=j;
						}					
					}
					v_char[num1_location].c=v_char[num1_location].num_c;
					v_char[num1_location].pro=v_char[num1_location].num_pro;
					v_char[num1_location].num_pro=0;
					for (int j=0;j<char_location;j++)
					{
						if (v_char[j].num_pro>best_num2)
						{
							best_num2=v_char[j].num_pro;
							num2_location=j;
						}					
					}
					v_char[num2_location].c=v_char[num2_location].num_c;
					v_char[num2_location].pro=v_char[num2_location].num_pro;
				//	printf("num1_ location =%d, num2 =%d \n",num1_location,num2_location);
					for (int j=0;j<char_location;j++)
					{
						if ((j!=num1_location)&&(j!=num2_location))
						{
							v_char[j].c=v_char[j].neg_c;
							v_char[j].pro=v_char[j].neg_pro;
						}
					}
			if (char_location<location_minus-1)
			{
				plate_special ps1;
				ps1=check_special (v_char[char_location].c);
				if (ps1.char_num==0) // khong phai truong hop bien dac biet
				{
					float max_num_pro=0.0;
					int max_num_index=0;
					for (int j=char_location+1;j<location_minus;j++) // Tim char co kha nang la so nhat
					{
						if (v_char[j].num_pro>max_num_pro)
						{
							max_num_pro=v_char[j].num_pro;
							max_num_index=j;
						}
					}
					for (int j=char_location+1;j<location_minus;j++)
					{
						if (j==max_num_index)
						{
							if (v_char[j].num_pro>v_char[j].neg_pro)
							{
								v_char[j].c=v_char[j].num_c;
								v_char[j].pro=v_char[j].num_pro;
							}
							else
							{
								v_char[j].c=v_char[j].neg_c;
								v_char[j].pro=v_char[j].neg_pro;
							}
						}
						else
						{
							v_char[j].c=v_char[j].neg_c;
							v_char[j].pro=v_char[j].neg_pro;
						}
					}
				}
				else if (ps1.char_num>0) // truong hop bien dac biet
				{	
					//float max_char_pro=0.0;
					//int max_char_index=0;
					for (int j=char_location+1;j<location_minus;j++)
					{
						if (v_char[j].num_pro>v_char[j].neg_pro)
						{
							v_char[j].c=v_char[j].num_c;
							v_char[j].pro=v_char[j].num_pro;
						}
						else 
						{
							v_char[j].c=v_char[j].neg_c;
							v_char[j].pro=v_char[j].neg_pro;
						}
						for (int k=0;k<ps1.char_num;k++)
						{
							if (v_char[j].pro<v_char[j].cnn_out[ps1.index[k]])
							{
								v_char[j].pro=v_char[j].cnn_out[ps1.index[k]];
								v_char[j].c=ch[ps1.index[k]];
							}
						}
					}
				}
			}
			}
		}
		else if ((ps.char_num==0)||(check==0))
		{
				// find the 2 best number before
				//printf("asdfasdfadfasdfasdf\n");
				float best_num1=0.0;
				int num1_location=0;
				int num2_location=0;
				float best_num2=0.0;
				for (int j=0;j<char_location;j++)
				{
					if (v_char[j].num_pro>best_num1)
					{
						best_num1=v_char[j].num_pro;
						num1_location=j;
					}					
				}
				v_char[num1_location].c=v_char[num1_location].num_c;
				v_char[num1_location].pro=v_char[num1_location].num_pro;
				v_char[num1_location].num_pro=0;
				for (int j=0;j<char_location;j++)
				{
					if (v_char[j].num_pro>best_num2)
					{
						best_num2=v_char[j].num_pro;
						num2_location=j;
					}					
				}
				v_char[num2_location].c=v_char[num2_location].num_c;
				v_char[num2_location].pro=v_char[num2_location].num_pro;
				//printf("num1_ location =%d, num2 =%d \n",num1_location,num2_location);
				for (int j=0;j<char_location;j++)
				{
					if ((j!=num1_location)&&(j!=num2_location))
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
				}
		if (char_location<location_minus-1)
		{
			plate_special ps1;
			ps1=check_special (v_char[char_location].c);
			if (ps1.char_num==0) // khong phai truong hop bien dac biet
			{
				float max_num_pro=0.0;
				int max_num_index=0;
				for (int j=char_location+1;j<location_minus;j++) // Tim char co kha nang la so nhat
				{
					if (v_char[j].num_pro>max_num_pro)
					{
						max_num_pro=v_char[j].num_pro;
						max_num_index=j;
					}
				}
				for (int j=char_location+1;j<location_minus;j++)
				{
					if (j==max_num_index)
					{
						if (v_char[j].num_pro>v_char[j].neg_pro)
						{
							v_char[j].c=v_char[j].num_c;
							v_char[j].pro=v_char[j].num_pro;
						}
						else
						{
							v_char[j].c=v_char[j].neg_c;
							v_char[j].pro=v_char[j].neg_pro;
						}
					}
					else
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
				}
			}
			else if (ps1.char_num>0) // truong hop bien dac biet
			{	
				//float max_char_pro=0.0;
				//int max_char_index=0;
				for (int j=char_location+1;j<location_minus;j++)
				{
					if (v_char[j].num_pro>v_char[j].neg_pro)
					{
						v_char[j].c=v_char[j].num_c;
						v_char[j].pro=v_char[j].num_pro;
					}
					else 
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
					for (int k=0;k<ps1.char_num;k++)
					{
						if (v_char[j].pro<v_char[j].cnn_out[ps1.index[k]])
						{
							v_char[j].pro=v_char[j].cnn_out[ps1.index[k]];
							v_char[j].c=ch[ps1.index[k]];
						}
					}
				}
			}
		}
		}

	}

	int num_bottom=0;
	int num_afterchar=0;
	for (int j=char_location+1;j<location_minus;j++)
	{
		if ((v_char[j].c!='_')&&(v_char[j].num_pro>v_char[j].char_pro))
		{
			num_afterchar++;
		}
	}
	while (num_afterchar>=2)
	{
		int min_index=0;
		float min_pro=100;
		for (int j=char_location+1;j<location_minus;j++)
		{
			if ((v_char[j].c!='_')&&(v_char[j].num_pro>v_char[j].char_pro))// hien dang duoc coi la so
			{
				if (v_char[j].num_pro<min_pro)
				{
					min_pro=v_char[j].num_pro;
					min_index=j;
				}
			}
		}
		v_char[min_index].c=v_char[min_index].neg_c;
		v_char[min_index].pro=v_char[min_index].neg_pro;
		v_char[min_index].num_pro=10;
		num_afterchar--;
	}
	if (v_char.size()-location_minus<=5)
	{
		for (int j=location_minus+1;j<(int) v_char.size();j++)
		{
			v_char[j].c=v_char[j].num_c;
			v_char[j].pro=v_char[j].num_pro;
		}
	}
	else
	{
		for (int j=location_minus+1;j<(int) v_char.size();j++)
		{
			//printf("j=%d num_pro=%f neg_pro=%f \n",j,v_char[j].num_pro,v_char[j].neg_pro);
			if (v_char[j].num_pro>v_char[j].neg_pro)
			{
				v_char[j].c=v_char[j].num_c;
				v_char[j].pro=v_char[j].num_pro;
				num_bottom++;
			}
			else 
			{
				v_char[j].c=v_char[j].neg_c;
				v_char[j].pro=v_char[j].neg_pro;
			}		
		}
		// for (int k=0;k<v_char.size();k++)
		// {
		// 	printf("%c \t %f \n",v_char[k].c,v_char[k].pro);
		// }
		if (num_bottom<4) // tim 4 chu so co xac xuat la so cao nhat
		{
			int num1=0;
			while (num1<4)
			{
				int max_index=0;
				float max_pro=0;
				for (int j=location_minus+1;j<(int) v_char.size();j++)
				{
					if (v_char[j].num_pro>max_pro)
					{
						max_index=j;
						max_pro=v_char[j].num_pro;
					}
				}
				v_char[max_index].c=v_char[max_index].num_c;
				v_char[max_index].pro=v_char[max_index].num_pro;
				v_char[max_index].num_pro=0;
				num1++;
			}
		}
		else if (num_bottom>=6)
		{
			while (num_bottom>=6)
			{
				int min_index=0;
				float min_pro=100;
				for (int j=location_minus+1;j<(int) v_char.size();j++)
				{
					if (v_char[j].num_pro>v_char[j].neg_pro)// hien dang duoc coi la so
					{
						if (v_char[j].num_pro<min_pro)
						{
							min_pro=v_char[j].num_pro;
							min_index=j;
						}
					}
				}
			//	printf("num_bottom=%d, min_index=%d min_pro=%f\n",num_bottom,min_index,min_pro);
				v_char[min_index].c=v_char[min_index].neg_c;
				v_char[min_index].pro=v_char[min_index].neg_pro;
				v_char[min_index].num_pro=0;
				num_bottom--;
			}
		}	
	}

	int char_position[20];
	int count=0;
	for (int i=0;i<location_minus;i++)
	{
		if ((v_char[i].c!='_')&&(v_char[i].c!='-'))
		{
			char_position[count]=i;
			count++;
		}
	}
	swp.num_char=count;
	//printf("swp.num_char=%d\n",swp.num_char);
	if (count>=5)
		{
			if (((v_char[char_position[2]].c=='L')||(v_char[char_position[2]].c=='M'))&&(v_char[char_position[3]].c=='0'))
			{
				v_char[char_position[3]].c='D';
			}
		}

	
	
	for (int i=0;i<(int) v_char.size();i++)
	{
		plateStr +=v_char[i].c;
		if (v_char[i].c!='_')
		{
			pro += v_char[i].pro;
		}		
		proEachChar += to_string(v_char[i].pro) + "_";
		//cout<<v_char[i].c<<"\t"<<v_char[i].pro<<endl;
		//printf("char=%c pro=%f\n",v_char[i].c,v_char[i].pro);
	}
	
	
	// if (plateStr.length()>=7)
	{
		swp.PlateStr = plateStr;
		swp.pro = pro;
		swp.proEachChar = proEachChar;
	}
	swp.vector_cwp=v_char;
	v_char.clear();
//	cout <<"Bien vuong \t"<< swp.PlateStr  << "\t"<<swp.pro <<endl;

		
	return (swp);
}
stringWithPro TextReader::GetPlateString_long(std::vector<cv::Rect> charRects, cv::Mat& img)
{
	stringWithPro swp;
	std::string plateStr = "";
	float pro = 0;
	string proEachChar = "";
	//int location_minus=100;
	int char_location_true=4;
	//float threshold_character=0.5; // threshold for character
	/*for (int i = 0; i < (int)charRects.size(); i++)
	{
		Rect r1 = charRects[i];
		if (r1.x==0 && r1.y==0 && r1.width==0 && r1.height==0) //tim vi tri dau "-"
			location_minus = i;
	}*/
	//mode: 1 = Num + Word + Negative; 	2 = Num + Word; 
	//		3 = Num + Negative; 		4 = Num; 
	//		5 = Word + Negative; 		6 = Word; 
	//		7= World + Num (1-9) + Negative
	charWithPro cwp;
	//int n = (int)charRects.size();
	//int k=1;
	vector<charWithPro> v_char;
	//printf("size=%d \n",n);
	for (int i=0;i<(int)charRects.size();i++)
	{
		if ((charRects[i].width==0) && (charRects[i].height==0)&&charRects[i].x==0 && charRects[i].y==0) 
		{
			cwp.c='-';
			cwp.pro=1;
			cwp.num_pro=0;
			cwp.char_pro=0;
			cwp.neg_pro=0;
			v_char.push_back(cwp);
			//printf("location minus=%d \n",i);
		}
		else
		{
			cwp= test_keras2cpp_with_mat(img(charRects[i]));
			v_char.push_back(cwp);
		}
	}
	// find the location of the character
	float max_tmp=0.0;
	int char_location;
	for (int i=2;i<6;i++)
	{
		if (v_char[i].char_pro>max_tmp)
		{
			max_tmp=v_char[i].char_pro;
			char_location=i;
		}
	}
	v_char[char_location].c=v_char[char_location].char_c;
	v_char[char_location].pro=v_char[char_location].char_pro;
	char_location_true=char_location;
	//printf("v_char.size=%d , char_location=%d\n",v_char.size(),char_location);
	if (char_location==2) // truoc day co 2 ki tu, bat buoc phai la so
	{
		for (int i=0;i<char_location;i++)
		{
			v_char[i].c=v_char[i].num_c;
			v_char[i].pro=v_char[i].num_pro;
		}
		if ((v_char[char_location].c=='A')||(v_char[char_location].c=='L')||(v_char[char_location].c=='N')||(v_char[char_location].c=='K'))
		{
			int num_num=0;// number of digit character
			int num_mode=0; //num_mode =1 if char[location+2] is num, otherwise num_mode=0;
			//printf("char =%f num=%f neg=%f \n",v_char[char_location+1].char_pro,v_char[char_location+1].num_pro,v_char[char_location+1].neg_pro );
			if ((v_char[char_location+1].neg_pro>v_char[char_location+1].num_pro)&&(v_char[char_location+1].neg_pro>v_char[char_location+1].char_pro))
			{// ki tu tiep theo la negative
				v_char[char_location+1].c='_';
				
				v_char[char_location+1].pro=v_char[char_location+1].neg_pro;
				if ((v_char[char_location+2].char_pro>v_char[char_location+2].num_pro)&&(v_char[char_location+2].char_pro>v_char[char_location+2].neg_pro))
				{
					v_char[char_location+2].pro=v_char[char_location+2].char_pro;
					char_location_true=char_location+2;
					v_char[char_location+2].c=v_char[char_location+2].char_c;
				}
				else if ((v_char[char_location+2].num_pro>v_char[char_location+2].char_pro)&&(v_char[char_location+2].num_pro>v_char[char_location+2].neg_pro))
				{	v_char[char_location+2].pro=v_char[char_location+2].num_pro;
					v_char[char_location+2].c=v_char[char_location+2].num_c;
					num_num++;
					num_mode=1;
				}
				else
				{
					v_char[char_location+2].pro=v_char[char_location+2].neg_pro;
					v_char[char_location+2].c=v_char[char_location+2].neg_c;
				}
				for (int j=char_location+3;j<(int)v_char.size();j++)
				{
					if (v_char[j].num_pro>v_char[j].neg_pro)
					{
						v_char[j].c=v_char[j].num_c;
						v_char[j].pro=v_char[j].num_pro;
						num_num++;
					}
					else 
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
				}
				float num_pro_min=1; //pro of digit that has least confident
				int min_index; //pro of digit that has ...
				if((num_mode==1)&&(num_num>=7))
				{
					for (int j=char_location+2;j<(int)v_char.size();j++)
					{
						if (v_char[j].pro<num_pro_min)
						{
							num_pro_min=v_char[j].pro;
							min_index=j;
						}
					}
					v_char[min_index].c=v_char[min_index].neg_c;
					v_char[min_index].pro=v_char[min_index].neg_pro;
				}
				else if ((num_mode==0)&&(num_num>=6))
				{
					for (int j=char_location+3;j<(int)v_char.size();j++)
					{
						if (v_char[j].pro<num_pro_min)
						{
							num_pro_min=v_char[j].pro;
							min_index=j;
						}
					}
					v_char[min_index].c=v_char[min_index].neg_c;
					v_char[min_index].pro=v_char[min_index].neg_pro;
				}					
		
			}
			else
			{
			//printf("fadfa\n");
				if (v_char[char_location+1].char_pro>v_char[char_location+1].num_pro)
				{
					v_char[char_location+1].pro=v_char[char_location+1].char_pro;
					v_char[char_location+1].c=v_char[char_location+1].char_c;
					char_location_true=char_location+1;
				}
				else if  (v_char[char_location+1].num_pro>v_char[char_location+1].char_pro)
				{
					v_char[char_location+1].pro=v_char[char_location+1].num_pro;
					v_char[char_location+1].c=v_char[char_location+1].num_c;
				}
				else
				{
					v_char[char_location+1].pro=v_char[char_location+1].neg_pro;
					v_char[char_location+1].c=v_char[char_location+1].neg_c;
				}
				for (int j=char_location+2;j<(int)v_char.size();j++)
				{
					if (v_char[j].num_pro>v_char[j].neg_pro)
					{
						v_char[j].c=v_char[j].num_c;
						v_char[j].pro=v_char[j].num_pro;
					}
					else 
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
				}
			}
		}
		else
		{
			for (int j=char_location+1;j<(int)v_char.size();j++)
			{
				if (v_char[j].num_pro>v_char[j].neg_pro)
					{
						v_char[j].c=v_char[j].num_c;
						v_char[j].pro=v_char[j].num_pro;
					}
					else 
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
			}

		}
	}
	else if (char_location>2)
	{
		// for (int j=0;j<char_location;j++)
		// {
		// 	printf("11j=%d num=%c pro=%f \n",j,v_char[j].num_c,v_char[j].num_pro);
		// }
		// check if the previouse Rect is also a character (in case of  AA, LD,NG)
		if ((v_char[char_location-1].char_pro>v_char[char_location-1].num_pro) && (v_char[char_location-1].char_pro>v_char[char_location-1].neg_pro)&&((v_char[char_location].c=='A')||(v_char[char_location].c=='D')||(v_char[char_location].c=='G')||(v_char[char_location].c=='T'))) 
		{
			v_char[char_location-1].c=v_char[char_location-1].char_c;
			v_char[char_location-1].pro=v_char[char_location-1].char_pro;
			// find the 2 best number before
			float best_num1=0.0;
			int num1_location=0;
			int num2_location=0;
			float best_num2=0.0;
			for (int j=0;j<char_location-1;j++)
				{
					if (v_char[j].num_pro>best_num1)
					{
						best_num1=v_char[j].num_pro;
						num1_location=j;
					}					
				}
			v_char[num1_location].c=v_char[num1_location].num_c;
			v_char[num1_location].pro=v_char[num1_location].num_pro;
			v_char[num1_location].num_pro=0;
			for (int j=0;j<char_location-1;j++)
				{
					if (v_char[j].num_pro>best_num2)
					{
						best_num2=v_char[j].num_pro;
						num2_location=j;
					}					
				}
			v_char[num2_location].c=v_char[num2_location].num_c;
			v_char[num2_location].pro=v_char[num2_location].num_pro;
			for (int j=0;j<char_location-1;j++)
				{
					if ((j!=num1_location)&&(j!=num2_location))
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
				}
			for (int j=char_location+1;j<(int)v_char.size();j++)
			{
				if (v_char[j].num_pro>v_char[j].neg_pro)
					{
						v_char[j].c=v_char[j].num_c;
						v_char[j].pro=v_char[j].num_pro;
					}
					else 
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
			}

		}
		else // the previous rect is not a charater, 2 two best rect are num
		{
			float best_num1=0.0;
			int num1_location=0;
			int num2_location=0;
			float best_num2=0.0;
			for (int j=0;j<char_location;j++)
				{
					if (v_char[j].num_pro>best_num1)
					{
						best_num1=v_char[j].num_pro;
						num1_location=j;
					}					
				}
			v_char[num1_location].c=v_char[num1_location].num_c;
			v_char[num1_location].pro=v_char[num1_location].num_pro;
			v_char[num1_location].num_pro=0;
			for (int j=0;j<char_location;j++)
				{
					if (v_char[j].num_pro>best_num2)
					{
						best_num2=v_char[j].num_pro;
						num2_location=j;
					}					
				}
			v_char[num2_location].c=v_char[num2_location].num_c;
			v_char[num2_location].pro=v_char[num2_location].num_pro;
			for (int j=0;j<char_location;j++)
				{
					if ((j!=num1_location)&&(j!=num2_location))
					{
						v_char[j].c=v_char[j].neg_c;
						v_char[j].pro=v_char[j].neg_pro;
					}
				}
			if ((v_char[char_location].c=='A')||(v_char[char_location].c=='L')||(v_char[char_location].c=='N')||(v_char[char_location].c=='K'))
			{
				if ((v_char[char_location+1].neg_pro>v_char[char_location+1].num_pro)&&(v_char[char_location+1].neg_pro>v_char[char_location+1].char_pro))
					{// ki tu v_char[j]tiep theo la negative
						v_char[char_location+1].c='_';
						v_char[char_location+1].pro=v_char[char_location+1].neg_pro;
						if ((v_char[char_location+2].char_pro>v_char[char_location+2].num_pro)&&(v_char[char_location+2].char_pro>v_char[char_location+2].neg_pro))
						{
							v_char[char_location+2].pro=v_char[char_location+2].char_pro;
							v_char[char_location+2].c=v_char[char_location+2].char_c;
							char_location_true=char_location+2;
						}
						else if ((v_char[char_location+2].num_pro>v_char[char_location+2].char_pro)&&(v_char[char_location+2].num_pro>v_char[char_location+2].neg_pro))
						{
							v_char[char_location+2].pro=v_char[char_location+2].num_pro;
							v_char[char_location+2].c=v_char[char_location+2].num_c;
						}
						else
						{
							v_char[char_location+2].pro=v_char[char_location+2].neg_pro;
							v_char[char_location+2].c=v_char[char_location+2].neg_c;
						}
						for (int j=char_location+3;j<(int)v_char.size();j++)
						{
							if (v_char[j].num_pro>v_char[j].neg_pro)
							{
								v_char[j].c=v_char[j].num_c;
								v_char[j].pro=v_char[j].num_pro;
							}
							else 
							{
								v_char[j].c=v_char[j].neg_c;
								v_char[j].pro=v_char[j].neg_pro;
							}
						}
					}
				else
				{
					if (v_char[char_location+1].char_pro>v_char[char_location+1].num_pro)
					{
						v_char[char_location+1].pro=v_char[char_location+1].char_pro;
						v_char[char_location+1].c=v_char[char_location+1].char_c;
						char_location_true=char_location+1;
					}
					else if  (v_char[char_location+1].num_pro>v_char[char_location+1].char_pro)
					{
						v_char[char_location+1].pro=v_char[char_location+1].num_pro;
						v_char[char_location+1].c=v_char[char_location+1].num_c;
					}
					else
					{
						v_char[char_location+1].pro=v_char[char_location+1].neg_pro;
						v_char[char_location+1].c=v_char[char_location+1].neg_c;
					}
					for (int j=char_location+2;j<(int)v_char.size();j++)
					{
						if (v_char[j].num_pro>v_char[j].neg_pro)
						{
							v_char[j].c=v_char[j].num_c;
							v_char[j].pro=v_char[j].num_pro;
						}
						else 
						{
							v_char[j].c=v_char[j].neg_c;
							v_char[j].pro=v_char[j].neg_pro;
						}
					}
				}
			}
			else
			{
				for (int j=char_location+1;j<(int)v_char.size();j++)
				{
					if (v_char[j].num_pro>v_char[j].neg_pro)
						{
							v_char[j].c=v_char[j].num_c;
							v_char[j].pro=v_char[j].num_pro;
						}
						else 
						{
							v_char[j].c=v_char[j].neg_c;
							v_char[j].pro=v_char[j].neg_pro;
						}
				}
			}
		}	
	}
	int num_afterchar=0;

	if (v_char.size()-char_location_true<=5)
	{
		for (int j=char_location_true+1;j<(int) v_char.size();j++)
		{
			v_char[j].c=v_char[j].num_c;
			v_char[j].pro=v_char[j].num_pro;
		}
	}	
	else
	{
		for (int j=char_location_true+1;j<(int) v_char.size();j++)
		{
			if ((v_char[j].c!='_')&&(v_char[j].num_pro>v_char[j].char_pro))
			{
				num_afterchar++;
			}
		}
		if (num_afterchar<4)
		{
			int num1=0;
			while (num1<4)
			{
				int max_index=0;
				float max_pro=0;
				for (int j=char_location+1;j<(int) v_char.size();j++)
				{
					if (v_char[j].num_pro>max_pro)
					{
						max_index=j;
						max_pro=v_char[j].num_pro;
					}
				}
				v_char[max_index].c=v_char[max_index].num_c;
				v_char[max_index].pro=v_char[max_index].num_pro;
				v_char[max_index].num_pro=0;
				num1++;
			}
		}
		else if (num_afterchar>=6)
		{
			while (num_afterchar>=6)
			{
				int min_index=0;
				float min_pro=100;
				for (int j=char_location;j<(int) v_char.size();j++)
				{
					if ((v_char[j].c!='_')&&(v_char[j].num_pro>v_char[j].char_pro))// hien dang duoc coi la so
					{
						if (v_char[j].num_pro<min_pro)
						{
							min_pro=v_char[j].num_pro;
							min_index=j;
						}
					}
				}
				v_char[min_index].c=v_char[min_index].neg_c;
				v_char[min_index].pro=v_char[min_index].neg_pro;
				v_char[min_index].num_pro=0;
				num_afterchar--;
			}
		}
	}
	int count=0;
	int char_position[20];
	for (int i=0;i<(int) v_char.size();i++)
	{
		if (v_char[i].c!='_')
		{
			char_position[count]=i;
			count++;
		}
	}
	swp.num_char=count;
	if (count>=8)
		{
			if (((v_char[char_position[2]].c=='L')||(v_char[char_position[2]].c=='M'))&&(v_char[char_position[3]].c=='0'))
			{
				v_char[char_position[3]].c='D';
			}
		}

	//printf("adfadf\n");
	swp.vector_cwp=v_char;
	for (int i=0;i<(int) v_char.size();i++)
	{
		plateStr +=v_char[i].c;
		if (v_char[i].c!='_')
		{
			pro += v_char[i].pro;
		}		
		proEachChar += to_string(v_char[i].pro) + "_";
	//	printf("char=%c pro=%f\n",v_char[i].c,v_char[i].pro);
	}
	
	swp.PlateStr = "";
	// if (plateStr.length()>=7)
	{
		swp.PlateStr = plateStr;
		swp.pro = pro;
		swp.proEachChar = proEachChar;
	}
	v_char.clear();
//	cout << "biendai \t"<<swp.PlateStr  <<"\t" <<swp.pro <<endl;
	//printf("done\n");
	return (swp);
}
stringWithPro TextReader::GetPlateString_square_blue(std::vector<cv::Rect> charRects, cv::Mat& img)
{
	stringWithPro swp;
	return (swp);
}
stringWithPro TextReader::GetPlateString_square_red(std::vector<cv::Rect> charRects, cv::Mat& img)
{
	stringWithPro swp;
	std::string plateStr = "";
	string proEachChar = "";
	float pro = 0;
	swp.PlateStr = "";
	swp.pro=0;
	swp.num_char=0;
	swp.PlateStr="";
	swp.proEachChar="";
	charWithPro cwp;
	//int n = (int)charRects.size();
	//int k=1;
	int location_minus;
	for (int i = 0; i < (int)charRects.size(); i++)
	{
		Rect r1 = charRects[i];
		if (r1.x==0 && r1.y==0 && r1.width==0 && r1.height==0) //tim vi tri dau "-"
			{
			location_minus = i;
			break;
			}
	}
	vector<charWithPro> v_char;
	//printf("location_minus=%d \n",location_minus);
	for (int i=0;i<(int)charRects.size();i++)
	{
		if (i==location_minus) 
		{
			cwp.c='-';
			cwp.pro=1;
			cwp.num_pro=0;
			cwp.char_pro=0;
			cwp.neg_pro=0;
			v_char.push_back(cwp);
		}
		else
		{
			cwp= TextReader::test_keras2cpp_with_mat(img(charRects[i]));
			v_char.push_back(cwp);
		//	cout<<cwp.c<<"\t"<<cwp.num_c<<"\t"<<cwp.char_c<<"\t"<<cwp.neg_c<<endl;
		}
	}
	// ------------------------Xu li hang tren-------------------------------
	if (location_minus<=2)
	{
		for (int i=0;i<location_minus;i++)
		{
			v_char[i].c=v_char[i].char_c;
			v_char[i].pro=v_char[i].char_pro;
		}

	}
	else //location_minus>=2
	{
		 //// tim 2 charect co xac xuat la ki tu cao nhat
		 for (int i=0;i<location_minus;i++)
		 {
			v_char[i].c=v_char[i].neg_c;
			v_char[i].pro=v_char[i].neg_pro; 
		 }
		for (int i=0;i<2;i++) 
		{
			int max_char_index=0;
			float max_char_pro=0.0;
			for(int j=0;j<location_minus;j++)
			{
				if (v_char[j].char_pro>max_char_pro)
				{
					max_char_pro=v_char[j].char_pro;
					max_char_index=j;
				}
			}
			v_char[max_char_index].c=v_char[max_char_index].char_c;
			v_char[max_char_index].pro=v_char[max_char_index].char_pro;
			v_char[max_char_index].char_pro=0;
		}
	}
	//------------------------------Xu li hang duoi----------------------------
//	printf("location minus=%d v_char size= %d \n",location_minus,v_char.size());
	if ((v_char.size()-location_minus)<=5)
	{
		for (int i=location_minus+1;i<(int) v_char.size();i++)
		{
			v_char[i].c=v_char[i].num_c;
			v_char[i].pro=v_char[i].num_pro;
		}

	}
	else //location_minus>=2
	{
		 //// tim 2 charect co xac xuat la ki tu cao nhat
		 for (int i=location_minus+1;i<(int) v_char.size();i++)
		 {
			v_char[i].c=v_char[i].neg_c;
			v_char[i].pro=v_char[i].neg_pro; 
		 }
		for (int i=0;i<4;i++) 
		{
			int max_num_index=0;
			float max_num_pro=0.0;
			for(int j=location_minus+1;j<(int) v_char.size();j++)
			{
				if (v_char[j].num_pro>max_num_pro)
				{
					max_num_pro=v_char[j].num_pro;
					max_num_index=j;
				}
			}
			v_char[max_num_index].c=v_char[max_num_index].num_c;
			v_char[max_num_index].pro=v_char[max_num_index].num_pro;
			v_char[max_num_index].num_pro=0;
		}
	}
	int count=0;
	for (int i=0;i<(int) v_char.size();i++)
	{
		if ((v_char[i].c!='_')&&(v_char[i].c!='-'))
		{
			count++;
		}
	}
	swp.num_char=count;
	swp.vector_cwp=v_char;
	for (int i=0;i<(int) v_char.size();i++)
	{
		plateStr +=v_char[i].c;
		if (v_char[i].c!='_')
		{
			pro += v_char[i].pro;
		}		
		proEachChar += to_string(v_char[i].pro) + "_";
	//	printf("char=%c pro=%f\n",v_char[i].c,v_char[i].pro);
	}
	
	swp.PlateStr = "";
	// if (plateStr.length()>=7)
	{
		swp.PlateStr = plateStr;
		swp.pro = pro;
		swp.proEachChar = proEachChar;
	}
	//cout << "bien do vuong:\t" <<swp.PlateStr <<  "\t" <<swp.pro <<endl;
	v_char.clear();
	return (swp);
}
stringWithPro TextReader::GetPlateString_long_red(std::vector<cv::Rect> charRects, cv::Mat& img)
{
	stringWithPro swp;
	std::string plateStr = "";
	string proEachChar = "";
	float pro = 0;
	swp.PlateStr = "";
	swp.pro=0;
	swp.num_char=0;
	swp.PlateStr="";
	swp.proEachChar="";
	charWithPro cwp;
	vector<charWithPro> v_char;
	for (int i=0;i<(int)charRects.size();i++)
	{
		cwp= TextReader::test_keras2cpp_with_mat(img(charRects[i]));
		v_char.push_back(cwp);
	}
	int char_location1=0;// position of the first character
	int char_location2=0;// position of the second character
	int char_location=3; // the position of the last character
	float min_pro=0;
	for (int i=0;i<5;i++)
	{
		if (v_char[i].char_pro>min_pro)
		{
			min_pro=v_char[i].char_pro;
			char_location1=i;
		}
	}
	v_char[char_location1].c=v_char[char_location1].char_c;
	v_char[char_location1].pro=v_char[char_location1].char_pro;
	v_char[char_location1].char_pro=0;
	min_pro=0;
	for (int i=0;i<5;i++)
	{
		if (v_char[i].char_pro>min_pro)
		{
			min_pro=v_char[i].char_pro;
			char_location2=i;
		}
	}
	v_char[char_location2].c=v_char[char_location2].char_c;
	v_char[char_location2].pro=v_char[char_location2].char_pro;
	if (char_location1>char_location2)
	{
		char_location=char_location1;
	}
	else
	{
		char_location=char_location2;
	}
	for (int i=char_location+1;i<(int)v_char.size();i++)
	{
		if (v_char[i].num_pro>v_char[i].neg_pro)
		{
			v_char[i].c=v_char[i].num_c;
			v_char[i].pro=v_char[i].num_pro;
		}
		else
		{
			v_char[i].c=v_char[i].neg_c;
			v_char[i].pro=v_char[i].neg_pro;
		}
	}
	for (int i=0;i<(int) v_char.size();i++)
	{
		plateStr +=v_char[i].c;
		if (v_char[i].c!='_')
		{
			pro += v_char[i].pro;
		}		
		proEachChar += to_string(v_char[i].pro) + "_";
		//printf("char=%c pro=%f\n",v_char[i].c,v_char[i].pro);
	}
	
	swp.PlateStr = "";
	// if (plateStr.length()>=7)
	{
		swp.PlateStr = plateStr;
		swp.pro = pro;
		swp.proEachChar = proEachChar;
	}
	int count=0;
	for (int i=0;i<(int) v_char.size();i++)
	{
		if ((v_char[i].c!='_')&&(v_char[i].c!='-'))
		{
			count++;
		}
	}
	swp.num_char=count;
	v_char.clear();
	//cout << "bien do dai:\t" <<swp.PlateStr <<  "\t" <<swp.pro <<endl;
	return (swp);
}
stringWithPro TextReader::GetPlateString_long_blue(std::vector<cv::Rect> charRects, cv::Mat& img)
{
	stringWithPro swp;
	return (swp);
}
stringWithPro TextReader::GetPlateString(std::vector<cv::Rect> charRects, cv::Mat& img,int plate_mode) 
{
	stringWithPro swp;
	stringWithPro swp1;
	stringWithPro swp2;
	std::string plateStr = "";
	//float pro = 0;
	string proEachChar = "";
//	int location_minus=0;
	//printf("plate_mode =%d \n",plate_mode);

	if (plate_mode==0)
	{
		
		swp=TextReader::GetPlateString_square(charRects, img);
	//	cout << "bien trang vuong:\t" <<swp.PlateStr <<  "\t" <<swp.pro <<endl;
	}
	else if (plate_mode==1)
	{
		
		swp=TextReader::GetPlateString_long( charRects, img);
	//	cout << "bien trang dai:\t" <<swp.PlateStr <<  "\t" <<swp.pro <<endl;
	}
	else if (plate_mode==2)
	{
		// if (img.channels() != 1) 
		// {
		// 	cv::cvtColor(img, img, CV_BGR2GRAY);
		// }
		bitwise_not(img,img);
		swp1=TextReader::GetPlateString_square_red( charRects, img);
		swp2=TextReader::GetPlateString_square(charRects, img);
		//cout << "bien do vuong:\t" <<swp1.PlateStr <<  "\t" <<(float) swp1.pro/(float)swp1.num_char <<endl;
	//	cout << "bien xanh vuong:\t" <<swp2.PlateStr <<  "\t" <<(float) swp2.pro/(float)swp2.num_char <<endl;
		if ((float) swp1.pro/(float)swp1.num_char<(float) swp2.pro/(float)swp2.num_char)
		{
			swp=swp2;
			
		}
		else
		{
			swp=swp1;
		}
	//	cout << "bien mau vuong:\t" <<swp.PlateStr <<  "\t" <<swp.pro <<endl;
	}
	else if (plate_mode==3)
	{
		// if (img.channels() != 1) 
		// {
		// 	cv::cvtColor(img, img, CV_BGR2GRAY);
		// }
		bitwise_not(img,img);
		swp1=TextReader::GetPlateString_long_red( charRects, img);
		//cout << "bien do dai:\t" <<swp1.PlateStr <<  "\t" <<(float) swp1.pro/(float)swp1.num_char <<endl;
		swp2=TextReader::GetPlateString_long(charRects, img);
		//cout << "bien xanh dai:\t" <<swp2.PlateStr <<  "\t" <<(float) swp2.pro/(float)swp2.num_char <<endl;
		if ((float) swp1.pro/(float)swp1.num_char<(float) swp2.pro/(float)swp2.num_char)
		{
			swp=swp2;
			//cout << "bien xanh dai:\t" <<swp.PlateStr <<  "\t" <<swp.pro <<endl;
		}
		else
		{
			swp=swp1;
			//cout << "bien do dai:\t" <<swp.PlateStr <<  "\t" <<swp.pro <<endl;
		}
	//	cout << "plate mode = "<<plate_mode<<"\t bien mau dai:\t" <<swp.PlateStr <<  "\t" <<swp.pro <<endl;
		
	}
//	cout << "plate mode = "<<plate_mode<<"\t" <<swp.PlateStr <<  "\t" <<swp.pro <<endl;
	return (swp);
}	