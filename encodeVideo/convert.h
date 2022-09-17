//
// Created by 小汉陈 on 2022/9/16.
//

#ifndef FFMPEGCLIONC___CONVERT_H
#define FFMPEGCLIONC___CONVERT_H
#include "iostream"
extern "C"{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/log.h"
    #include "libavdevice/avdevice.h"
    #include <libswresample/swresample.h>
    #include "libavutil/avutil.h"
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>

}


using namespace std;
void yuyv422ToYuv420p(AVFrame **frame, AVPacket *pkt);
void yuyv422ToYuv420p(AVFrame *frame, AVPacket *pkt,int width, int height);
bool convert(AVFrame *frame);
#endif //FFMPEGCLIONC___CONVERT_H
