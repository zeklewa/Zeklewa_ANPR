#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>
#include <ctime>
#include <queue>
#include <thread>
#include <mutex>
#include <exception>
#include <ctime>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "Onvif_controller.h"
#include "Onvif.h"

//using namespace pr;
using namespace std;
using namespace cv;
struct exposure_status
{
int exposure_time;
float blurriness;	
};

int exposure_value[17]={12,25,30,35,50,100,150,200,250,300,400,500,1000,2000,4000,6000,8000};
string IPCameraAddress = IOData::GetCongfigData("IPCameraAddress:").c_str();
OnvifController onvifController(IPCameraAddress+":2000");
//OnvifController onvifController("10.12.11.149:2000"); // Cam a Tuc
int check_range=2;
time_t t = time(0);   // get time now
struct tm * now = localtime( & t );
mutex OnvifDetailsQueueMutex;

Onvif_controller::Onvif_controller(
    queue<image_details> &queue_)
    : OnvifDetailsQueue(queue_)
{
}

exposure_status exposure_tuning(queue<image_details> &OnvifDetailsQueue,int current_exposure_time)
{
        exposure_status es;
        int down_bound=0;
    int up_bound=0;
    int current_exposure_level=0;
    int get_current_et;
    get_current_et=onvifController.getExposureTime();
    if (get_current_et!=-1)
    {
        for (int i=0;i<17;i++)
        {
            if (exposure_value[i]==get_current_et)
            {
                current_exposure_level=i;
                break;
            }
        }
        down_bound=max(0,current_exposure_level-check_range);
        up_bound=min(16,current_exposure_level+check_range);

    }
    else
    {
        if (current_exposure_time==0)
        {
            down_bound=5;
            up_bound=16;
        }
        else
        {
            for (int i=0;i<17;i++)
            {
                if (exposure_value[i]==current_exposure_time)
                {
                    current_exposure_level=i;
                    break;
                }
            }
            down_bound=max(5,current_exposure_level-check_range);
            up_bound=min(16,current_exposure_level+check_range);
        }
        //cout << "WARNING: Get_exposure_time. Cannot get the respond from camera "<< endl;

    }

    //int current_exposure=onvifController.getExposureTime();
    //int current_exposure=1000;

    cout<<"downbound=\t"<<down_bound<<"\t Upbound \t"<<up_bound<<endl;
    float max_blurriness=0;
    float max_brightness=0;
   int max_index=0;
   for (int i=down_bound;i<=up_bound;i++)
   {
       onvifController.setExposureTime(exposure_value[i], "VideoSource0");
       sleep(50);
       OnvifDetailsQueueMutex.lock();
       while (!OnvifDetailsQueue.empty())
       {
           OnvifDetailsQueue.pop();
           //sleep_for(chrono::milliseconds(20));
       }
       OnvifDetailsQueueMutex.unlock();
       float blurriness=0;
       int count=0;
       int check=0;
       float brightness=0;
       while ((count<30)&&(check<100))
       {
           if (!OnvifDetailsQueue.empty())
           {
               blurriness+=OnvifDetailsQueue.front().blurriness;
               brightness+=OnvifDetailsQueue.front().brightness;
               OnvifDetailsQueue.pop();
               count++;
               check=0;
               cout<<"i=:\t"<<i<<"\t count=:\t"<<count<<endl;
           }
           else
           {
               check++;
               cout<<"i=:\t"<<i<<"\t check=:\t"<<check<<endl;
           }
           this_thread::sleep_for(chrono::milliseconds(50));
       }
       if (check<99)
       {
          // cout<<"blurriness=\t"<<blurriness<<endl;
           blurriness=blurriness/(float)count;
           brightness=(float) brightness/(float) count;
           if (blurriness>max_blurriness)
           {
               max_blurriness=blurriness;
               max_brightness=brightness;                    
               max_index=i;
           }    
       }        

   }
   cout<<"max_blurriness=:\t"<<max_blurriness<<"max_index=:\t"<<max_index<<"max_brightness=:\t"<<max_brightness<<endl;
   es.blurriness=max_blurriness;
   es.exposure_time=exposure_value[max_index];
   if (max_blurriness>0)
   {
       onvifController.setExposureTime(exposure_value[max_index], "VideoSource0");
       onvifController.setBrightness((220-max_brightness),"VideoSource0");
       cout<<"new exposure time is\t"<< exposure_value[max_index]<<"new brightness is:\t"<<(220-max_brightness)<<endl;
   }
   else
   {
       int current_hour=now->tm_hour;    
       cout<<"do not detect enough plate for tunning, adjustment is made base on the current hour:\t"<<current_hour<<endl;
       if (current_hour>6 && current_hour<17)
       {
            onvifController.setExposureTime(2000, "VideoSource0");
            onvifController.setBrightness(110,"VideoSource0");
       }
       else
       {
            onvifController.setExposureTime(300, "VideoSource0");
            onvifController.setBrightness(170,"VideoSource0");
       }
   }
   
   return (es);
}


void Onvif_controller::operator()()
{
    exposure_status current_state;
    current_state.blurriness=0;
    current_state.exposure_time=0;
    int skip_check=0;
    int current_hour=now->tm_hour; 
    cout<<"current hour is:\t"<<current_hour<<endl;
    
    while(1)
    {  // cout<<"1111111111111111111111111"<<endl;
        float current_blurriness=0;

        while (!OnvifDetailsQueue.empty())
        {
            OnvifDetailsQueue.pop();
            //sleep_for(chrono::milliseconds(20));
        }
        int count=0;
        int check=0;
        while ((count<30)&&(check<100))
        {
            if (!OnvifDetailsQueue.empty())
            {
                current_blurriness+=OnvifDetailsQueue.front().blurriness;
                OnvifDetailsQueue.pop();
                count++;
            }
            else
            {
                check++;
            }
            this_thread::sleep_for(chrono::milliseconds(50));
        }
        if (check<99)
        {
            current_blurriness=current_blurriness/(float)count;
        }
        else
        {
            current_blurriness=0;
        }
        cout<<"current blurriness=: \t"<<current_blurriness<<endl;
        if ((current_state.exposure_time==0)||(current_blurriness<0.8*current_state.blurriness)||(skip_check==10)||(current_state.blurriness==0))     
        {
            cout<<"Exposure tunning"<<endl;
            current_state= exposure_tuning(OnvifDetailsQueue,current_state.exposure_time); 
        }      
        else
        {
            cout<<"skip exposure tunning"<<endl;
            skip_check++;
            if (skip_check>10)
            {
                skip_check=0;
            }
            
        }
        this_thread::sleep_for(chrono::milliseconds(30000));
    }
}
