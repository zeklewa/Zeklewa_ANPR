#include "PlateRecognizator.h"
#include "CascadeTrainingStrategy.h"
#include "opencv2/opencv.hpp"
#include "CascadeTrainingInputData.h"
#include "iostream"

using namespace std;
using namespace pr;

//#define DEBUG;
//void Non_Max_Sup(const vector<cv::Rect>& srcRects,vector<cv::Rect>& resRects,float thresh)
void Non_Max_Sup(const vector<pr::PlateRegion>& srcPlates, vector<pr::PlateRegion >& resPlates,float thresh)
{

        vector<cv::Rect> srcRects;
        vector<cv::Rect> resRects;
        
        resPlates.clear();
        
        for(int i=0; i<(int)srcPlates.size();i++)
        {
            cv::Rect item = srcPlates[i].region;
            srcRects.push_back(item);
        }
        
        const size_t size = srcRects.size();
	if (!size)
	{
		return;
	}

	vector<pair<double,size_t> > idxs;
	for (size_t i = 0; i < size; ++i)
	{
		idxs.push_back(pair<double,size_t>(srcRects[i].area(),i));
	}

	sort(idxs.begin(),idxs.end(),greater<pair<double,int> >());
	
	while(idxs.size()>0){
		const Rect& rect1 = srcRects[idxs[0].second];
		resRects.push_back(rect1);
		idxs.erase(idxs.begin());
		for (vector<pair<double,size_t> >::iterator pos = idxs.begin(); pos != idxs.end(); )
		{
			// grab the current rectangle
			const Rect& rect2 = srcRects[pos->second];

			float intArea = (rect1 & rect2).area();
			float unionArea = rect1.area() + rect2.area() - intArea;
			float overlap = intArea / unionArea;

			// if there is sufficient overlap, suppress the current bounding box
			if (overlap > thresh || intArea>0.8*rect2.area() || (rect1 & rect2) == rect2)
			{
				pos = idxs.erase(pos);
			}
			else pos++;
		}
	}
        // check and store the remain Rects
        for(int i = 0; i < (int)resRects.size();i++)
        {
            for(int j = 0; j < (int)srcPlates.size();j++)
            {
                if( resRects[i] == srcPlates[j].region)
                {
                    resPlates.push_back(srcPlates[j]);
                }
            }
        }
}
std::vector<pr::PlateRegion> PlateRecognizator::GetPlateRegions()
{
	// Get Plates Regions
	CascadeTrainingInputData *casData = new CascadeTrainingInputData();
	casData->img = img;
	PlateDetectorInputData *plateDetectorData = (PlateDetectorInputData *)casData;
	plateDetector->SetInputData(plateDetectorData);
	std::vector<PlateRegion> plates = plateDetector->GetPlateRegions();
	casData->img.release();
	img.release();
	
	//Anh Hà sửa code check trùng Rect ở đây. Cụ trể là các plates[i].region
	 // Non maximum suppression s to remove extra plate by Halm
	//        input: vector of plates
	std::vector<PlateRegion> inputplates = plates; 
	std::vector<PlateRegion> outputplates;// = input;
	Non_Max_Sup(inputplates,outputplates,0.2); // no overlap more than 20 % 
	//return plates;
	return outputplates;
}

void pr::PlateRecognizator::Init(std::string cascadeFileURL, cv::Size minSize, cv::Size maxSize, double scale, int neighbor)
{
	this->cascadeFileURL = cascadeFileURL;
	InitPlateDetector(minSize, maxSize, scale, neighbor);
}

void pr::PlateRecognizator::InitPlateDetector(cv::Size minSize, cv::Size maxSize, double scale, int neighbor)
{
	std::cout << "Call init plate detector" << std::endl;
	plateDetector = new PlateDetector();
	CascadeTrainingStrategy *casStrategy = new CascadeTrainingStrategy(cascadeFileURL);
	std::cout << minSize << std::endl;
	std::cout << maxSize << std::endl;
	casStrategy->SetMinSize(minSize);
	casStrategy->SetMaxSize(maxSize);
	casStrategy->SetScaleFactor(scale);
	casStrategy->SetMinNeighbor(neighbor);
	IPlateDetectStrategy *strategy = (IPlateDetectStrategy *)casStrategy;
	plateDetector->SetDetectStrategy(strategy);
}

void pr::PlateRecognizator::SetImg(cv::Mat &img)
{
	this->img = img.clone();
}

