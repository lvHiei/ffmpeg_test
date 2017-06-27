//============================================================================
// Name        : ffmpeg_test.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

using namespace std;

#include "clipVideo/LocalVideo.h"
#include "avfilter/VVFilterUtil.h"
#include "avspeed/AVSpeed.h"

void testFilter()
{
	const char* path = "/home/mj/disk/videotest/2017/06/22/";
	const char* videoFile = "clip.flv";
	const char* picFile = "zimu1.png";
	const char* outFile = "filter.flv";
//	const char* outFile = "filter.yuv";

	char absVideo[1024] = {0};
	char absPic[1024] = {0};
	char absOut[1024] = {0};

	strcat(absVideo, path);
	strcat(absVideo, videoFile);

	strcat(absPic, path);
	strcat(absPic, picFile);

	strcat(absOut, path);
	strcat(absOut, outFile);

	VVFilterUtil* pFilter = new VVFilterUtil();
	pFilter->initFilter(absVideo, absPic, absOut);
	pFilter->createFilter();

	pthread_join(pFilter->getThreadId(), NULL);

	delete pFilter;
	return;
}

void testLocalVideo()
{
	LocalVideo* pVideo = LocalVideo::getInstance();

	pVideo->prepare("/home/mj/disk/videotest/2017/06/06/clip/linshi.mp4");
	pVideo->initDecoder();
}

void testSpeed()
{

	float speed = 1.5f;

	const char* path = "/home/mj/disk/videotest/2017/06/26/";
	const char* videoFile = "test_120.flv";
	const char* outFile = "speed_%1.1f.flv";

	char absVideo[1024] = {0};
	char absOut[1024] = {0};
	char out[1024] = {0};
	sprintf(out, outFile, speed);

	strcat(absVideo, path);
	strcat(absVideo, videoFile);

	strcat(absOut, path);
	strcat(absOut, out);

	AVSpeed* pSpeed = new AVSpeed();
	pSpeed->initFile(absVideo, absOut);
	pSpeed->setSpeed(speed);
	pSpeed->start();

	pthread_join(pSpeed->getThreadId(), NULL);

	delete pSpeed;
}

int main() {
//	testLocalVideo();

	testFilter();

//	testSpeed();

//	printf("hello world");

	return 0;
}
