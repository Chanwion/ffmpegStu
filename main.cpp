#include <iostream>

extern "C"{
    //编码
    #include "libavcodec/avcodec.h"
    //封装格式处理
    #include "libavformat/avformat.h"
    #include "libavutil/imgutils.h"
    //像素处理
    #include "libswscale/swscale.h"
}

#define WORD uint16_t
#define DWORD uint32_t
#define LONG int32_t
#pragma pack(2)
typedef struct tagBITMAPFILEHEADER {
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;


typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;
    LONG  biWidth;
    LONG  biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    LONG  biXPelsPerMeter;
    LONG  biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

using namespace std;

int decodeWriteFrame(AVPacket *avPacket, AVFrame* avFrame, int *frameCount, int last,AVCodecContext *codecCtx);
void saveBmp(AVFrame *avFrame, char *imgName);


static SwsContext *swsContext = NULL;

int mainA() {
    const char* path = "/Users/xiaohanchen/Desktop/ffmpegClionC++/video_frame/videoData/torres.mp4";//记录视频源文件的路径，这里视频文件ds.mov直接放在项目工程里面了，所以可以直接用视频名称
    //如果视频不在项目工程里面，路径的书写格式举例：D:\\code of visual studio\\ffmpegTest\\ffmpegTest\\ds.mov
    //路径要使用“\\”，不然会被视为转义字符
    //注册各大组件
    av_register_all();

    //打开文件
    AVFormatContext *pContext = avformat_alloc_context();
    if(avformat_open_input(&pContext,path,NULL,NULL)<0){
        cout << "open failure " << endl;
        return -1;
    }

    // 检查文件的流媒体信息是否完善
    if(avformat_find_stream_info(pContext,NULL) < 0){
        cout << "Couldn't find stream info" << endl;
        return -1;
    }
    int videoChannelId = -1;
    //找到第一个媒体流信息，主要是根据类型来进行判断，跟mediacodec类似
    for(int channelPos=0; channelPos < pContext->nb_streams ; channelPos++){
        if(pContext->streams[channelPos]->codec->codec_type == AVMEDIA_TYPE_VIDEO){
            videoChannelId = channelPos;
            break;
        }
    }
    if(videoChannelId == -1){
        cout << "Could't find valid video channel" << endl;
        return -1;

    }
    // 找到编码器
    AVCodecContext *videoCodecContext = pContext->streams[videoChannelId]->codec;
    AVCodec *pCodec =avcodec_find_decoder(videoCodecContext->codec_id);
    if(pCodec == NULL){
        cout << "couldn't find video codec" << endl;
        return -1;
    }

    // 打开解码器
    if(avcodec_open2(videoCodecContext,pCodec,NULL) < 0){
        cout << "Could't open video codec "  << endl;
        return -1;
    }

    // 给帧数据分配空间
    AVFrame *pFrame = av_frame_alloc();
    AVFrame *pFrameRGB = av_frame_alloc();
    // 计算单帧数据的大小 malloc 分配的内存是不为空的，随机数据
    int numVideoFrameBytes = avpicture_get_size(AV_PIX_FMT_RGBA,
                                                videoCodecContext->width,videoCodecContext->height);
    unsigned char *buffer = (uint8_t *)av_malloc(numVideoFrameBytes * sizeof(unsigned char));
    avpicture_fill((AVPicture *) pFrameRGB, buffer, AV_PIX_FMT_RGBA,
                   videoCodecContext->width, videoCodecContext->height);
    //获取sws上下文
    swsContext = sws_getContext(videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
                                     videoCodecContext->width, videoCodecContext->height, AV_PIX_FMT_BGR24, SWS_BICUBIC, NULL, NULL, NULL);
    cout << "========================" << endl;

    if(pFrame == NULL || pFrameRGB == NULL){
        cout << "Couldn't allocate frame";
        return -1;
    }

    AVPacket packet;
    int frameCount = 1;
    AVFrame *avFrame = av_frame_alloc();
    while(true){
        int ret = av_read_frame(pContext,&packet);
        if(packet.stream_index == videoChannelId){
            if(ret >= 0){
                decodeWriteFrame(&packet, avFrame, &frameCount, 0,videoCodecContext);
            }else{
                cout << "read frame failure " << ret << endl;
                break;
            }
            frameCount++;
        }
        av_packet_unref(&packet);
    }
    cout << "here we go" << endl ;
    return 0;

}


void SaveFrame(AVFrame* pFrame,int width,int height,int index) {
    FILE *pFile;
    char szFilename[100];
    int y;
    // /Users/xiaohanchen/Desktop/ffmpegClionC++/imgFrame
    sprintf(szFilename, "/Users/xiaohanchen/Desktop/ffmpegClionC++/imgFrame/frame%d.jpg", index);
    pFile = fopen(szFilename, "wb");

    if (pFile == NULL) {
        return;
    }
    fprintf(pFile, "P6%d %d 255", width, height);

    for (y = 0; y < height; y++) {
        fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);
    }
    fclose(pFile);

}


