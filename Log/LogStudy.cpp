//
// Created by 小汉陈 on 2022/9/6.
//

extern "C"{
    //编码
    #include "libavcodec/avcodec.h"
    #include "libavutil/log.h"
    #include "string.h"
}
#include <iostream>

using namespace std;

int main(){
    char *ffmpeg = "ffmpeg";
    av_log_set_level(AV_LOG_DEBUG);
    av_log(NULL,AV_LOG_DEBUG,"Hello %s ",ffmpeg);
    return 0;
}