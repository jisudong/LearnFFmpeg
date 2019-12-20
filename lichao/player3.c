#include <stdio.h>
#include <assert.h>
#include <SDL.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000

struct SwrContext *audio_convert_ctx = NULL;

typedef struct PacketQueue {
  AVPacketList *first_pkt, *last_pkt;
  int nb_packets;
  int size;
  SDL_mutex *mutex;
  SDL_cond *cond;
} PacketQueue;

PacketQueue audioq;

int quit = 0;

void packet_queue_init(PacketQueue *q) {
  memset(q, 0, sizeof(PacketQueue));
  q->mutex = SDL_CreateMutex();
  q->cond = SDL_CreateCond();
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {

  AVPacketList *pkt1;
  if(av_dup_packet(pkt) < 0) {
    return -1;
  }
  pkt1 = av_malloc(sizeof(AVPacketList));
  if (!pkt1)
    return -1;
  pkt1->pkt = *pkt;
  pkt1->next = NULL;
  
  SDL_LockMutex(q->mutex);
  
  if (!q->last_pkt) {
    q->first_pkt = pkt1;
  }else{
    q->last_pkt->next = pkt1;
  }

  q->last_pkt = pkt1;
  q->nb_packets++;
  q->size += pkt1->pkt.size;
  SDL_CondSignal(q->cond);
  
  SDL_UnlockMutex(q->mutex);
  return 0;
}

int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block)
{
  AVPacketList *pkt1;
  int ret;
  
  SDL_LockMutex(q->mutex);
  
  for(;;) {
    
    if(quit) {
      ret = -1;
      break;
    }

    pkt1 = q->first_pkt;
    if (pkt1) {
      q->first_pkt = pkt1->next;
      if (!q->first_pkt)
          q->last_pkt = NULL;
      q->nb_packets--;
      q->size -= pkt1->pkt.size;
      *pkt = pkt1->pkt;
      av_free(pkt1);
      ret = 1;
      break;
    } else if (!block) {
      ret = 0;
      break;
    } else {
      SDL_CondWait(q->cond, q->mutex);
    }
  }
  SDL_UnlockMutex(q->mutex);
  return ret;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {

    static AVPacket pkt;
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    static AVFrame frame;

    int len1, data_size = 0;

    for(;;) {
        while(audio_pkt_size > 0) {
            int got_frame = 0;
            len1 = avcodec_decode_audio4(aCodecCtx, &frame, &got_frame, &pkt);
            if(len1 < 0) {
                /* if error, skip frame */
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            data_size = 0;
            if(got_frame) {
                //fprintf(stderr, "channels:%d, nb_samples:%d, sample_fmt:%d \n", aCodecCtx->channels, frame.nb_samples, aCodecCtx->sample_fmt);
                /*
            data_size = av_samples_get_buffer_size(NULL,
                                   aCodecCtx->channels,
                                   frame.nb_samples,
                                   aCodecCtx->sample_fmt,
                                   1);
            */
                data_size = 2 * 2 * frame.nb_samples;
 
                assert(data_size <= buf_size);
                swr_convert(audio_convert_ctx,
                            &audio_buf,
                            MAX_AUDIO_FRAME_SIZE*3/2,
                            (const uint8_t **)frame.data,
                            frame.nb_samples);

              //memcpy(audio_buf, frame.data[0], data_size);
            }
            if(data_size <= 0) {
                /* No data yet, get more frames */
                continue;
            }
            /* We have data, return it and come back for more later */
            return data_size;
        }
        if(pkt.data)
            av_free_packet(&pkt);

        if(quit) {
            return -1;
        }

        if(packet_queue_get(&audioq, &pkt, 1) < 0) {
            return -1;
        }
        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
    }
}

void audio_callback(void *userdata, Uint8 *stream, int len) {

    AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
    int len1, audio_size;

    static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;

    while(len > 0) {
        if(audio_buf_index >= audio_buf_size) {
            /* We have already sent all our data; get more */
            audio_size = audio_decode_frame(aCodecCtx, audio_buf, sizeof(audio_buf));
            if(audio_size < 0) {
                /* If error, output silence */
                audio_buf_size = 1024; // arbitrary?
                memset(audio_buf, 0, audio_buf_size);
            } else {
                audio_buf_size = audio_size;
            }
            audio_buf_index = 0;
        }
        len1 = audio_buf_size - audio_buf_index;
        if(len1 > len)
            len1 = len;
        fprintf(stderr, "index=%d, len1=%d, len=%d\n",audio_buf_index, len, len1);
        memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
}

int main(int argc, char *argv[])
{
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext *vCodecCtx = NULL;
	AVCodecContext *aCodecCtx = NULL;
	AVCodec *vCodec = NULL;
	AVCodec *aCodec = NULL;
	AVFrame *pFrame = NULL;
	AVPicture *pict;
	AVPacket pkt;
	struct SwsContext *sws_ctx = NULL;

	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;
	SDL_Rect rect;

	SDL_AudioSpec wanted_spec, spec;
	
	int64_t in_channel_layout;
	int64_t out_channel_layout;

	int video_index = -1;
	int audio_index = -1;
	int got_picture = 0;
	int window_w = 640, window_h = 480;
    int ret = -1;

	if (argc < 2) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Usage: command <file>");
		return -1;
	}

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL - %s\n", SDL_GetError());
		return -1;
	}

	av_register_all();

	if (avformat_open_input(&pFormatCtx, argv[1], NULL,  NULL) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open multi-media file");
		goto __FAIL;
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't find stream information ");
		goto __FAIL;
	}

	av_dump_format(pFormatCtx, 0, argv[1], 0);

	for (int i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO && video_index < 0) {
			video_index = i;
		}
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO && audio_index < 0) {
			audio_index = i;
		}
	}

	if (video_index == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, " Didn't find a video stream ");
		goto __FAIL;
	}
	if (audio_index == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, " Didn't find a audio stream ");
		goto __FAIL;
	}

	// 处理音频
	aCodecCtx = pFormatCtx->streams[audio_index]->codec;
	aCodec = avcodec_find_decoder(aCodecCtx->codec_id);
	if (aCodec == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported codec! ");
		goto __FAIL;
	}

	wanted_spec.freq = aCodecCtx->sample_rate;
	wanted_spec.format = AUDIO_S16SYS;
	wanted_spec.channels = aCodecCtx->channels;
	wanted_spec.silence = 0;
	wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
	wanted_spec.callback = audio_callback;
	wanted_spec.userdata = aCodecCtx;

	if (SDL_OpenAudio(&wanted_spec, &spec) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open audio device - %s!", SDL_GetError());
		goto __FAIL;
	}

	avcodec_open2(aCodecCtx, aCodec, NULL);

	packet_queue_init(&audioq);

	in_channel_layout = av_get_default_channel_layout(aCodecCtx->channels);
	out_channel_layout = in_channel_layout;
	fprintf(stderr, "in layout:%lld, out layout:%lld \n", in_channel_layout, out_channel_layout);

	audio_convert_ctx = swr_alloc();
	if (audio_convert_ctx) {
		swr_alloc_set_opts(audio_convert_ctx,
				   out_channel_layout,
				   AV_SAMPLE_FMT_S16,
				   aCodecCtx->sample_rate,
				   in_channel_layout,
				   aCodecCtx->sample_fmt,
				   aCodecCtx->sample_rate,
				   0,
				   NULL);
	}

	swr_init(audio_convert_ctx);

	SDL_PauseAudio(0);


	vCodecCtx = pFormatCtx->streams[video_index]->codec;
	vCodec = avcodec_find_decoder(vCodecCtx->codec_id);
	if (vCodec == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported codec! ");
		goto __FAIL;
	}
    
    if (avcodec_open2(vCodecCtx, vCodec, NULL) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open codec! ");
        goto __FAIL;
    }
    
    pFrame = av_frame_alloc();

    window_w = vCodecCtx->width;
    window_h = vCodecCtx->height;
    window = SDL_CreateWindow("Media player",
                              SDL_WINDOWPOS_UNDEFINED,
                              SDL_WINDOWPOS_UNDEFINED,
                              window_w,
                              window_h,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window!");
        goto __FAIL;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (renderer == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create renderer!");
        goto __FAIL;
    }

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_IYUV,
                                SDL_TEXTUREACCESS_STREAMING,
                                vCodecCtx->width,
                                vCodecCtx->height);
    if (texture == NULL) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create Texture!");
        goto __FAIL;
    }

	
	pict = (AVPicture *)malloc(sizeof(AVPicture));
	avpicture_alloc(pict,
                    AV_PIX_FMT_YUV420P,
                    vCodecCtx->width,
                    vCodecCtx->height);
	
	sws_ctx = sws_getContext(vCodecCtx->width,
                             vCodecCtx->height,
                             vCodecCtx->pix_fmt,
                             vCodecCtx->width,
                             vCodecCtx->height,
                             AV_PIX_FMT_YUV420P,
                             SWS_BILINEAR,
                             NULL,
                             NULL,
                             NULL);

	while (av_read_frame(pFormatCtx, &pkt) >= 0) {
        if (pkt.stream_index == video_index) {
            avcodec_decode_video2(vCodecCtx, pFrame, &got_picture, &pkt);
            
            if (got_picture) {
                sws_scale(sws_ctx,
                      (const uint8_t *const)pFrame->data,
                      pFrame->linesize,
                      0,
                      vCodecCtx->height,
                      pict->data,
                      pict->linesize);

                SDL_UpdateYUVTexture(texture, NULL,
                             pict->data[0], pict->linesize[0],
                             pict->data[1], pict->linesize[1],
                             pict->data[2], pict->linesize[2]);

                rect.x = 0;
                rect.y = 0;
                rect.w = vCodecCtx->width;
                rect.h = vCodecCtx->height;

                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, NULL, &rect);
                SDL_RenderPresent(renderer);

            }
        
                    av_free_packet(&pkt);
        } else if (pkt.stream_index == audio_index) {
            packet_queue_put(&audioq, &pkt);
        } else {
            av_free_packet(&pkt);
        }
        
        SDL_Event event;
        SDL_PollEvent(&event);
       switch (event.type) {
            case SDL_QUIT:
                quit = 1;
                goto __QUIT;
                break;
            default:
                break;
        }
	} 
  
__QUIT:
    ret = 0;
    
__FAIL:
	if (pict) {
        avpicture_free(pict);
        free(pict);
    }

    if (pFrame) {
        av_frame_free(&pFrame);
    }

    if (vCodecCtx) {
        avcodec_close(vCodecCtx);
    }
    
    if (aCodecCtx) {
        avcodec_close(aCodecCtx);
    }

    if (pFormatCtx) {
        avformat_close_input(&pFormatCtx);
    }
    
    
    if (texture) {
        SDL_DestroyTexture(texture);
    }

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }

    SDL_Quit();
    
    return 0;
}
