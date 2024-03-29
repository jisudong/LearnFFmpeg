//
//  main.cpp
//  ffmpeg
//
//  Created by 秀健身 on 2019/9/5.
//  Copyright © 2019 秀健身. All rights reserved.
//

#include <iostream>
#include "simplest_mediadata_raw.hpp"
#include "simplest_mediadata_h264.hpp"
#include "simplest_mediadata_aac.hpp"
#include "simplest_mediadata_flv.hpp"
#include "simplest_ffmpeg_player.hpp"
#include "simplest_ffmpeg_player_su.hpp"
#include "simplest_ffmpeg_decoder_pure.hpp"
#include "simplest_ffmpeg_decoder.hpp"
#include "simplest_video_play_sdl2.hpp"
#include "simplest_ffmpeg_helloworld.hpp"
#include "simplest_ffmpeg_audio_player.hpp"
#include "simplest_ffmpeg_picture_encoder.hpp"


int main(int argc, const char * argv[]) {
    // insert code here...
    
//    simplest_yuv420_split("lena_256x256_yuv420p.yuv", 256, 256, 1);
    
//    simplest_yuv444_split("lena_256x256_yuv444p.yuv", 256, 256, 1);
    
//    simplest_yuv420_gray("lena_256x256_yuv420p.yuv", 256, 256, 1);
    
//    simplest_yuv420_halfy("lena_256x256_yuv420p.yuv", 256, 256, 1);
    
//    simplest_yuv420_border("lena_256x256_yuv420p.yuv", 256, 256, 20, 1);
    
//    simplest_yuv420_graybar(640, 360, 0, 255, 10, "graybar_640x360.yuv");
    
//    simplest_yuv420_psnr("lena_256x256_yuv420p.yuv", "lena_distort_256x256_yuv420p.yuv", 256, 256, 1);
    
//    simplest_rgb24_split("cie1931_500x500.rgb", 500, 500, 1);
    
//    simplest_rgb24_to_bmp("lena_256x256_rgb24.rgb", 256, 256, "output_lena.bmp");
    
//    simplest_rgb24_to_yuv420("lena_256x256_rgb24.rgb", 256, 256, 1, "output_lena.yuv");
    
//    simplest_rgb24_colorbar(640, 360, "colorbar_640x360.rgb");
    
//    simplest_pcm16le_split("NocturneNo2inEflat_44.1k_s16le.pcm");
    
//    simplest_pcm16le_halfvolumeleft("NocturneNo2inEflat_44.1k_s16le.pcm");
    
//    simplest_pcm16le_doublespeed("NocturneNo2inEflat_44.1k_s16le.pcm");
    
//    simplest_pcm16le_to_pcm8("NocturneNo2inEflat_44.1k_s16le.pcm");
    
//    simplest_pcm16le_cut_singlechannel("drum.pcm", 2360, 120);
    
//    simplest_pcm16le_to_wave("NocturneNo2inEflat_44.1k_s16le.pcm", 2, 44100, "output_nocturne.wav");
    
//    simplest_h264_parser("sintel.h264");
//    
//    simplest_aac_parser("nocturne.aac");
    
//    simplest_flv_parser("cuc_ieschool.flv");
    
//    ffmpeg_player();
    
//    ffmpeg_player_su();
    
//    ffmpeg_decoder_pure();
    
//    ffmpeg_decoder();
    
//    sdl2_play();
    
//    printFFmpegInfo();
    
//    audio_player();
    
    encoder_picture();
    
}


