#include <iostream>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <fstream>
#include <ctime>
#include <queue>
#include <thread>
#include <mutex>
#include <exception>

#include "VideoReader.h"
#include "PlateReader.h"
#include "PlateRecognizator.h"
#include "PlateRecognitionAction.h"
#include "Onvif_controller.h"
#include "Onvif.h"
#include "xmlParser.h"
#include "publicValue.h"

using namespace pr;
using namespace cv;
using namespace std;

queue<FrameData> frameQueue;//Frames from Video Streaming
mutex frameQueueMutex;

//Mode: kinds plate
//0: While square
//1: While long
//2: BlueRed square
//3: BlueRed long
cv::Rect cropRect;

QueuePlates plateQueue;
queue<PlateData> plateQueue_showFrame;
mutex plateQueue_showFrameMutex;
queue<image_details> OnvifDetailsQueue;

pr::ObjectUtils objects;
int FrameIgnored2=atoi(IOData::GetCongfigData("FrameIgnored:").c_str());

void showFrame()
{
	int nframe = 0;
	auto start = CLOCK_NOW();
	int fps = atoi(IOData::GetCongfigData("fps_video_reading:").c_str())+1;
	int delay = 1000/fps*FrameIgnored2;
	while (true)
	{
		this_thread::sleep_for(chrono::milliseconds(1));
		auto startime = CLOCK_NOW();
		if (plateQueue_showFrame.empty())
			continue;
		plateQueue_showFrameMutex.lock();
		PlateData data = plateQueue_showFrame.front();
		plateQueue_showFrame.pop();
		plateQueue_showFrameMutex.unlock();

		cv::Mat frame = data.frame.clone();
		cv::resize(frame, frame, cv::Size(), 0.4, 0.4);
		cv::imshow("Plate1", frame);
		cv::waitKey(1);
		nframe++;
		auto end = CLOCK_NOW();
		ElapsedTime elapsed = end - start;
		if (nframe % 100 == 0)
		{
			//cout << " display fps=" << nframe / elapsed.count() << endl;
		}
		while (1)
		{
			auto endtime = CLOCK_NOW();
			ElapsedTime elapsedtime = endtime - startime;
			if (elapsedtime.count()*1000 < delay)
				this_thread::sleep_for(chrono::milliseconds(1));
			else break;
		}
	}
}
int killthread=1;
int kn12=1;
void writelog()
{
	string link = "../logrunning.txt";
	while(1)
	{
		string detial = objects.getCurrentDateTime_pushSQL()+"\t is running!";
		objects.wtfile_demo(link,detial);
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}

int main()
{
	int cropX = atoi(IOData::GetCongfigData("cropX:").c_str());
	int cropY = atoi(IOData::GetCongfigData("cropY:").c_str());
	int cropWidth = atoi(IOData::GetCongfigData("cropWidth:").c_str());
	int cropHeight = atoi(IOData::GetCongfigData("cropHeight:").c_str());
	int showVideo_readfile = atoi(IOData::GetCongfigData("showVideo:").c_str());
	if (showVideo_readfile==1)
		showVideo=true;
	else showVideo=false;

	cropRect = cv::Rect(cropX, cropY, cropWidth, cropHeight);

	//-------------------Huu Ton Test-----------------------------------------
	//string IPCameraAddress = IOData::GetCongfigData("IPCameraAddress:").c_str();
	//OnvifController onvifController(IPCameraAddress+":2000");
	// //OnvifController onvifController("10.12.11.149:8999"); // Cam a Tuc
	//int current_exposure=0;
	//int count=0;
	//onvifController.setExposureTime(1000,"VideoSource0");
	//------------------------------------------------------------------------

	VideoReader VideoReader(IOData::GetLinkURL(),
							atoi(IOData::GetCongfigData("video_width:").c_str()),
							atoi(IOData::GetCongfigData("video_height:").c_str()),
							cropRect,
							frameQueue);

	PlateReader plateReader(
		cropRect,
		frameQueue,
		frameQueueMutex,
		plateQueue,
		OnvifDetailsQueue,
		plateQueue_showFrame,
		plateQueue_showFrameMutex);
	
	PlateRecognitionAction PlateRecognitionAction(
			cropRect,
			plateQueue);
	
	//Onvif_controller Onvif_controller(
	//		OnvifDetailsQueue);

	std::vector<std::thread> ths;
	ths.push_back(std::thread(VideoReader));
	ths.push_back(std::thread(plateReader));
	ths.push_back(std::thread(PlateRecognitionAction));
	if (showVideo)
		ths.push_back(std::thread(showFrame));
	// ths.push_back(std::thread(Onvif_controller));
	//ths.push_back(std::thread(writelog));

	for (auto &t : ths)
		t.join();
	
}