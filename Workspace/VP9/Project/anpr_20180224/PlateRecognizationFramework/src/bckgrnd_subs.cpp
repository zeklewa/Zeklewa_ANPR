// #include <vector>
// #include <fstream>
// #include <iostream>
// #include <math.h>
// #include "common_function.h"

// #include "opencv2/video/background_segm.hpp"
// #include "opencv2/highgui/highgui.hpp"
// #include "opencv2/imgproc/imgproc.hpp"
// #include "opencv2/core/core.hpp"

// using namespace cv;
// using namespace std;
// //Mat img, foregroundMask, backgroundImage, foregroundImg;
// bool kn=false;
// Ptr<BackgroundSubtractor> bg_model;
// const float resize_value=0.4;
// int history = 1;
// float varThreshold = 16;
// bool bShadowDetection = true;
// vector<Rect> Object_detection(Mat framein)
// {
//     int rows=framein.rows,cols=framein.cols;
//     if (kn==false)
//     {
//         //opencv 3.2
//         bg_model = createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();
//         //opencv 2.4.9
//         // bg_model = new BackgroundSubtractorMOG2(history, varThreshold, bShadowDetection); //MOG2 approach
//         // bg_model->setInt("nmixtures", 3);
//         // bg_model->setDouble("fTau", 0.5);
//         kn=true;
//     }
//     cv::resize(framein, framein, cv::Size(), resize_value, resize_value);

//     //opencv 2.4.9
//     // bg_model->operator()(framein, framein);
//     //opencv 3.2
//     bg_model->apply(framein, framein, true ? -1 : 0);
//     threshold(framein, framein, 10,255,THRESH_BINARY);

//     int erosion_elem = 2;
//     int erosion_size = 2;
//     int dilation_elem = 0;
//     int dilation_size = 6;
    
//     int erosion_type;
//     if( erosion_elem == 0 ){ erosion_type = MORPH_RECT; }
//     else if( erosion_elem == 1 ){ erosion_type = MORPH_CROSS; }
//     else if( erosion_elem == 2) { erosion_type = MORPH_ELLIPSE; }

//     Mat element_eros = getStructuringElement( erosion_type,
//                                          Size( 2*erosion_size + 1, 2*erosion_size+1 ),
//                                          Point( erosion_size, erosion_size ) );
  
//     int dilation_type;
//     if( dilation_elem == 0 ){ dilation_type = MORPH_RECT; }
//     else if( dilation_elem == 1 ){ dilation_type = MORPH_CROSS; }
//     else if( dilation_elem == 2) { dilation_type = MORPH_ELLIPSE; }

//     Mat element_dile = getStructuringElement( dilation_type,
//                                        Size( 2*dilation_size + 1, 2*dilation_size+1 ),
//                                        Point( dilation_size, dilation_size ) );

//     bwareaopen(framein,500);
//     //erode( framein, framein, element_eros );
//     dilate( framein, framein, element_dile );   
//     //erode( framein, framein, element_eros );
//     // imshow("foreground mask", framein);
//     // waitKey(1);


//     vector<vector<Point> > contours;
// 	vector<Vec4i> hierarchy;
//     vector<Rect> rect_contours;
//     long long segment_size;
//     segment_size=2000*resize_value;
//     findContours(framein, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
//     for (int i = 0; i < contours.size(); i++)
//     {
//         // Calculate contour area
//         double area = cv::contourArea(contours[i]);
//         //cout<<i<<"\t size="<<area<<endl;

//         // Remove small objects by drawing the contour with black color
//         if (area > 0 && area > segment_size)
//         {
//             int max_x=0;
//             int max_y=0;
//             int min_x=framein.size().width;
//             int min_y=framein.size().height;
//             for (int j=0;j<contours[i].size();j++)
//             {

//                 if (contours[i][j].x>max_x)
//                 {
//                     max_x=contours[i][j].x;
//                 }
//                 if(contours[i][j].x<min_x)
//                 {
//                     min_x=contours[i][j].x;
//                 }
//                 if(contours[i][j].y>max_y)
//                 {
//                     max_y=contours[i][j].y;
//                 }
//                 if (contours[i][j].y<min_y)
//                 {
//                     min_y=contours[i][j].y;
//                 }
//             }
//             Rect rect_c;
//             // rect_c.x=min_x;
//             // rect_c.y=min_y;
//             // rect_c.width=max_x-min_x;
//             // rect_c.height=max_y-min_y;
//             rect_c.x=max(0,(int)((float)min_x/(float)resize_value));
//             rect_c.y=max(0,(int)((float)min_y/(float)resize_value));
//             rect_c.width=min(cols,(int)(((float)max_x-(float)min_x)/(float)resize_value));
//             rect_c.height=min(rows,(int)(((float)max_y-(float)min_y)/(float)resize_value));
//             // cout << rect_c.x<<"\t"<< rect_c.y<<"\t"<< rect_c.width<<"\t"<< rect_c.height<<"\t"<<endl;
//             rect_contours.push_back(rect_c);
//         }
//     }
//     double sum =0;
//     float ratio =0;
//     sum = (float)cv::sum( framein )[0]/255;
//     ratio = sum /(framein.cols*framein.rows);
//     bool detected = false;
    
//     if(ratio >= 0.3 && ratio <=0.7)
//     {
//         detected = true;
//     }

//     contours.clear();
//     hierarchy.clear();
//     //  rect_contours.clear();
//     //vector<Rect> rect_contours;
//     return rect_contours;
// }
