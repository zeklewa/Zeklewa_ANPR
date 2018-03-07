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

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "PlateRecognizator.h"
#include "PlateReader.h"
#include "group_plates.h"
#include "estimate_square_cascade.h"
#include "Crop.h"
#include "bckgrnd_subs.h"
#include "publicValue.h"



using namespace pr;
using namespace std;
using namespace cv;

long num_plate_detected[MaxKindsPlate];

queue <long> car_estimate_square_cascade;
int FrameIgnored=atoi(IOData::GetCongfigData("FrameIgnored:").c_str());

float Neighbor[2];
PlateReader::PlateReader(cv::Rect cropRect,
                         std::queue<FrameData> &frameQueue_, std::mutex &frameMutex_,
                         QueuePlates &plateQueue_,
                         std::queue<image_details> &OnvifDetailsQueue_,
                         std::queue<PlateData> &plateQueue_showFrame_,std::mutex &plateQueue_showFrameMutex_)
    : frameQueue(frameQueue_), frameMutex(frameMutex_),
      plateQueue(plateQueue_),
      OnvifDetailsQueue(OnvifDetailsQueue_),
      plateQueue_showFrame(plateQueue_showFrame_),
      plateQueue_showFrameMutex(plateQueue_showFrameMutex_)

{
    //ObjectUtils objects;
    this->cropRect = cropRect;
    std::string queuesize = pr::IOData::GetCongfigData("frameQueueSize:");
    this->QueueSize = atoi(queuesize.c_str());
    //Square = 0
    //Long = 1
    int MinWidth[2], MinHeight[2], MaxWidth[2], MaxHeight[2];
    float DetectScale[2];

    MinWidth[0] = atoi(IOData::GetCongfigData("SquareMinWidth:").c_str());
    MinHeight[0] = atoi(IOData::GetCongfigData("SquareMinHeight:").c_str());
    MaxWidth[0] = atoi(IOData::GetCongfigData("SquareMaxWidth:").c_str());
    MaxHeight[0] = atoi(IOData::GetCongfigData("SquareMaxHeight:").c_str());

    MinWidth[1] = atoi(IOData::GetCongfigData("LongMinWidth:").c_str());
    MinHeight[1] = atoi(IOData::GetCongfigData("LongMinHeight:").c_str());
    MaxWidth[1] = atoi(IOData::GetCongfigData("LongMaxWidth:").c_str());
    MaxHeight[1] = atoi(IOData::GetCongfigData("LongMaxHeight:").c_str());

    DetectScale[0] = atof(IOData::GetCongfigData("SquareDetectScale:").c_str());
    DetectScale[1] = atof(IOData::GetCongfigData("LongDetectScale:").c_str());
    Neighbor[0] = atof(IOData::GetCongfigData("SquareNeighbor:").c_str());
    Neighbor[1] = atof(IOData::GetCongfigData("LongNeighbor:").c_str());
    for (int mode=0;mode<MaxKindsPlate;mode++)
    {

        recognizator_vehicle[mode].Init(IOData::GetCongfigData(objects.kindcascade(mode)),
                cv::Size(MinWidth[objects.kindsizecascade(mode)], MinHeight[objects.kindsizecascade(mode)]), 
                cv::Size(MaxWidth[objects.kindsizecascade(mode)], MaxHeight[objects.kindsizecascade(mode)]), 
                DetectScale[objects.kindsizecascade(mode)], Neighbor[objects.kindsizecascade(mode)]);

    }
    for (int i=0;i<MaxKindsPlate;i++)
        num_plate_detected[i]=0;
}
float width = atof(IOData::GetCongfigData("video_width:").c_str());
float height = atof(IOData::GetCongfigData("video_height:").c_str());
void PlateReader::operator()()
{
    int nframe = 0,fframe=0;
    auto start = CLOCK_NOW();
    while (true)
    {
        this_thread::sleep_for(chrono::milliseconds(1));
        frameMutex.lock();
        if (frameQueue.empty()) {
            frameMutex.unlock();
            continue;
        }
        
        if ((int)frameQueue.size() > this->QueueSize)
        {
             cout << "full cascade!!!" <<endl;
            frameQueue.pop();
            frameMutex.unlock();
            continue;
        }
        FrameData thisFrame = frameQueue.front();
        frameQueue.pop();
        frameMutex.unlock();

        // vector<cv::Rect> detected = Object_detection(thisFrame.frame.clone());
         vector<cv::Rect> detected;
         detected.push_back(Rect(0,0,cropRect.width,cropRect.height));
        
        // //------------Vẽ hình vuông lên frame-------------------------
        if (showVideo)
            for (int i = 0; i < (int)detected.size(); i++)
            {
                float rate_crop_width = (float)thisFrame.framecolor.cols/(float)width;
                float rate_crop_height = (float)thisFrame.framecolor.rows/(float)height;
                Rect RectReal = detected[i];
                RectReal.x = (int)((float)RectReal.x*rate_crop_width);
                RectReal.y = (int)((float)RectReal.y*rate_crop_height);
                RectReal.width = (int)((float)RectReal.width*rate_crop_width);
                RectReal.height = (int)((float)RectReal.height*rate_crop_height);
                RectReal.x += cropRect.x*rate_crop_width;
                RectReal.y += cropRect.y*rate_crop_height;
                cv::rectangle(thisFrame.framecolor, RectReal, cv::Scalar(255, 0,0), 2, 8, 0);       
            }
        // -------------------------------------------------------------

        cv::Mat frameROI = thisFrame.frame;//(cropRect);
        
        std::vector<cv::Rect> prects;
        for (int mode=0; mode<MaxKindsPlate; mode++)//Duyet tat ca kieu bien so xe
        {
                std::vector<pr::PlateRegion> plates_square[MaxKindsPlate];
                std::vector<string> vehicle_speed[MaxKindsPlate];
                for (int i = 0; i < (int)detected.size(); i++)
                {
                    Mat frameCroped = frameROI(detected[i]);
                    recognizator_vehicle[mode].SetImg(frameCroped);
                    vector<PlateRegion> plate_square = recognizator_vehicle[mode].GetPlateRegions();
                    for (int j=0;j<(int)plate_square.size();j++)
                    {
                        plate_square[j].region.x+=detected[i].x;
                        plate_square[j].region.y+=detected[i].y;
                        plates_square[mode].push_back(plate_square[j]);
                    }
                    plate_square.clear();
                    frameCroped.release();

                }

                // // ToDo: draw square cascade into frame
                if (showVideo)
                    for (int i = 0; i < (int)plates_square[mode].size(); i++)
                    {
                        float rate_crop_width = (float)thisFrame.framecolor.cols/(float)width;
                        float rate_crop_height = (float)thisFrame.framecolor.rows/(float)height;
                        Rect RectReal = plates_square[mode][i].region;
                        RectReal.x = (int)((float)RectReal.x*rate_crop_width);
                        RectReal.y = (int)((float)RectReal.y*rate_crop_height);
                        RectReal.width = (int)((float)RectReal.width*rate_crop_width);
                        RectReal.height = (int)((float)RectReal.height*rate_crop_height);
                        RectReal.x += cropRect.x*rate_crop_width;
                        RectReal.y += cropRect.y*rate_crop_height;
                        cv::rectangle(thisFrame.framecolor, RectReal, cv::Scalar(0, 255, 0), 2, 8, 0);
                    }

                //ToDO: init vector Plates: plates_process
                vector <vehicle> plates_process;
                int isplated[plates_square[mode].size()+1];
                for (int i = 0; i < (int)plates_square[mode].size(); i++)
                {
                    cv::Rect RegionPlus = cv::Rect(
                        std::max(plates_square[mode][i].region.x - 15, 0),
                        std::max(plates_square[mode][i].region.y - 15, 0),
                        std::min(plates_square[mode][i].region.width + 30, frameROI.cols - plates_square[mode][i].region.x + 15-1),
                        std::min(plates_square[mode][i].region.height + 30, frameROI.rows - plates_square[mode][i].region.y + 15-1));

                    vehicle a;
                    a.locationX = plates_square[mode][i].region.x;
                    a.locationY = plates_square[mode][i].region.y;    
                    a.time = objects.convert_time(thisFrame.frameTime);
                    a.frameID = thisFrame.frameID;
                    a.plate = frameROI(RegionPlus);
                    a.square_cascade = plates_square[mode][i].region;
                    a.CurrentDateTime = thisFrame.frameTime;
                    a.plate_croped_detail = crop_plate_corner(frameROI(RegionPlus),mode,ratio_crop[mode]); //crop plate
                    if (a.plate_croped_detail.isplate)
                    {
                        image_details b;
                        b.blurriness = a.plate_croped_detail.blurriness;
                        b.brightness=  a.plate_croped_detail.brightness;
                        OnvifDetailsQueue.push(b);
                        plates_process.push_back(a);
                        prects.push_back(a.square_cascade); //push to prects (to cms)
                        isplated[i]=1;
                    }
                    else isplated[i]=0;
                }

                //ToDO: Estimate square cascade; ReInit recognizator_vehicle cascade
                for (int i = 0; i < (int)plates_square[mode].size(); i++)
                    if (isplated[i])
                    {
                        num_plate_detected[mode]++;
                        // cout <<"mode: "<<mode<< "\tnum: " << num_plate_detected[mode]<<endl;
                        if (num_plate_detected[mode]%50000 <= num_plate_estimate_square_cascade[mode]) //Each 50.000 plate recalculate plate size for cascade
                        {
                            long multiply = plates_square[mode][i].region.width*plates_square[mode][i].region.height;
                            car_estimate_square_cascade.push(multiply);
                            if (num_plate_detected[mode]%50000 == num_plate_estimate_square_cascade[mode])
                            {
                                //ToDO: Estimate square cascade
                                cascade_input cascade_square_result = estimate_square_cascade(car_estimate_square_cascade,mode);
                                //ToDO: ReInit recognizator_vehicle cascade
                                if (mode ==0 || mode ==2)
                                {
                                    
                                    recognizator_vehicle[0].Init(IOData::GetCongfigData(objects.kindcascade(0)),
                                    cv::Size(cascade_square_result.carMin_x, cascade_square_result.carMin_y), 
                                    cv::Size(cascade_square_result.carMax_x, cascade_square_result.carMax_y), 
                                    cascade_square_result.detectScale, Neighbor[objects.kindsizecascade(0)]);
                                    if (2<=MaxKindsPlate)
                                    {
                                        recognizator_vehicle[2].Init(IOData::GetCongfigData(objects.kindcascade(2)),
                                        cv::Size(cascade_square_result.carMin_x, cascade_square_result.carMin_y), 
                                        cv::Size(cascade_square_result.carMax_x, cascade_square_result.carMax_y), 
                                        cascade_square_result.detectScale, Neighbor[objects.kindsizecascade(2)]);
                                    }
                                }
                                else
                                {
                                    recognizator_vehicle[1].Init(IOData::GetCongfigData(objects.kindcascade(1)),
                                    cv::Size(cascade_square_result.carMin_x, cascade_square_result.carMin_y), 
                                    cv::Size(cascade_square_result.carMax_x, cascade_square_result.carMax_y), 
                                    cascade_square_result.detectScale, Neighbor[objects.kindsizecascade(1)]);
                                    if (3<=MaxKindsPlate)
                                    {
                                        recognizator_vehicle[3].Init(IOData::GetCongfigData(objects.kindcascade(3)),
                                        cv::Size(cascade_square_result.carMin_x, cascade_square_result.carMin_y), 
                                        cv::Size(cascade_square_result.carMax_x, cascade_square_result.carMax_y), 
                                        cascade_square_result.detectScale, Neighbor[objects.kindsizecascade(3)]);
                                    }
                                }
                                //ToDO: Free queue
                                while (!car_estimate_square_cascade.empty())
                                    car_estimate_square_cascade.pop();
                            }
                        }
                    }

                //ToDO: init vector plates_square
                vector <Rect> plates_square_region;
                for (int i = 0; i < (int)plates_square[mode].size(); i++)
                    if (isplated[i])
                        plates_square_region.push_back(plates_square[mode][i].region);
                
                
                //ToDO: kalman tracking & group_plates
                //input: vector rect: plates_square[mode]; vector vehicle: plates_process; frameFull
                //output: vector Rect + vector Speed; group result: input for cnn
                result_group result = group_plates(plates_square_region, plates_process, thisFrame.framecolor, cropRect,thisFrame.frameID,mode);
                //ToDO: draw speed on frame
                if (showVideo)
                    for (int i=0; i< (int)result.speed_plates.size();i++)
                        if (result.speed_plates[i]!="0")
                            {
                                float rate_crop_width = (float)thisFrame.framecolor.cols/(float)width;
                                float rate_crop_height = (float)thisFrame.framecolor.rows/(float)height;
                                Rect RectReal;
                                RectReal.x = result.rect_plates[i].x;
                                RectReal.y = result.rect_plates[i].y;
                                RectReal.x = (int)((float)RectReal.x*rate_crop_width);
                                RectReal.y = (int)((float)RectReal.y*rate_crop_height);
                                RectReal.x += cropRect.x*rate_crop_width;
                                RectReal.y += cropRect.y*rate_crop_height;
                                putText(thisFrame.framecolor, result.speed_plates[i] +"km/h", cvPoint(RectReal.x,RectReal.y-50), FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(0, 0, 100), 25, CV_AA);
                                putText(thisFrame.framecolor, result.speed_plates[i] +"km/h", cvPoint(RectReal.x,RectReal.y-50), FONT_HERSHEY_COMPLEX_SMALL, 2, cvScalar(255, 255, 0), 2, CV_AA);
                            }
                //ToDO: Push data to CNN
                for (int i=0;i<(int)result.queuepush.size();i++)
                    plateQueue.platequeue[mode].push(result.queuepush[i]);
                plates_square[mode].clear();
        }
        //Push prects to CMS
        // float rate_crop_width = (float)thisFrame.framecolor.cols/(float)width;
        // float rate_crop_height = (float)thisFrame.framecolor.rows/(float)height;
        // for (int i=0;i<prects.size();i++)
        // {    
        //     Rect RectReal;
        //     RectReal = prects[i];
        //     RectReal.x = (int)((float)RectReal.x*rate_crop_width);
        //     RectReal.y = (int)((float)RectReal.y*rate_crop_height);
        //     RectReal.x += cropRect.x*rate_crop_width;
        //     RectReal.y += cropRect.y*rate_crop_height;        
        //     RectReal.width = RectReal.width*rate_crop_width;
        //     RectReal.height = RectReal.height*rate_crop_height;
        //     prects[i]=RectReal;
        // }
        // CMSInstance.push(prects, thisFrame.timestamp);


        //ToDO: printf Framefps
        nframe++;
		auto end = CLOCK_NOW();
		ElapsedTime elapsed = end - start;
		if (nframe - fframe >= 100)
		{
            cout << " process  fps=" << (nframe - fframe) / elapsed.count() << endl;
            start = end;
            fframe = nframe;    
        }

        // Push square plate to queue for video realtime
        if (showVideo)
        {
            PlateData data;
            data.frame = thisFrame.framecolor;
            plateQueue_showFrame.push(data);
        }
    }
}

