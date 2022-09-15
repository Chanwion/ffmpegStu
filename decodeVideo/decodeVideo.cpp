//
// Created by 小汉陈 on 2022/9/15.
//

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

#include <string.h>
#include <unistd.h>
#include "convert.h"
using namespace std;
static int rec_status = 0;
#include <iostream>

// 视频宽高应是固定的
#define V_WIDTH 1280
#define V_HEIGHT 720

static
void open_decoder(AVCodecContext **codec_ctx);

static
AVFrame* create_frame();

static
AVFormatContext* open_dev();

static
void encode(AVCodecContext *codec,AVFrame *srcFrame,AVPacket *dstPacket,FILE *encodeFile);

int main(){
    int ret = 0;
    avdevice_register_all(); // 设备注册
    AVCodecContext *codec_ctx = NULL;
    AVFrame *frame = create_frame();

    AVFormatContext *format_ctc = open_dev();    // 打开摄像头
    if(!format_ctc){
        printf("Error, Failed to open device!\n");
        exit(1);
    }
    const char *out = "/Users/xiaohanchen/Desktop/one.yuv";   //得到的是原始数据
    const char *encodeFilePath = "/Users/xiaohanchen/Desktop/one.h264";   //得到的是原始数据
    FILE *outfile = fopen(out, "wb+");
    FILE *encodeFile = fopen(encodeFilePath, "wb+");
    int count = 0;
    open_decoder(&codec_ctx);

    // 摄像头数据采集
    AVPacket *pkt = av_packet_alloc();
    if(pkt == NULL){
        cout << "Failed to alloc avpacket" << endl;
        exit(1);
    }
    AVPacket *encodePkt = av_packet_alloc();
    if(!encodePkt){
        cout << "Failed to alloc encode packet" << endl;
        exit(1);
    }
    while( true){
        ret = av_read_frame(format_ctc, pkt);
        if(ret == -35){
            usleep(100);
            continue;
        }
        yuyv422ToYuv420p(&frame,pkt);
        //写入文件
//        fwrite(frame->data, 1, V_WIDTH*V_HEIGHT*1.5, outfile);
//        fwrite(pkt->data, 1, V_WIDTH*V_HEIGHT*2, outfile);


        // frame 的数据写入，不能像packet一样，要根据自己的yuv通道去根据yuv420p一样写入
        fwrite(frame->data[0], 1, V_WIDTH*V_HEIGHT, outfile);
        fwrite(frame->data[1], 1, V_WIDTH*V_HEIGHT / 4, outfile);
        fwrite(frame->data[2], 1, V_WIDTH*V_HEIGHT / 4, outfile);
        frame->pts = count++;
        fflush(outfile);

//        void encode(AVCodecContext *codec,AVFrame *srcFrame,AVPacket *dstPacket,FILE *encodeFile);
        // 编码
        encode(codec_ctx,frame,encodePkt,encodeFile);

        cout << " frame  " << count << endl;
        if(count > 100){
            break;
        }
        av_packet_unref(pkt);
    }

//    avcodec_send_frame
    return 0;
}




/**
  * @brief open camera device
  * @return succ: AVFormatContext*, fail: NULL
  */
static
AVFormatContext* open_dev(){

    int ret = 0;
    char errors[1024] = {0, };

    //ctx
    AVFormatContext *fmt_ctx = NULL;
    AVDictionary *options = NULL;
    av_dict_set(&options,"framerate","30",0);
//    av_dict_set(&options, "video_size", V_WIDTH+"x"+V_HEIGHT, 0);
    av_dict_set(&options, "video_size", "1280x720", 0);
    av_dict_set(&options, "pixel_format", "yuyv422", 0);
//    av_dict_set(&options, "pixel_format", AV_PIX_FMT_UYVY422, 0);

    //[[video device]:[audio device]]
    const char *devicename = "0";

    //get format
    AVInputFormat *iformat = av_find_input_format("avfoundation");

    //open device
    ret = avformat_open_input(&fmt_ctx, devicename, iformat, &options);
    if(ret  < 0 ){
        av_strerror(ret, errors, 1024);
        fprintf(stderr, "Failed to open video device, [%d]%s\n", ret, errors);
        return NULL;
    }

    return fmt_ctx;
}


