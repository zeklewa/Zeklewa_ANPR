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
#include "tracking_thanhnn.h"


using namespace cv;
using namespace std;


struct group_plate
{
    queue <vehicle> vehicles;
    int groupedID;
    bool pushed;
    int pointNoiSuy;
};


queue <vehicle> es_speed[MaxKindsPlate][MaxVehicleInFrame];
int num_es_speed[MaxKindsPlate][MaxVehicleInFrame];


bool compare(vehicle a,vehicle b,int Range_LocationX, int Range_LocationY)
{
	if ((abs(a.locationX-b.locationX)<Range_LocationX)
			 && (abs(a.locationY-b.locationY)<Range_LocationY)
    )
		return true;
	else
		return false;
}

bool distance_ok(vehicle b,vehicle a)
{
    ObjectUtils objects;
    float range_frame = abs(a.frameID - b.frameID);
    float distance_2_point = sqrt((float)(pow(a.locationX-b.locationX,2)+pow(a.locationY-b.locationY,2)));
    float distance_line = objects.distance_point_to_line(b.linear_equations,a.locationX,a.locationY);
    if (distance_2_point<b.distance_to_nearest_plate*range_frame && range_frame!=0 && range_frame<Range_max_frame_each_plate)
        if (distance_line < range_plate_to_linear)
            return true;
    return false;
}

int IDs = 0;
group_plate group_plated[MaxKindsPlate][MaxVehicleInFrame];

Struct_kalmans group_plate_thanhnn(vector<Point2d> centers,int frameID, int mode)
{
    for (int i=0;i<MaxVehicleInFrame;i++)
        group_plated[mode][i].pushed=false;
    ObjectUtils objects;
    Struct_kalmans result;
    for (int k=0;k<(int)centers.size();k++)
    {
        vehicle a;
        a.locationX = centers[k].x;
        a.locationY = centers[k].y;
        
        a.frameID = frameID;
        
        bool push_queue_check=false;
        float distance_line[MaxVehicleInFrame], distance_point[MaxVehicleInFrame];
        for (int i=0;i<MaxVehicleInFrame;i++)
        {
            distance_line[i]=0;
            distance_point[i]=0;
        }
        
        //int min_range_frame=10000;
        float min_distance_point=10000;
        float min_distance_line=10000;
        int location_min_distance;
        
        for (int i=0;i<MaxVehicleInFrame;i++)
            if (!group_plated[mode][i].vehicles.empty() && group_plated[mode][i].pushed==false)
                if (distance_ok(group_plated[mode][i].vehicles.back(),a))
                {
                    vehicle b = group_plated[mode][i].vehicles.back();
                    if (group_plated[mode][i].vehicles.size()==1 || 
                        ((b.direction==1 && a.locationY<=b.locationY+Range_Minimize_of_the_same_plate) ||
                        (b.direction==0 && a.locationY>=b.locationY-Range_Minimize_of_the_same_plate)))
                    {
                        
                        float range_frame = abs(a.frameID - b.frameID);
                        distance_line[i] = objects.distance_point_to_line(b.linear_equations,a.locationX,a.locationY);
                        distance_point[i] = sqrt((float)(pow(a.locationX-b.locationX,2)+pow(a.locationY-b.locationY,2)))/range_frame;
                        if (min_distance_line>distance_line[i])
                        {
                            //min_range_frame = range_frame;
                            min_distance_line = distance_line[i];
                            min_distance_point = distance_point[i];
                            location_min_distance = i;
                            push_queue_check = true;
                        }               
                    }
                }
        //cout << "-----------------------------------------------" <<endl;
        if (push_queue_check)
        {
            a.distance_to_nearest_plate = min_distance_point+Range_Minimize_of_the_same_plate;
            int i = location_min_distance;
            vehicle b = group_plated[mode][i].vehicles.back();
            vehicle f = group_plated[mode][i].vehicles.front();
            a.linear_equations = objects.calculation_line_equa(a.locationX,a.locationY,b.locationX,b.locationY);
            if (group_plated[mode][i].vehicles.size()==1)
                if (a.locationY <= f.locationY)
                    a.direction = 1;
                else a.direction = 0;
            else
                a.direction = b.direction;
            a.distanceX_pointbefore = (a.locationX-b.locationX)/(a.frameID-b.frameID);
            a.distanceY_pointbefore = (a.locationY-b.locationY)/(a.frameID-b.frameID);
            group_plated[mode][i].vehicles.push(a);
            group_plated[mode][i].pushed = true;
            group_plated[mode][i].pointNoiSuy = 0;
            
            result.IDs.push_back(group_plated[mode][i].groupedID);
            result.centers_out.push_back(centers[k]);
        }
        else
        {
            for (int i=0;i<MaxVehicleInFrame;i++)
                if (group_plated[mode][i].vehicles.empty()){
                    IDs++;
                    a.direction = 2;
                    a.distance_to_nearest_plate = First_distance_to_nearest_plate;
                    a.linear_equations.a = -1; 
                    a.linear_equations.b = 0;
                    a.linear_equations.c = a.locationX;

                    group_plated[mode][i].vehicles.push(a);
                    group_plated[mode][i].groupedID = IDs;
                    group_plated[mode][i].pointNoiSuy = 0;
                    group_plated[mode][i].pushed=true;

                    result.IDs.push_back(IDs);
                    result.centers_out.push_back(centers[k]);

                    break;
                }
        }

        
    }
    //ToDO: free queue
    for (int i=0;i<MaxVehicleInFrame;i++)
        if (!group_plated[mode][i].vehicles.empty())
            if (frameID - group_plated[mode][i].vehicles.back().frameID > Range_max_frame_each_plate)
                while (!group_plated[mode][i].vehicles.empty())
                    group_plated[mode][i].vehicles.pop();
    //ToDO: Nội suy point misss
    for (int i=0;i<MaxVehicleInFrame;i++)
    if (group_plated[mode][i].vehicles.size()>1 && group_plated[mode][i].pushed == false)
    {
        vehicle b = group_plated[mode][i].vehicles.back();
        if (group_plated[mode][i].pointNoiSuy < num_frame_noisuy)
        {
            //cout << "Noi suy: " << group_plated[mode][i].groupedID << endl;
            Point2d p;
            
            p.x = b.locationX + b.distanceX_pointbefore*(frameID-b.frameID);
            p.y = b.locationY + b.distanceY_pointbefore*(frameID-b.frameID);

            result.assigned_traces.push_back(p);
            result.track_id.push_back(group_plated[mode][i].groupedID);
            group_plated[mode][i].pointNoiSuy++;
            /*vehicle x = b;
            x.locationX = p.x;
            x.locationY = p.y;
            x.locationX_pointbefore = b.locationX;
            x.locationY_pointbefore = b.locationY;
            x.frameID = frameID;
            group_plated[mode][i].vehicles.push(x);*/
        }
    }

    return result;
}