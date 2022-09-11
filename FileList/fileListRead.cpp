//
// Created by 小汉陈 on 2022/9/6.
//

extern "C"{
//编码
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/log.h"
    #include "string.h"
}
#include <iostream>

int mainFileList(){

    AVIODirContext *dirContext;
    AVIODirEntry *dirEntry;
    int ret = avio_open_dir(&dirContext,"/Users/xiaohanchen/Desktop",NULL);
    av_log_set_level(AV_LOG_DEBUG);
    if(ret < 0){
        av_log(NULL,AV_LOG_ERROR,"open file error");
        return -1;
    }
    av_log(NULL,AV_LOG_DEBUG,"open file success");
    // int avio_read_dir(AVIODirContext *s, AVIODirEntry **next);

    while(1){
        ret = avio_read_dir(dirContext,&dirEntry);
        if(ret < 0){
            av_log(NULL,AV_LOG_DEBUG,"read fileDir error");
            return -1;
        }
        av_log(NULL,AV_LOG_DEBUG,"read file success");
        char *fileName = dirEntry->name;
        int fileSize = dirEntry->size;
        av_log(NULL,AV_LOG_DEBUG,"fileName = %s,size = %d \n",fileName,fileSize);
        avio_free_directory_entry(&dirEntry);

    }

    avio_close_dir(&dirContext);

    return 0;
}