#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>
#include <ctime>
#include <queue>
#include <thread>
#include <mutex>
#include <exception>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "Onvif_controller.h"
#include "Onvif.h"
#include "xmlParser.h"

//using namespace pr;
using namespace std;
using namespace cv;

int exposure_value[17]={12,25,30,35,50,100,150,200,250,300,400,500,1000,2000,4000,6000,8000};
OnvifController onvifController("10.12.11.206:2000");
//OnvifController onvifController("192.168.1.20:2000");

//OnvifController onvifController("10.12.11.149:2000"); // Cam a Tuc
int check_range=2;

Onvif_controller::Onvif_controller(
    queue<image_details> &queue_)
    : OnvifDetailsQueue(queue_)
{
}

float exposure_tuning(queue<image_details> &OnvifDetailsQueue)
{
    int current_exposure=onvifController.getExposureTime();
    int current_exposure_level=0;
    cout<<"current_Exposure_time is:\t"<<current_exposure<<endl;
	for (int i=0;i<17;i++)
    {
        if (exposure_value[i]==current_exposure)
        {
            current_exposure_level=i;
            break;
        }
     }
     int down_bound=max(0,current_exposure_level-check_range);
     int up_bound=min(16,current_exposure_level+check_range);
	
      
    //int current_exposure=1000;
       cout<<"BBBBBBBBBBBBB \t"<<current_exposure<<endl;
   // cout<<"down_bound=\t "<<down_bound<<"\t up_bound= \t"<<up_bound<<endl;
   
    float max_blurriness=0;
    int max_index=0;
    for (int i=down_bound;i<=up_bound;i++)
    {
        onvifController.setExposureTime(exposure_value[i], "VideoSource0");
        sleep(20);
        while (!OnvifDetailsQueue.empty())
        {
            OnvifDetailsQueue.pop();
            //sleep_for(chrono::milliseconds(20));
        }
        float blurriness=0;
        int count=0;
        while (count<30)
        {
            if (!OnvifDetailsQueue.empty())
            {
                blurriness+=OnvifDetailsQueue.front().blurriness;
                OnvifDetailsQueue.pop();
              //  cout<<"i=:\t"<<i<<"\t count=\t"<<count<<endl;
                count++;
            }
            this_thread::sleep_for(chrono::milliseconds(20));

        }
        blurriness=blurriness/(float)count;
        if (blurriness>max_blurriness)
        {
            max_blurriness=blurriness;
            max_index=i;
        }
        
    }
    onvifController.setExposureTime(exposure_value[max_index], "VideoSource0");
    cout<<"new exposure time is\t"<< exposure_value[max_index]<<"blurriness value is:\t"<<max_blurriness<<endl;
    return (max_blurriness);
}

void Onvif_controller::operator()()
{
    float tunned_blurriness=0;
    while(1)
    {   //cout<<"1111111111111111111111111"<<endl;
        float current_blurriness=0;
        while (!OnvifDetailsQueue.empty())
        {
            OnvifDetailsQueue.pop();
            //sleep_for(chrono::milliseconds(20));
        }
        int count=0;
        while (count<30)
        {
            if (!OnvifDetailsQueue.empty())
            {
                current_blurriness+=OnvifDetailsQueue.front().blurriness;
                OnvifDetailsQueue.pop();
                count++;
            }
            this_thread::sleep_for(chrono::milliseconds(20));
        }
        current_blurriness=current_blurriness/(float)count;
        cout<<"current blurriness=: \t"<<current_blurriness<<"tunned blurriness=\t"<<tunned_blurriness<<endl;
        if ((tunned_blurriness==0)||(current_blurriness<0.9*tunned_blurriness))
        {
            cout<<"Exposure tunning"<<endl;
            tunned_blurriness= exposure_tuning(OnvifDetailsQueue); 
        }      
        else
        {
            cout<<"skip exposure tunning"<<endl;
        }
        this_thread::sleep_for(chrono::milliseconds(30000));
    }
}

