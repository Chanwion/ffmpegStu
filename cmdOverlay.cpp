//
// Created by 小汉陈 on 2022/6/6.
//
#include <iostream>

#include <string.h>
#include <cstdlib>

using namespace std;


int mainCmdOerlay() {
    string overlayImgPath = "/Users/xiaohanchen/Desktop/ffmpegClionC++/video_frame/scnu-overlay.png";
    string outPutFilePath =
            "/Users/xiaohanchen/Desktop/ffmpegClionC++/video_frame/outputVideo/torresOverlay.mp4";// _最终的输出文件的完整路径

    string inputFilePath = "/Users/xiaohanchen/Desktop/ffmpegClionC++/video_frame/videoData/torres.mp4";
    string command = "ffmpeg -i " + inputFilePath + " -i " + overlayImgPath + " -filter_complex \"overlay=5:5\" " + outPutFilePath;


    system(command.c_str());

}
