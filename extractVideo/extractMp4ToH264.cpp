//
// Created by 小汉陈 on 2022/9/17.
//

extern "C"{
    //编码
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libavutil/log.h"
}
#include "demuxer_video.h"
#include "iostream"
using namespace std;

/**
 * 抽取mp4的视频到
 * @param input_fmt_ctx
 * @param video_stream_index
 * @param fp
 */
static void extractVideoData(AVFormatContext *input_fmt_ctx,int video_stream_index,FILE *fp);

int mainExtractVideoData(){
    int ret = 0;
    // 打开多媒体上下文
    char *src_file_path = "/Users/xiaohanchen/Desktop/storm.mp4";   // 源文件
    char *dst_file_path = "/Users/xiaohanchen/Desktop/stormCode.h264";  // 目标文件
    AVFormatContext *fmt_ctx = NULL;
    if ((ret = avformat_open_input(&fmt_ctx, src_file_path, NULL, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "could not open source file: %s, %d(%s)\n", src_file_path, ret, av_err2str(ret));
        avformat_close_input(&fmt_ctx);
        return ret;
    }

    // 检索多媒体信息
    if ((ret = avformat_find_stream_info(fmt_ctx, NULL)) < 0) {
        av_log(NULL, AV_LOG_ERROR, "failed to find stream infomation: %s, %d(%s)\n", src_file_path, ret, av_err2str(ret));
        avformat_close_input(&fmt_ctx);
        return ret;
    }
    // 打印多媒体信息
    av_dump_format(fmt_ctx, 0, src_file_path, 0);


    // 找到最佳流
    int video_channel_id = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (video_channel_id < 0) {
        av_log(NULL, AV_LOG_DEBUG, "Could not find %s stream in input file %s\n",
               av_get_media_type_string(AVMEDIA_TYPE_VIDEO),
               src_file_path);
        avformat_close_input(&fmt_ctx);
        return AVERROR(EINVAL);
    }
    FILE *dstFile = fopen(dst_file_path,"wb+");
    extractVideoData(fmt_ctx,video_channel_id,dstFile);
    return 0;
}


static void extractVideoData(AVFormatContext *input_fmt_ctx,int video_stream_index,FILE *fp){
    int ret = 0;
    AVBSFContext * h264bsfc;
    const AVBitStreamFilter * filter = av_bsf_get_by_name("h264_mp4toannexb");
    ret = av_bsf_alloc(filter, &h264bsfc);
    avcodec_parameters_copy(h264bsfc->par_in, input_fmt_ctx->streams[video_stream_index]->codecpar);
    av_bsf_init(h264bsfc);

    AVPacket* packet = av_packet_alloc();
    while( av_read_frame(input_fmt_ctx, packet) >= 0 ) {
        if( packet->stream_index == video_stream_index ) {
            ret = av_bsf_send_packet(h264bsfc, packet);
            if(ret < 0)
                cout << "av_bsf_send_packet error" << endl;

            while ((ret = av_bsf_receive_packet(h264bsfc, packet)) == 0) {
                fwrite(packet->data, packet->size, 1, fp);
            }
        }

        av_packet_unref(packet);
    }

    av_packet_free(&packet);
    av_bsf_free(&h264bsfc);
}