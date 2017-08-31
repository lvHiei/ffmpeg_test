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
#include "codec/VVAVEncoder.h"
#include "codec/VVAVDecoder.h"

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


void testCodec(){
	const char* in_file = "/home/mj/disk/videotest/2017/08/25/format_test.mp4";
//	const char* out_file = "/home/mj/disk/videotest/2017/08/25/out_format.mp4";
	const char* out_file_faststart = "/home/mj/disk/videotest/2017/08/25/out_codec_faststart.mp4";

	VVAVFormat* pVVFormat = new VVAVFormat();
	VVAVEncoder* pEncoder = new VVAVEncoder();
	VVAVDecoder* pDecoder = new VVAVDecoder();


	AVFormatContext* pIFCtx = pVVFormat->alloc_foramt_context();
	AVFormatContext* pOFCtx = pVVFormat->alloc_foramt_context();

	AVCodecContext* pADeCodecCtx = NULL;
	AVCodecContext* pVDeCodecCtx = NULL;
	AVCodecContext* pAEnCodecCtx = NULL;
	AVCodecContext* pVEnCodecCtx = NULL;

    int iAudioIndex;
    int iVideoIndex;
    int oAudioIndex;
    int oVideoIndex;


    AVStream* pVideoStream = NULL;
    AVStream* pAudioStream = NULL;
    int ret = 0;
    int framerate;
    int bitrate;

	AVFrame* audioFrame = av_frame_alloc();
	AVFrame* videoFrame = av_frame_alloc();

	int got_picture;

    ret = pVVFormat->open_input_file(&pIFCtx, in_file);

    if(ret < 0){
    	goto codecFail;
    }

    iAudioIndex = pVVFormat->find_audio_stream(pIFCtx);
    iVideoIndex = pVVFormat->find_video_stream(pIFCtx);

    pADeCodecCtx = pIFCtx->streams[iAudioIndex]->codec;
    pVDeCodecCtx = pIFCtx->streams[iVideoIndex]->codec;

//    ret = pDecoder->open_decoder(pADeCodecCtx);
//    if(ret < 0){
//    	goto codecFail;
//    }

    ret = pDecoder->open_decoder(pVDeCodecCtx);
    if(ret < 0){
    	goto codecFail;
    }

    ret = pVVFormat->alloc_output_context(&pOFCtx, out_file_faststart);

    if(ret < 0){
    	goto codecFail;
    }

//    pVideoStream = pVVFormat->add_stream(pOFCtx, pIFCtx->streams[iVideoIndex], false);
//    pAudioStream = pVVFormat->add_stream(pOFCtx, pIFCtx->streams[iAudioIndex], false);

	pVideoStream = pVVFormat->add_stream(pOFCtx, (AVCodecContext*)NULL, false);
//    pAudioStream = pVVFormat->add_stream(pOFCtx, (AVCodecContext*)NULL, false);
    pAudioStream = pVVFormat->add_stream(pOFCtx, pIFCtx->streams[iAudioIndex], true);

    pVideoStream->time_base = pIFCtx->streams[iVideoIndex]->time_base;
    pAudioStream->time_base = pIFCtx->streams[iAudioIndex]->time_base;


    oAudioIndex = pAudioStream->index;
    oVideoIndex = pVideoStream->index;


    pAEnCodecCtx = pAudioStream->codec;
    pVEnCodecCtx = pVideoStream->codec;

    framerate = pVDeCodecCtx->framerate.num == 0 ?
    		12 : pVDeCodecCtx->framerate.num / pVDeCodecCtx->framerate.den;

    bitrate  = pVDeCodecCtx->bit_rate > 0 ? pVDeCodecCtx->bit_rate : 800*1000;

    ret = pEncoder->init_and_open_video_encoder(pVEnCodecCtx, pVDeCodecCtx->codec_id,
    		pVDeCodecCtx->width, pVDeCodecCtx->height, framerate, bitrate);

    if(ret < 0){
    	goto codecFail;
    }

//    ret = pEncoder->init_and_open_audio_encoder(pAEnCodecCtx, pADeCodecCtx->codec_id,
//    		pADeCodecCtx->sample_rate, pADeCodecCtx->channels, pADeCodecCtx->bit_rate, 0);
//
//	if(ret < 0){
//		goto codecFail;
//	}

	printf("ia:%d,iv:%d,oa:%d,ov:%d\n", iAudioIndex, iVideoIndex, oAudioIndex, oVideoIndex);

	ret = pVVFormat->open_output_file(pOFCtx);
	if(ret < 0){
		goto codecFail;
	}

	ret = pVVFormat->write_mp4_header(pOFCtx, true);
	if(ret < 0){
		goto codecFail;
	}

	AVPacket rpacket;
	AVPacket epacket;

	av_init_packet(&epacket);

	// 这里封装也是有问题的 应该按照时间戳大小封装
	while(true){
		if(pVVFormat->read_packet(pIFCtx, &rpacket) < 0){
			break;
		}

		if(rpacket.stream_index == iAudioIndex){
//			ret = pDecoder->decode_audio(pADeCodecCtx, audioFrame, &got_picture, &rpacket);
//			if(ret >= 0 && got_picture){
//				// 这里编码音频是错误的 应该按照音频编码需要的长度喂给编码器 因为是测试程序 所以这里就不写那么复杂了
//				ret = pEncoder->encode_audio_frame(pAEnCodecCtx, &epacket, audioFrame, &got_picture);
//				if(ret >= 0 && got_picture){
//					epacket.stream_index = oAudioIndex;
//					pVVFormat->write_packet(pOFCtx, epacket);
//					av_packet_unref(&epacket);
//				}
//			}

			rpacket.stream_index = oAudioIndex;
			pVVFormat->write_packet(pOFCtx, rpacket);
		}else if(rpacket.stream_index == iVideoIndex){

			ret = pDecoder->decode_video(pVDeCodecCtx, videoFrame, &got_picture, &rpacket);
			if(ret >= 0 && got_picture){
				ret = pEncoder->encode_video_frame(pVEnCodecCtx, &epacket, videoFrame, &got_picture);
				if(ret >= 0 && got_picture){
					epacket.stream_index = oVideoIndex;
					pVVFormat->write_packet(pOFCtx, epacket);
					av_packet_unref(&epacket);
				}
			}
		}

		av_packet_unref(&rpacket);
	}

	while(true){
		ret = pDecoder->flush_video_decoder(pVDeCodecCtx, videoFrame, &got_picture, &rpacket);
		if(ret < 0 || !got_picture){
			break;
		}

		ret = pEncoder->encode_video_frame(pVEnCodecCtx, &epacket, videoFrame, &got_picture);
		if(ret >= 0 && got_picture){
			epacket.stream_index = oVideoIndex;
			pVVFormat->write_packet(pOFCtx, epacket);
			av_packet_unref(&epacket);
		}
	}


	while(true){
		ret = pEncoder->encode_video_frame(pVEnCodecCtx, &epacket, NULL, &got_picture);
		if(ret < 0 || !got_picture){
			break;
		}

		epacket.stream_index = oVideoIndex;
		pVVFormat->write_packet(pOFCtx, epacket);
		av_packet_unref(&epacket);
	}

//	while(true){
//		ret = pDecoder->flush_audio_decoder(pADeCodecCtx, audioFrame, &got_picture, &rpacket);
//		if(ret < 0 || !got_picture){
//			break;
//		}
//
//		ret = pEncoder->encode_audio_frame(pAEnCodecCtx, &epacket, audioFrame, &got_picture);
//		if(ret >= 0 && got_picture){
//			epacket.stream_index = oAudioIndex;
//			pVVFormat->write_packet(pOFCtx, epacket);
//			av_packet_unref(&epacket);
//		}
//	}
//
//
//	while(true){
//		ret = pEncoder->encode_audio_frame(pAEnCodecCtx, &epacket, NULL, &got_picture);
//		if(ret < 0 || !got_picture){
//			break;
//		}
//
//		epacket.stream_index = oAudioIndex;
//		pVVFormat->write_packet(pOFCtx, epacket);
//		av_packet_unref(&epacket);
//	}


	pVVFormat->write_tailer(pOFCtx);

codecFail:

	pEncoder->close_encoder(pVEnCodecCtx);
//	pEncoder->close_encoder(pAEnCodecCtx);
	pDecoder->close_decoder(pVDeCodecCtx);
//	pDecoder->close_decoder(pADeCodecCtx);
	pVVFormat->close_in_out_file(&pIFCtx, &pOFCtx);
	pVVFormat->free_format_context(pIFCtx);
	pVVFormat->free_format_context(pOFCtx);
	delete pVVFormat;
	delete pEncoder;
	delete pDecoder;
}

int main() {
//	testLocalVideo();

//	testFilter();

//	testSpeed();

//	testFormat();

	testCodec();

//	testPureFilter();
	printf("hello world");
	return 0;
}
