#pragma once
#include "util.h"
#include "cvBufferCapture.h"

typedef long timestampType;
struct frame_packet {
  int index;
  cv::Mat frame;
  timestampType timestamp;
};

void setRunningStt(bool stt);
void initializeDecoder();
void decodeRTPPackage(vector<char> rtpPackage);
void splitData(vector<char> rawData);
frame_packet getFrame();
void decodeProcess();
void splitMainRtpPackage(vector<char> rtpPackage);
