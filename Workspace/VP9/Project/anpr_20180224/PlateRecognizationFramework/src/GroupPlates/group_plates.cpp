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
#include <map>
#include "group_plates.h"
#include "r_distance.h"



using namespace cv;
using namespace std;

struct grouping_data
{
    vector <vehicle> Queue;
    int IDs;
    int numPlates;
    bool pushed_to_cnn;
};
map <int,int> location_grouping[MaxKindsPlate];
vector <grouping_data> grouping[MaxKindsPlate];

//const int Range_max_frame_each_plate = 30;
const float time_one_frame = 0.00001111;//(1s / 25 hinh) (tinh theo h)
bool firstpush=true;

result_group group_plates(vector <Rect> plates_square, vector <vehicle> plates_process, Mat framefull, Rect cropRect,int frameID,int mode)
{
    if (firstpush)
    {
        grouping_data firstdata;
        for (int i=0;i<MaxKindsPlate;i++)
            grouping[i].push_back(firstdata);
        firstpush=false;
    }
    ObjectUtils objects;
    //init vector Point2d: centers
    result_group result;
    vector<Point2d> centers;
    Point2d point;
    for (int i = 0; i < (int)plates_square.size(); i++)
    {
        point.x = plates_square[i].x+plates_square[i].width/2;
        point.y = plates_square[i].y+plates_square[i].height/2;
        
        centers.push_back(point);
    }
    
    //ToDO: Thanhnn tracking
    Struct_kalmans resultKM = group_plate_thanhnn(centers,frameID,mode);

    //ToDO: grouping + cal speed
    if (resultKM.centers_out.size()>0)
        for (int i=0; i< (int)resultKM.centers_out.size();i++)
        {
            //cout <<resultKM.IDs[i]<<"\t"<< location_grouping[resultKM.IDs[i]]<<"\t";
            if (location_grouping[mode][resultKM.IDs[i]]==0) //chua ton tai queue chua IDs nay -> tao queue moi chua data
            {
                bool pushed=false;
                plates_process[i].vehicleImage = framefull;
                //create grouping_data: data
                grouping_data data;
                data.Queue.push_back(plates_process[i]);
                data.IDs = resultKM.IDs[i];
                data.pushed_to_cnn = false;
                if (plates_process[i].plate_croped_detail.isplate)
                    data.numPlates = 1;
                else data.numPlates = 0;

                for (int j=1;j<(int)grouping[mode].size();j++)
                    if (grouping[mode][j].IDs ==0 && pushed==false)
                    {
                        grouping[mode][j] = data;
                        location_grouping[mode][resultKM.IDs[i]] = j;
                        pushed=true;
                    }
                if (pushed==false)
                {
                    grouping[mode].push_back(data);
                    location_grouping[mode][resultKM.IDs[i]] = grouping[mode].size()-1;
                }
                
                //result.speed_plates.push_back("0");
                result.speed_plates.push_back(objects.FloatToStr(resultKM.IDs[i]));
                result.rect_plates.push_back(resultKM.centers_out[i]);
            }
            else //da ton tai queue chua IDs nay, push data vao queue da co 
            {
                int location_push = location_grouping[mode][resultKM.IDs[i]];
                grouping[mode][location_push].Queue.push_back(plates_process[i]);
                if (plates_process[i].plate_croped_detail.isplate)
                   grouping[mode][location_push].numPlates++;
                //ToDO: Free Mat if numPlates > MaxPlateProcessCNN
                //if (grouping[mode][location_push].numPlates > MaxPlateProcessCNN)
                //    grouping[mode][location_push].Queue[grouping[mode][location_push].Queue.size()-1].plate_croped_detail.plate_extended.release();
                //ToDo: cal speed_plates -----------------------------------------------------
                float speed;
                if (grouping[mode][location_push].Queue.size() % MaxFrameForEstimateSpeed == 0)
                {
                    vehicle a = plates_process[i];
                    vehicle b = grouping[mode][location_push].Queue[0];
                    float distance_frame = a.frameID-b.frameID;
                    float distance = d_dist((float)a.locationX+cropRect.x+a.plate_croped_detail.cn1.x,(float)a.locationY+cropRect.y+a.plate_croped_detail.cn1.y,(float)b.locationX+cropRect.x+b.plate_croped_detail.cn1.x,(float)b.locationY+cropRect.y+b.plate_croped_detail.cn1.y);
                    // float distance = 1;
                    speed = distance/(float)1000/((float)distance_frame*(float)time_one_frame);
                }
                else 
                    speed = grouping[mode][location_push].Queue[grouping[mode][location_push].Queue.size()-2].speed;

                grouping[mode][location_push].Queue[grouping[mode][location_push].Queue.size()-1].speed = speed;
                if (speed>0 || speed<100)
                    speed = ceilf(speed);
                else speed=0;
                result.speed_plates.push_back(objects.FloatToStr(speed));
                // result.speed_plates.push_back(objects.FloatToStr(grouping[mode][location_push].IDs));
                result.rect_plates.push_back(resultKM.centers_out[i]);
            }
            
        }
    
    //ToDO: chose group for cnn + free group
    for (int i=1;i<(int)grouping[mode].size();i++)
    {
        //cout << grouping[i].numPlates <<endl;
        //chose group to cnn
        // if (grouping[mode][i].numPlates == MaxPlateProcessCNN && grouping[mode][i].pushed_to_cnn == false)
        // {
        //     //cout << grouping[mode][i].IDs <<endl;
        //     queue <vehicle> queuevehicle;
        //     for (int j=0;j<(int)grouping[mode][i].Queue.size();j++)
        //         if (grouping[mode][i].Queue[j].plate_croped_detail.isplate)
        //             queuevehicle.push(grouping[mode][i].Queue[j]);

        //     queuevehicle.front().vehicleImage = grouping[mode][i].Queue[0].vehicleImage.clone();
        //     queuevehicle.front().square_cascade = grouping[mode][i].Queue[0].square_cascade;

        //     result.queuepush.push_back(queuevehicle);
        //     grouping[mode][i].pushed_to_cnn = true;
        // }
        //free group
        if (frameID - grouping[mode][i].Queue[grouping[mode][i].Queue.size()-1].frameID >= Range_max_frame_each_plate)
        {
            //group chua push -> push to cnn
            if (grouping[mode][i].numPlates>0 && grouping[mode][i].pushed_to_cnn ==false)
            {
                //-----------push cnn frame blurriness thap nhat - dep nhat-------------------------------------
                int n = (int)grouping[mode][i].Queue.size();
                float a[n];
                int vt[n];
                for (int ii=0;ii<n;ii++)
                {
                    a[ii] = grouping[mode][i].Queue[ii].plate_croped_detail.blurriness;
                    vt[ii]=ii;
                }
                for (int ii=0;ii<n-2;ii++)
                    for (int jj=ii+1;jj<n-1;jj++)
                        if (a[ii]<a[jj])
                        {
                            float tg=a[ii];a[ii]=a[jj];a[jj]=tg;
                            int tg1=vt[ii];vt[ii]=vt[jj];vt[jj]=tg1;
                        }
                queue <vehicle> queuevehicle;
                for (int j=0;j<min(n,MaxPlateProcessCNN);j++)
                {
                    if (grouping[mode][i].Queue[vt[j]].plate_croped_detail.isplate)
                    {
                        //cout << grouping[mode][i].Queue[vt[j]].plate_croped_detail.blurriness<<"\t"<<vt[j] <<endl;
                        queuevehicle.push(grouping[mode][i].Queue[vt[j]]);
                    }
                }
                //-----------------------------------------------------------------------   
                // queue <vehicle> queuevehicle;
                // for (int j=0;j<(int)grouping[mode][i].Queue.size();j++)
                //     if (grouping[mode][i].Queue[j].plate_croped_detail.isplate)
                //         queuevehicle.push(grouping[mode][i].Queue[j]);

                queuevehicle.front().vehicleImage = grouping[mode][i].Queue[0].vehicleImage;
                queuevehicle.front().square_cascade = grouping[mode][i].Queue[0].square_cascade;

                result.queuepush.push_back(queuevehicle);
                grouping[mode][i].pushed_to_cnn = true;
            }   
            //clear group
            location_grouping[mode][grouping[mode][i].IDs]=0;
            grouping[mode][i].IDs = 0;
            grouping[mode][i].numPlates=0;
            grouping[mode][i].pushed_to_cnn = false;

            //for (int j=0;j<grouping[i].Queue.size();j++)
            {
                //Dear Bach, anh push ra 4 point o day nhe:
                /*Point lefttop_square, cornerplate1,cornerplate2,cornerplate3,cornerplate4;
                vehicle a = grouping[i].Queue[j];
                lefttop_square.x = a.locationX - a.plate_croped_detail.cn1.x + cropRect.x;
                lefttop_square.y = a.locationY - a.plate_croped_detail.cn1.y + cropRect.y;
                cornerplate1 = lefttop_square+a.plate_croped_detail.cn1;
                cornerplate2 = lefttop_square+a.plate_croped_detail.cn2;
                cornerplate3 = lefttop_square+a.plate_croped_detail.cn3;
                cornerplate4 = lefttop_square+a.plate_croped_detail.cn4;
                cout << a.frameID << "\t" << cornerplate1 <<"\t"<< cornerplate2 <<"\t"<< cornerplate3 <<"\t"<< cornerplate4 <<"\t"<<endl;*/

            }
            grouping[mode][i].Queue.clear();
            
        }
    }
    
    //ToDO: Push kết quả nội suy tới queue vẽ hình
    if (resultKM.assigned_traces.size()>0)
        for (int i=0; i< (int)resultKM.assigned_traces.size();i++)
        {
            float speed=0;
            for (int j=1;j<(int)grouping[mode].size();j++)
                if ((int)grouping[mode][j].IDs == (int)resultKM.track_id[i])
                {
                    speed = grouping[mode][j].Queue[grouping[mode][j].Queue.size()-1].speed;
                    break;
                }
            // result.speed_plates.push_back(objects.FloatToStr(resultKM.track_id[i]));
            if (speed>0 || speed<100)
                speed = ceilf(speed);
            else speed=0;
            result.speed_plates.push_back(objects.FloatToStr(speed));
            result.rect_plates.push_back(resultKM.assigned_traces[i]);
        }

    return result;
}
