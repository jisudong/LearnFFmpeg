//
//  simplest_ffmpeg_audio_player.cpp
//  ffmpeg
//
//  Created by apple on 2019/12/24.
//  Copyright © 2019 apple. All rights reserved.
//

#include "simplest_ffmpeg_audio_player.hpp"

extern "C" {
#include <SDL2/SDL.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}


#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio

//Output PCM
#define OUTPUT_PCM 1
//Use SDL
#define USE_SDL 1

//Buffer:
//|-----------|-------------|
//chunk-------pos---len-----|
static  Uint8  *audio_chunk;
static  Uint32  audio_len;
static  Uint8  *audio_pos;


void audio_callback(void *udata, Uint8 *stream, int len) {
    
    SDL_memset(stream, 0, len);
    if (audio_len == 0) {
        return;
    }
    len = len > audio_len ? audio_len : len;
    SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
    audio_pos += len;
    audio_len -= len;
}


int audio_player() {
    
    AVFormatContext *pFormatCtx = NULL;
    AVCodecContext *pCodecCtx = NULL;
    AVCodec *pCodec = NULL;
    AVFrame *pFrame = NULL;
    AVPacket *pkt;
    SwrContext *audio_convert_ctx = NULL;
    
    SDL_AudioSpec wanted_spec, spec;
    
    int audioStream = -1;
    int got_frame = 0;
    int index = 0;
    int64_t in_channel_layout;
    uint8_t *out_buffer;
    
    char url[] = "Titanic.mkv";
    
#if OUTPUT_PCM
    FILE *pFile = NULL;
#endif
    
    av_register_all();
    avformat_network_init();
    pFormatCtx = avformat_alloc_context();
    
    if (avformat_open_input(&pFormatCtx, url, NULL, NULL) != 0) {
        return -1;
    }
    
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        return -1;
    }
    
    av_dump_format(pFormatCtx, 0, url, 0);
    
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
            audioStream = i;
            break;
        }
    }
    
    if (audioStream == -1) {
        return -1;
    }
    
    pCodecCtx = pFormatCtx->streams[audioStream]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    
    if (pCodec == NULL) {
        return -1;
    }
    
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        return -1;
    }
    
#if OUTPUT_PCM
    pFile = fopen("output.pcm", "wb");
#endif
    
    pkt = (AVPacket *)malloc(sizeof(AVPacket));
    av_init_packet(pkt);
    
    uint64_t out_channel_layout = AV_CH_LAYOUT_STEREO;
    int out_nb_samples = pCodecCtx->frame_size;
    AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
    int out_sample_rate = 44100;
    int out_channels = av_get_channel_layout_nb_channels(out_channel_layout);
    int out_buffer_size = av_samples_get_buffer_size(NULL,
                                                     out_channels,
                                                     out_nb_samples,
                                                     out_sample_fmt,
                                                     1);
    out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE * 2);
    
    pFrame = av_frame_alloc();
    
#if USE_SDL
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_TIMER)) {
        return -1;
    }
    
    wanted_spec.freq = out_sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = out_channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = out_nb_samples;
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = NULL;
    
    if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
        return -1;
    }
    
#endif
    
    in_channel_layout = av_get_default_channel_layout(pCodecCtx->channels);
    
    audio_convert_ctx = swr_alloc();
    if (audio_convert_ctx) {
        swr_alloc_set_opts(audio_convert_ctx,
                           out_channel_layout,
                           out_sample_fmt,
                           out_sample_rate,
                           in_channel_layout,
                           pCodecCtx->sample_fmt,
                           pCodecCtx->sample_rate,
                           0,
                           NULL);
    }
    swr_init(audio_convert_ctx);
    
    SDL_PauseAudio(0);
    
    while (av_read_frame(pFormatCtx, pkt) >= 0) {
        if (pkt->stream_index == audioStream) {
            avcodec_decode_audio4(pCodecCtx, pFrame, &got_frame, pkt);
            if (got_frame) {
                swr_convert(audio_convert_ctx, &out_buffer, MAX_AUDIO_FRAME_SIZE, (const uint8_t **)pFrame->data, pFrame->nb_samples);
                
                printf("index:%5d\t pts:%lld\t packet size:%d\n", index, pkt->pts,pkt->size);
                
#if OUTPUT_PCM
                fwrite(out_buffer, 1, out_buffer_size, pFile);
#endif
                index++;
            }
#if USE_SDL
            while (audio_len > 0) {
                SDL_Delay(1);
            }
            audio_chunk = (Uint8 *)out_buffer;
            audio_len = out_buffer_size;
            audio_pos = audio_chunk;
#endif
            
        }
        av_free_packet(pkt);
    }
    
    swr_free(&audio_convert_ctx);
    
#if USE_SDL
    SDL_CloseAudio();
    SDL_Quit();
#endif
    
#if OUTPUT_PCM
    fclose(pFile);
#endif
    
    av_free(out_buffer);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}
