//
// Created by 小汉陈 on 2022/9/7.
//

extern "C"{
    //编码
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/log.h"
}
#include "../header/adts.h"
#include "iostream"
using namespace std;

int mainAudioExtract() {
    av_register_all();
    av_log_set_level(AV_LOG_INFO);
    int audioChannel = 0;
    char *videoPath = "/Users/xiaohanchen/Desktop/milan.mp4";
    char *audioPath = "/Users/xiaohanchen/Desktop/milan.aac";
    AVFormatContext *videoFormatContext = NULL;
    int ret = avformat_open_input(&videoFormatContext,videoPath,NULL,NULL);
    if(ret < 0){
        av_log(NULL, AV_LOG_ERROR, "can't open video file %s", av_err2str(ret));
        return -1;
    }
    // 一般 0 在c语言都是有含义的，不需要填充的数值给-1
    ret = av_find_best_stream(videoFormatContext,AVMEDIA_TYPE_AUDIO,-1,-1,NULL,0);
    av_log(NULL,AV_LOG_INFO,"find best track or stream %d \n",ret);
    if(ret < 0){
        av_log(NULL, AV_LOG_ERROR, "can't find best video track or stream  %s \n", av_err2str(ret));
        return -1;
    }

    audioChannel = ret;
    AVPacket packet;
    av_init_packet(&packet);
    FILE *dst_fd = fopen(audioPath,"wb");
    char adts_header_buf[7];
    while(av_read_frame(videoFormatContext, &packet) >= 0){
        // 只趣audio track 数据
        if(packet.stream_index == audioChannel){
            // 需要写入头文件
            adts_header(adts_header_buf, packet.size,
                        videoFormatContext->streams[audioChannel]->codecpar->profile,
                        videoFormatContext->streams[audioChannel]->codecpar->sample_rate,
                        videoFormatContext->streams[audioChannel]->codecpar->channels);
            fwrite(adts_header_buf,1,strlen(adts_header_buf),dst_fd);
            ret = fwrite(packet.data,1,packet.size,dst_fd);
            if(ret != packet.size){
                av_log(NULL,AV_LOG_ERROR,"write error %s \n ",av_err2str(ret));
            }
        }
        av_packet_unref(&packet);
    }
    if(dst_fd){
        fclose(dst_fd);
    }

    avformat_close_input(&videoFormatContext);



    return 0;
}