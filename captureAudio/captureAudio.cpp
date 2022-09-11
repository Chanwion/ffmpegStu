//
// Created by 小汉陈 on 2022/9/9.
//
extern "C"{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/log.h"
    #include "libavdevice/avdevice.h"
}

void capture_audio() ;

int main(){
    capture_audio();
    return 0;
}

void capture_audio() {
    int ret = 0;
    char errors[1024] = {0, };
    AVFormatContext *fmt_ctx = NULL;
    AVDictionary *options = NULL;
    int count = 0;
    AVPacket pkt;

    //[[video device]:[audio device]]   mac是如此获取音频的
    char *devicename = ":0";

    av_log_set_level(AV_LOG_DEBUG);

    //注册设备
    avdevice_register_all();

    //设置采集方式
    AVInputFormat *iformat = av_find_input_format("avfoundation");   // mac是基于avfoudation这个库去获取

    //打开音频设备
    if ((ret = avformat_open_input(&fmt_ctx, devicename, iformat, &options)) < 0) {
        av_strerror(ret, errors, 1024);
        fprintf(stderr, "Failed to open audio device, [%d]%s\n", ret, errors);
        return;
    }

    //写入文件 w：写入 b：二进制 +：文件不存在就自动创建
    char *out = "/Users/mac/Downloads/my_av_base.pcm";
    FILE *outfile = fopen(out, "wb+");

    av_init_packet(&pkt);
    //读取音频数据
    while ((ret = av_read_frame(fmt_ctx, &pkt)) == 0  && count++ < 500) {

        //写入文件
        fwrite(pkt.data, pkt.size, 1, outfile);
        fflush(outfile);

        av_log(NULL, AV_LOG_INFO, "pkt size is %d(%p), count=%d \n",pkt.size, pkt.data, count);
        //释放资源
        av_packet_unref(&pkt);
    }

    //关闭文件
    fclose(outfile);

    //关闭设备
    avformat_close_input(&fmt_ctx);

    av_log(NULL, AV_LOG_DEBUG, "finish!\n");
}



