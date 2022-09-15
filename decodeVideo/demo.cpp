//
// Created by 小汉陈 on 2022/9/16.
//

#define CAMERA_DEV "/dev/video0"
#define CAMERA_FMT AV_PIX_FMT_YUYV422
#define ENCODE_FMT AV_PIX_FMT_YUV420P

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


typedef struct {
    uint8_t *buf[4];
    int linesize[4];
    int size;
} cnv_t;

typedef struct {
    AVFormatContext *fmt_ctx;   // 输入设备
    AVCodecContext *enc_ctx;    // 编码器上下文
    AVPacket *camera_packet;    // 从设备中读取图像
    AVFrame *yuvframe;          // 原始数据帧
    int frame_index;            // 对应帧中的pts
    void *convert_ctx;          // 转换上下文
    cnv_t cnv_src, cnv_dest;    // 转换前后
} AVInputDev;

void get_rawframe(AVInputDev *input) {
    AVPacket *camera_packet = input->camera_packet;
    // 摄像头获取图像数据
    av_read_frame(input->fmt_ctx, camera_packet);
    memcpy(input->cnv_src.buf[0], camera_packet->data, camera_packet->size);
    // 图像格式转化
    sws_scale((struct SwsContext *)input->convert_ctx,\
            (const uint8_t **)input->cnv_src.buf, input->cnv_src.linesize, \
            0, input->enc_ctx->height, input->cnv_dest.buf, input->cnv_dest.linesize);
    av_packet_unref(camera_packet); // 清理数据

    input->yuvframe->data[0] = input->cnv_dest.buf[0];
    input->yuvframe->data[1] = input->cnv_dest.buf[1];
    input->yuvframe->data[2] = input->cnv_dest.buf[2];
    input->frame_index++;
    input->yuvframe->pts = input->frame_index;
}


void convert_init(AVInputDev *input, int w, int h) {
    // 图像格式转化
    struct SwsContext *sws_ctx;
    sws_ctx = sws_getContext(w, h, CAMERA_FMT, \
            w, h, ENCODE_FMT, 0, NULL, NULL, NULL);

    cnv_t *yuy2 = &input->cnv_src;
    cnv_t *iyuv = &input->cnv_dest;
    yuy2->size = av_image_alloc(yuy2->buf, yuy2->linesize, w, h, CAMERA_FMT, 1);
    iyuv->size = av_image_alloc(iyuv->buf, iyuv->linesize, w, h, ENCODE_FMT, 1);

    input->convert_ctx = sws_ctx;
}

int VideoInput_Init(AVInputDev *input, int w, int h) {
    avdevice_register_all();
    AVInputFormat *in_fmt = av_find_input_format("v4l2");
    if (in_fmt == NULL) {
        printf("can't find_input_format\n");
        return -1;
    }

    // 设置摄像头的分辨率
    AVDictionary *option = NULL;
    char video_size[10];
    sprintf(video_size, "%dx%d", w, h);
    av_dict_set(&option, "video_size", video_size, 0);

    AVFormatContext *fmt_ctx = NULL;
    if (avformat_open_input(&fmt_ctx, CAMERA_DEV, in_fmt, &option) < 0) {
        printf("can't open_input_file\n");
        return -1;
    }
    av_dump_format(fmt_ctx, 0, CAMERA_DEV, 0);

    // 初始化编码器
    AVCodec *cod = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (cod == NULL) {
        printf("failed to find encoder\n");
        return -1;
    }

    AVCodecContext *cod_ctx = avcodec_alloc_context3(cod);
    cod_ctx->pix_fmt = ENCODE_FMT;
    cod_ctx->width = w;
    cod_ctx->height = h;
    cod_ctx->time_base.num = 1;
    cod_ctx->time_base.den = 30;
    cod_ctx->bit_rate = 400000;
    cod_ctx->qmin = 10;
    cod_ctx->qmax = 51;
    cod_ctx->max_b_frames = 0;
    cod_ctx->thread_count = 4;
    cod_ctx->gop_size = 15;

    if (avcodec_open2(cod_ctx, cod, NULL) < 0) {
        printf("failed to open encoder\n");
        return -1;
    }

    // 格式转换初始化
    convert_init(input, w, h);

    // 初始化frame，存放原始数据
    AVFrame *frame = av_frame_alloc();
    frame->width = w;
    frame->height = h;
    frame->format = ENCODE_FMT;
    av_frame_get_buffer(frame, 0);
    input->camera_packet = av_packet_alloc();

    input->fmt_ctx = fmt_ctx;
    input->enc_ctx = cod_ctx;
    input->yuvframe = frame;
    input->frame_index = 0;
    return 0;
}
