cmake_minimum_required(VERSION 3.21)
project(ffmpegClionC__)

set(CMAKE_CXX_STANDARD 14)


# FFmpeg的安装目录，可以通过命令"brew info ffmpeg"获取
set(FFMPEG_DIR /opt/homebrew/Cellar/ffmpeg/4.4_2)
# 头文件搜索路径
include_directories(${FFMPEG_DIR}/include/)
# 动态链接库或静态链接库的搜索路径
link_directories(${FFMPEG_DIR}/lib/)

add_executable(ffmpegClionC__
        main.cpp         # 抽帧程序
        cmdOverlay.cpp   # cmd 命令尝试


#        Log/LogStudy.cpp
#        FileList/fileListRead.cpp
#        captureAudio/captureAudio.cpp
        recoderAndEncoder/demo.cpp

        videoMetaData/metaInfo.cpp extractAudio/extractAudio.cpp adts/adts.cpp header/adts.h extractVideo/extractVideo.cpp extractVideo/demuxer_video.h encodeVideo/encodeVideo.cpp encodeVideo/convert.cpp encodeVideo/convert.h encodeVideo/demo.cpp extractVideo/extractMp4ToH264.cpp mp4toflv/mp4toflv.cpp)

target_link_libraries(
        ffmpegClionC__
        swscale
        swresample
        avcodec
        avutil
        avdevice
        avfilter
        avformat


#        avcodec
#        avdevice
#        avfilter
#        avformat
        )
