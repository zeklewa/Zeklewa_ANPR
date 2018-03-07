#ifndef TRACKING_THANHNN
#define TRACKING_THANHNN

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include "PlateRecognizator.h"
#include "ObjectUtils.h"
#include "trackingkalman.h"

using namespace cv;
using namespace std;
const int MaxVehicleInFrame = 50;
const float First_distance_to_nearest_plate = 100;//khoang cach den point truoc
const int Range_max_frame_each_plate = 20;//khoang cach toi da miss frame
const int Range_Minimize_of_the_same_plate = 30;//khoang cach coi nhu 1 plate
const float range_plate_to_linear = 200;//khoang cach toi da giua point va line
const float num_frame_noisuy = 5;

    Struct_kalmans group_plate_thanhnn(vector<Point2d> centers,int frameID, int mode);
#endif
