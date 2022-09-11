//
// Created by 小汉陈 on 2022/9/8.
//

#ifndef FFMPEGCLIONC___ADTS_H
#define FFMPEGCLIONC___ADTS_H


int adts_header(char * const p_adts_header, const int data_length,
                const int profile, const int samplerate,
                const int channels);
#endif //FFMPEGCLIONC___ADTS_H
