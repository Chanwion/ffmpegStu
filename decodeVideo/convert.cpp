//
// Created by 小汉陈 on 2022/9/16.
//


#include "convert.h"

#define SWS_ERROR_BUF(ret) \
    char errbuf[1024]; \
    av_strerror(ret, errbuf, sizeof (errbuf));
#define V_WIDTH 1280
#define V_HEIGHT 720

void yuyv_to_yuv420P(char *in, char*out,int width,int height)
{
    char *p_in, *p_out, *y, *u, *v;
    int index_y, index_u, index_v;
    int i, j, in_len;

    y = out;
    u = out + (width * height);
    v = out + (width * height * 5/4);

    index_y = 0;
    index_u = 0;
    index_v = 0;
    for(j=0; j< height*2; j++)
    {
        for(i=0; i<width; i=i+4)
        {
            *(y + (index_y++)) = *(in + width * j + i);
            *(y + (index_y++)) = *(in + width * j + i + 2);
            if(j%2 == 0)
            {
                *(u + (index_u++)) = *(in + width * j + i + 1);
                *(v + (index_v++)) = *(in + width * j + i + 3);
            }
        }
    }
}



void yuyv422ToYuv420p(AVFrame **frame, AVPacket *pkt) {
    int i = 0;
    int yuv422_length = V_WIDTH * V_HEIGHT * 2;
    int y_index = 0;
    // copy all y
    for (i = 0; i < yuv422_length; i += 2) {
        (*frame)->data[0][y_index] = pkt->data[i];
        y_index++;
    }

    // copy u and v
    int line_start = 0;

    int u_index = 0;
    int v_index = 0;
    // copy u, v per line. skip a line once
    for (i = 0; i < V_HEIGHT; i += 2) {
        // line i offset
        line_start = i * V_WIDTH * 2;
        for (int j = line_start + 1; j < line_start + V_WIDTH * 2; j += 4) {
            (*frame)->data[1][u_index] = pkt->data[j];
            u_index++;
            (*frame)->data[2][v_index] = pkt->data[j + 2];
            v_index++;
        }
    }
}



void yuyv422ToYuv420p(AVFrame *frame, AVPacket *pkt,int width, int height) {
    int i = 0;
    int yuv422_length = width * height * 2;
    int y_index = 0;
    // copy all y
    for (i = 0; i < yuv422_length; i += 2) {
        frame->data[0][y_index] = pkt->data[i];
        y_index++;
    }

    // copy u and v
    int line_start = 0;
    int is_u = 1;
    int u_index = 0;
    int v_index = 0;
    // copy u, v per line. skip a line once
    for (i = 0; i < height; i += 2) {
        // line i offset
        line_start = i * width * 2;
        for (int j = line_start + 1; j < line_start + width * 2; j += 4) {
            frame->data[1][u_index] = pkt->data[j];
            u_index++;
            frame->data[2][v_index] = pkt->data[j + 2];
            v_index++;
        }
    }
}


bool convert(AVFrame *frame){
    // 创建转换上下文
    SwsContext *sws_context = sws_getContext(frame->width, frame->height, AV_PIX_FMT_UYVY422,
                                             frame->width, frame->height, AV_PIX_FMT_YUV420P,
                                             SWS_BILINEAR, 0, 0, 0);
    AVFrame *rgbFrame = av_frame_alloc();
    if (NULL == rgbFrame)
    {
        return false;
    }
    rgbFrame->height = frame->height;
    rgbFrame->width = frame->width;
    rgbFrame->format = AV_PIX_FMT_RGB24;
    if (av_frame_get_buffer(rgbFrame, 0)<0)
    {
        fprintf(stderr, "Could not allocate audio data buffers\n");
        av_frame_free(&rgbFrame);
        rgbFrame = NULL;
        return false;
    }
    int ret  = sws_scale(sws_context, frame->data, frame->linesize,0, frame->height, rgbFrame->data, rgbFrame->linesize);

    cout << "convert result =  " << av_err2str(ret) << endl;
    return true;
}