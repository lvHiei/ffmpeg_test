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
#include "avfilter/VVFilterUtilPure.h"
#include "avspeed/AVSpeed.h"
#include "format/VVAVFormat.h"

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
//	float speed = 0.5f;
//	float speed = 1.0f;
//	float speed = 1.5f;
	float speed = 2.0f;

	const char* path = "/home/mj/disk/videotest/2017/06/26/";
	const char* videoFile = "test_120.flv";
	const char* outFile = "speed_video_%1.1f.flv";

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


void testPureFilter()
{
	AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	AVCodecContext* pCodecContext = avcodec_alloc_context3(codec);
	pCodecContext->width = 360;
	pCodecContext->height = 640;
	pCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
	pCodecContext->time_base = AVRational{1, 25};
	pCodecContext->sample_aspect_ratio = AVRational{9, 16};

	int stride =  pCodecContext->width * pCodecContext->height;

	const char* filterdes = "movie=/home/mj/disk/videotest/2017/07/13/filter.png[mf];[in][mf]overlay=x=main_w-overlay_w-20:y=20[out]";

	VVFilterUtilPure* pFilter = new VVFilterUtilPure(AVFILTER_VIDEO, pCodecContext);
	pFilter->setFilterDescr(filterdes);
	pFilter->initFilter();

	FILE* pFile = fopen("/home/mj/disk/videotest/2017/07/13/360_640.yuv", "r");
	FILE* pOutFile = fopen("/home/mj/disk/videotest/2017/07/13/360_640f.yuv", "w");

	int read_len = pCodecContext->width*pCodecContext->height*3/2;

	uint8_t* originData = (uint8_t*)malloc(read_len);

	AVFrame* pOriginFrame = av_frame_alloc();
	AVFrame* pFilterFrame = av_frame_alloc();

	pOriginFrame->width = pCodecContext->width;
	pOriginFrame->height = pCodecContext->height;
	pOriginFrame->format = pCodecContext->pix_fmt;

	av_image_fill_arrays(pOriginFrame->data, pOriginFrame->linesize, originData,
			AV_PIX_FMT_YUV420P,pCodecContext->width , pCodecContext->height,1);
	pOriginFrame->data[0] = originData;
	pOriginFrame->data[1] = originData + stride;
	pOriginFrame->data[2] = originData + stride * 5/4;

	SwsContext* m_pImgConvertCtx = sws_getContext(pCodecContext->width, pCodecContext->height, pCodecContext->pix_fmt,
									pCodecContext->width, pCodecContext->height, AV_PIX_FMT_YUV420P,
                                      SWS_BICUBIC, NULL, NULL, NULL);

    int pts = 0;
	while(1)
	{
		int aread_len = fread(originData, 1, read_len, pFile);

		if(aread_len != read_len){
			break;
		}

		pOriginFrame->pts = pts++;
		pFilter->processFilter(pOriginFrame, pFilterFrame);

	    sws_scale(m_pImgConvertCtx, (const uint8_t* const*)pFilterFrame->data, pFilterFrame->linesize, 0, pCodecContext->height,
	    		pOriginFrame->data, pOriginFrame->linesize);

		av_frame_unref(pFilterFrame);
		fwrite(originData, 1, read_len, pOutFile);
		printf("write 1 frame\n");
	}


	fclose(pFile);
	fclose(pOutFile);

	av_frame_free(&pOriginFrame);
	av_frame_free(&pFilterFrame);

	sws_freeContext(m_pImgConvertCtx);

	free(originData);
	delete pFilter;
}

void testFormat(){
	const char* in_file = "/home/mj/disk/videotest/2017/08/25/format_test.mp4";
	const char* out_file = "/home/mj/disk/videotest/2017/08/25/out_format.mp4";
	const char* out_file_faststart = "/home/mj/disk/videotest/2017/08/25/out_format_faststart.mp4";

	VVAVFormat* pVVFormat = new VVAVFormat();

	AVFormatContext* pIFCtx = pVVFormat->alloc_foramt_context();
	AVFormatContext* pOFCtx = pVVFormat->alloc_foramt_context();

    int iAudioIndex;
    int iVideoIndex;
    int oAudioIndex;
    int oVideoIndex;

    int ret = 0;


//	ret = pVVFormat->open_in_out_file(&pIFCtx, in_file, iAudioIndex, iVideoIndex,
//			&pOFCtx, out_file, oAudioIndex, oVideoIndex);
	ret = pVVFormat->open_in_out_file(&pIFCtx, in_file, iAudioIndex, iVideoIndex,
			&pOFCtx, out_file_faststart, oAudioIndex, oVideoIndex);

	if(ret < 0){
		goto fail;
	}

	printf("ia:%d,iv:%d,oa:%d,ov:%d\n", iAudioIndex, iVideoIndex, oAudioIndex, oVideoIndex);

//	ret = pVVFormat->write_hearder(pOFCtx);
	ret = pVVFormat->write_mp4_header(pOFCtx, true);
	if(ret < 0){
		goto fail;
	}

	AVPacket packet;

	while(true){
		if(pVVFormat->read_packet(pIFCtx, &packet) < 0){
			break;
		}

		if(packet.stream_index == iAudioIndex){
			packet.stream_index = oAudioIndex;
		}else if(packet.stream_index == iVideoIndex){
			packet.stream_index = oVideoIndex;
		}

		pVVFormat->write_packet(pOFCtx, packet);
		av_packet_unref(&packet);
	}

	pVVFormat->write_tailer(pOFCtx);

fail:

	pVVFormat->close_in_out_file(&pIFCtx, &pOFCtx);
	pVVFormat->free_format_context(pIFCtx);
	pVVFormat->free_format_context(pOFCtx);
	delete pVVFormat;
}

int main() {
//	testLocalVideo();

//	testFilter();

//	testSpeed();

	testFormat();

//	testPureFilter();
	printf("hello world");
	return 0;
}
