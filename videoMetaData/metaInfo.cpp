//
// Created by 小汉陈 on 2022/9/7.
//

extern "C"{
    //编码
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/log.h"
}
using namespace std;

int mainMetaInfo(){
    av_register_all();
    av_log_set_level(AV_LOG_INFO);
    AVFormatContext *avFormatContext = NULL;   // 这里一定要设置null
    char *filePath = "/Users/xiaohanchen/Desktop/milan.mp4";
    int ret = avformat_open_input(&avFormatContext,filePath,NULL,NULL);
    if(ret < 0){
        av_log(NULL,AV_LOG_INFO,"can't open file %s",filePath);
        return -1;
    }
    av_log(NULL,AV_LOG_INFO,"open file succcess");
    av_dump_format(avFormatContext,0,filePath,0); // 第四个参数为输入输出，0为输入，1为输出
    avformat_close_input(&avFormatContext);
    return 0;
}