/**
 *
 * @param frame 创建一个可用于编码的avframe
 */
static
AVFrame* create_frame(){
    int ret = 0;
    AVFrame *frame = NULL;
    frame = av_frame_alloc();     // 初始化，分配完内存地址，什么都是空的，图片参数什么的
    // 用于之后编码使用，需要设置基本的信息，宽高以及格式
    // 音频有三个要设置，通道数，单通道采集数，位深
    // 视频有三个要设置，格式，宽，高
    frame->height = V_HEIGHT;
    frame->width = V_WIDTH;
    frame->format = AV_PIX_FMT_YUV420P;
    // 初始化，需要用他真正打开空间
    ret = av_frame_get_buffer(frame,32);  // 只用在调用函数之前设置好图像的宽高和图像格式等信息，音频设置好nb_samples，channel_layout 和采样格式等信息
    if(ret < 0){
        cout << "Failed to alloc frame for buffer" << endl;
        av_frame_free(&frame);   // 释放内存
        exit(1);
    }

    return frame;
}


static
void open_decoder(AVCodecContext **codec_ctx){

    AVCodec *codec = avcodec_find_encoder_by_name("libx264");
//    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if(codec == NULL){
        cout << "cannot find decoder codec" << endl;
        exit(0);
    }

    *codec_ctx = avcodec_alloc_context3(codec);
    if(codec_ctx == NULL){
        cout << "Could not allocate video codec context" << endl;
        exit(0);
        return;
    }
//    //SPS PPS
    (*codec_ctx)->gop_size     = 200;      //GOP最大差距
    (*codec_ctx)->max_b_frames = 1;           // b帧多增加编码时间，实时通信要求这个没有b帧
//
    // 视频基本长宽，必设
    (*codec_ctx)->width        = V_WIDTH;
    (*codec_ctx)->height       = V_HEIGHT;
    // 码率
//    (*codec_ctx)->bit_rate     = 400000;

    // 帧率
    (*codec_ctx)->time_base.num= 1;   //每秒25帧
    (*codec_ctx)->time_base.den= 25;

    // 编码的格式是必须的
    (*codec_ctx)->pix_fmt      = AV_PIX_FMT_YUV420P; //原视频格式

    // SPS/PPS
    (*codec_ctx)->profile = FF_PROFILE_H264_HIGH_444;
    (*codec_ctx)->level = 50; //表示LEVEL是5.0

    //设置分辫率
    (*codec_ctx)->width = V_WIDTH;   // 640
    (*codec_ctx)->height = V_HEIGHT; // 480

    // GOP
    (*codec_ctx)->gop_size = 20;
    (*codec_ctx)->keyint_min = 25; // option



    //设置输入YUV格式
    (*codec_ctx)->pix_fmt = AV_PIX_FMT_YUV420P;

    int ret = avcodec_open2(*codec_ctx,codec,NULL);
    if(ret < 0){
        cout << "could not open avcodec " << endl;
        exit(1);
    }
}

/**
 * @brief 编码
 * @param codec
 * @param srcFrame
 * @param dstPacket
 * @param encodeFile
 */
static
void encode(AVCodecContext *codec,AVFrame *srcFrame,AVPacket *dstPacket,FILE *encodeFile){

    int ret = 0;
    ret = avcodec_send_frame(codec,srcFrame);
    if(ret < 0 ){
        cout << "Failed to send frame to encode " << av_err2str(ret) << endl;
        exit(1);
    }
    // 发送一次压缩命令，可能会分多次收到压缩好的包
    while(ret >= 0){
        ret = avcodec_receive_packet(codec,dstPacket);
        //如果编码器数据不足时会返回  EAGAIN,或者到数据尾时会返回 AVERROR_EOF
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            return;
        } else if (ret < 0) {
            printf("Error, Failed to encode!\n");
            exit(1);
        }
        cout << "write h264" << endl;
        fwrite(dstPacket->data,1,dstPacket->size,encodeFile);
        fflush(encodeFile);
        av_packet_unref(dstPacket);
    }


}