int SaveAsJPEG(AVFrame* pFrame, int width, int height, int index)
{
    // 输出文件路径
    char out_file[100];
    sprintf(out_file, "/Users/xiaohanchen/Desktop/ffmpegClionC++/new/frame%d.jpg", index);
//    char *out_file = "/Users/xiaohanchen/Desktop/ffmpegClionC++/new/lastPic.jpg";
//    sprintf_s(out_file, sizeof(out_file), "%s%d.jpg", "E:/QT/test_ffmpegSavePic/ffmpeg/output/", index);

    // 分配AVFormatContext对象
    AVFormatContext* pFormatCtx = avformat_alloc_context();

    // 设置输出文件格式
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);

    // 创建并初始化一个和该url相关的AVIOContext
    if( avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0)
    {
        printf("Couldn't open output file.");
        return -1;
    }

    // 构建一个新stream
    AVStream* pAVStream = avformat_new_stream(pFormatCtx, 0);
    if( pAVStream == NULL )
    {
        return -1;
    }

    // 设置该stream的信息
    AVCodecContext* pCodecCtx = pAVStream->codec;

    pCodecCtx->codec_id   = pFormatCtx->oformat->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt    = AV_PIX_FMT_YUVJ420P;
    pCodecCtx->width      = width;
    pCodecCtx->height     = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;

    //打印输出相关信息
    av_dump_format(pFormatCtx, 0, out_file, 1);

    //================================== 查找编码器 ==================================//
    AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if( !pCodec )
    {
        printf("Codec not found.");
        return -1;
    }

    // 设置pCodecCtx的解码器为pCodec
    if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 )
    {
        printf("Could not open codec.");
        return -1;
    }

    //================================Write Header ===============================//
    avformat_write_header(pFormatCtx, NULL);

    int y_size = pCodecCtx->width * pCodecCtx->height;

    //==================================== 编码 ==================================//
    // 给AVPacket分配足够大的空间
    AVPacket pkt;
    av_new_packet(&pkt, y_size * 3);

    //
    int got_picture = 0;
    int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
    if( ret < 0 )
    {
        printf("Encode Error.\n");
        return -1;
    }
    if( got_picture == 1 )
    {
        pkt.stream_index = pAVStream->index;
        ret = av_write_frame(pFormatCtx, &pkt);
    }

    av_free_packet(&pkt);

    //Write Trailer
    av_write_trailer(pFormatCtx);


    if( pAVStream )
    {
        avcodec_close(pAVStream->codec);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);

    return 0;
}


const char *outName = "/Users/xiaohanchen/Desktop/ffmpegClionC++/imgFrames/";
// 解码AVPacket成avframe，avframe一般是原始数据的帧，包含音视频，pcm以及yuv rgb
int decodeWriteFrame(AVPacket *avPacket, AVFrame* avFrame, int *frameCount, int last,AVCodecContext *codecCtx){  //AVCodecContext *codecCtx
    int isFinishDecoder = 0;  //avcodec_decode_video2
    char errors[200] = {0};
    char buffer[200] = {0};
    int len = avcodec_decode_video2(codecCtx,avFrame,&isFinishDecoder,avPacket);
    if(len < 0){
        av_strerror(len, errors, 200);
        av_log(NULL, AV_LOG_WARNING, "avcodec_decode_video2 error: ret=%d, msg=%s, frame=%d\n", len, errors, frameCount);
        return len;
    }

    // 解码得到avframe成功
    cout << "解码成功 " << isFinishDecoder << endl;
    cout << "dts   " <<  avFrame->pkt_dts  << endl;
    cout << "pts   " <<  avFrame->pkt_pts  << endl;
    if(isFinishDecoder){
        if(avPacket->pos % 100 == 0){
            snprintf(buffer, 200, "%s_%d.bmp", outName, *frameCount);
            saveBmp(avFrame, buffer);
        }

    }
}

// 保存bmp，bmp有自己的格式，按照格式填充参数
void saveBmp(AVFrame *avFrame, char *imgName)
{
    int w = avFrame->width;
    int h = avFrame->height;
    int size = avpicture_get_size(AV_PIX_FMT_BGR24, w, h);
    uint8_t *buffer = (uint8_t*)av_malloc(size * sizeof(uint8_t));
    AVFrame *frameRgb = av_frame_alloc();
    avpicture_fill((AVPicture*)frameRgb, buffer, AV_PIX_FMT_BGR24, w, h);
    sws_scale(swsContext, avFrame->data, avFrame->linesize, 0, h, frameRgb->data, frameRgb->linesize);

    //2 构造 BITMAPINFOHEADER 位图信息头
    BITMAPINFOHEADER header;
    header.biSize = sizeof(BITMAPINFOHEADER);
    header.biWidth = w;
    header.biHeight = h * (-1);
    header.biBitCount = 24;
    header.biCompression = 0;
    header.biSizeImage = 0;
    header.biClrImportant = 0;
    header.biClrUsed = 0;
    header.biXPelsPerMeter = 0;
    header.biYPelsPerMeter = 0;
    header.biPlanes = 1;
    //3 构造文件头
    BITMAPFILEHEADER bmpFileHeader = { 0, };
    //HANDLE hFile = NULL;
    DWORD dwTotalWriten = 0;
    //DWORD dwWriten;

    bmpFileHeader.bfType = 0x4d42; //'BM';
    bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + size;
    bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    FILE *pFile = fopen(imgName, "wb");
    fwrite(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, pFile);
    fwrite(&header, sizeof(BITMAPINFOHEADER), 1, pFile);
    fwrite(frameRgb->data[0], 1, size, pFile);
    fclose(pFile);
    av_freep(&frameRgb);
    av_free(frameRgb);
}




