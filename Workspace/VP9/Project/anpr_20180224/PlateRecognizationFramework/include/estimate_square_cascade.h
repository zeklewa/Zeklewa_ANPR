#ifndef ESTIMATE_SQUARE_CASCADE
#define ESTIMATE_SQUARE_CASCADE

#include <iostream>
#include <vector>
#include <chrono>
#include <mutex>
#include "PlateRecognizator.h"

using namespace cv;
using namespace std;

struct cascade_input
{
    int carMin_x;
    int carMin_y;
    int carMax_x;
    int carMax_y;
    float detectScale;
};



cascade_input estimate_square_cascade(queue<long> car_estimate_square_cascade, int mode);


#endif