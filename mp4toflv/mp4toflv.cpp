//
// Created by 小汉陈 on 2022/9/17.
//

/*将mp4转成flv格式*/
extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/timestamp.h>
}


int main()
{
    AVOutputFormat *ofmt = NULL;  // 输出格式
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL; // 输入、输出是上下文环境
    AVPacket pkt;
    const char *in_filename, *out_filename;
    int ret, i;
    int stream_index = 0;
    int *stream_mapping = NULL; // 数组用于存放输出文件流的Index
    int stream_mapping_size = 0; // 输入文件中流的总数量

    in_filename  = "/Users/xiaohanchen/Desktop/storm.mp4";
    out_filename = "/Users/xiaohanchen/Desktop/stormCode.ts";
    FILE *dstFile = fopen(out_filename,"wb+");
    if(dstFile == NULL){
        fprintf(stderr, "Could not create input file '%s'", dstFile);
    }

    // 打开输入文件为ifmt_ctx分配内存
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        fprintf(stderr, "Could not open input file '%s'", in_filename);
        goto end;
    }

    // 检索输入文件的流信息
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        fprintf(stderr, "Failed to retrieve input stream information");
        goto end;
    }

    // 打印输入文件相关信息
    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    // 为输出上下文环境分配内存，format_name：指定输出格式的名称。根据格式名称，FFmpeg会推测输出格式。输出格式可以是“flv”，“mkv”等等。
    avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
    if (!ofmt_ctx) {
        fprintf(stderr, "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }

    // 输入文件流的数量
    stream_mapping_size = ifmt_ctx->nb_streams;

    // 分配stream_mapping_size段内存，每段内存大小是sizeof(*stream_mapping)
    stream_mapping = static_cast<int *>(av_mallocz_array(stream_mapping_size, sizeof(*stream_mapping)));
    if (!stream_mapping) {
        ret = AVERROR(ENOMEM);
        goto end;
    }

    // 输出文件格式
    ofmt = ofmt_ctx->oformat;

    // 遍历输入文件中的每一路流，对于每一路流都要创建一个新的流进行输出
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        AVStream *out_stream; // 输出流
        AVStream *in_stream = ifmt_ctx->streams[i]; // 输入流
        AVCodecParameters *in_codecpar = in_stream->codecpar; // 输入流的编解码参数

        // 只保留音频、视频、字幕流，其他的流不需要
        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            stream_mapping[i] = -1;
            continue;
        }

        // 对于输出的流的index重写编号
        stream_mapping[i] = stream_index++;

        // 创建一个对应的输出流
        out_stream = avformat_new_stream(ofmt_ctx, NULL);
        if (!out_stream) {
            fprintf(stderr, "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }

        // 直接将输入流的编解码参数拷贝到输出流中
        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            fprintf(stderr, "Failed to copy codec parameters\n");
            goto end;
        }
        out_stream->codecpar->codec_tag = 0;
    }

    // 打印要输出的多媒体文件的详细信息
    av_dump_format(ofmt_ctx, 0, out_filename, 1);

    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            fprintf(stderr, "Could not open output file '%s'", out_filename);
            goto end;
        }
    }

    // 写入新的多媒体文件的头
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        fprintf(stderr, "Error occurred when opening output file\n");
        goto end;
    }

    while (1) {
        AVStream *in_stream, *out_stream;

        // 循环读取每一帧数据
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0) // 读取完后退出循环
            break;

        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        if (pkt.stream_index >= stream_mapping_size ||
            stream_mapping[pkt.stream_index] < 0) {
            av_packet_unref(&pkt);
            continue;
        }

        pkt.stream_index = stream_mapping[pkt.stream_index]; // 按照输出流的index给pkt重新编号
        out_stream = ofmt_ctx->streams[pkt.stream_index]; // 根据pkt的stream_index获取对应的输出流

        // 对pts、dts、duration进行时间基转换，不同格式时间基都不一样，不转换会导致音视频同步问题
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base,
                                   static_cast<AVRounding>(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;

        // 将处理好的pkt写入输出文件
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
        if (ret < 0) {
            fprintf(stderr, "Error muxing packet\n");
            break;
        }
        av_packet_unref(&pkt);
    }

    // 写入新的多媒体文件尾
    av_write_trailer(ofmt_ctx);
    end:

    avformat_close_input(&ifmt_ctx);

    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_closep(&ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);

    av_freep(&stream_mapping);

    if (ret < 0 && ret != AVERROR_EOF) {
        fprintf(stderr, "Error occurred: %s\n", av_err2str(ret));
        return 1;
    }

    return 0;
}
