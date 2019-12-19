#include <stdio.h>
#include <SDL.h>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif


int main(int argc, char *argv[])
{
	AVFormatContext *pFormatCtx = NULL;
	AVCodecContext *pCodecCtx = NULL;
	AVCodec *pCodec = NULL;
	AVFrame *pFrame = NULL;
	AVPicture *pict;
	AVPacket pkt;
	struct SwsContext *sws_ctx = NULL;

	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;
	SDL_Rect rect;

	int ret = -1, video_index = -1, got_picture = 0;
	int w_width = 0, w_height = 0;

	if (argc < 2) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Usage: command <file>");
		return ret;
	}

	av_register_all();
	
	if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open video file!");
		goto __FAIL;
	}

	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to find stream infomation!");
		goto __FAIL;
	}

	for (int i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_index = i;
			break;
		}
	}

	if (video_index == -1) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Din't find a video stream!");
		goto __FAIL;
	}

	pCodecCtx = pFormatCtx->streams[video_index]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (!pCodec) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Unsupported codec!\n");
		goto __FAIL;
	}

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to open decoder!\n");
		goto __FAIL;
	}

	pFrame = av_frame_alloc();
	
	w_width = pCodecCtx->width;
	w_height = pCodecCtx->height;	
	
	window = SDL_CreateWindow("Media palyer",
				  SDL_WINDOWPOS_UNDEFINED,
				  SDL_WINDOWPOS_UNDEFINED,
				  w_width, w_height,
				  SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (window == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window by SDL");
		goto __FAIL;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (renderer == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create Renderer by SDL");  
		goto __FAIL;
	}

	texture = SDL_CreateTexture(renderer,
				    SDL_PIXELFORMAT_IYUV,
				    SDL_TEXTUREACCESS_STREAMING,
				    pCodecCtx->width,
				    pCodecCtx->height);
	if (texture == NULL) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to create Texture by SDL"); 
		goto __FAIL; 
	}

	sws_ctx = sws_getContext(pCodecCtx->width,
				 pCodecCtx->height,
				 pCodecCtx->pix_fmt,
				 pCodecCtx->width,
				 pCodecCtx->height,
				 AV_PIX_FMT_YUV420P,
				 SWS_BILINEAR,
				 NULL, 
				 NULL,
				 NULL);

	pict = (AVPicture *)malloc(sizeof(AVPicture));
	avpicture_alloc(pict, 
			AV_PIX_FMT_YUV420P,
			pCodecCtx->width,
			pCodecCtx->height);

	while (av_read_frame(pFormatCtx, &pkt) >= 0) {
		if (pkt.stream_index == video_index) {
			avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &pkt);
			
			if (got_picture) {
				sws_scale(sws_ctx,
					  (const uint8_t * const)pFrame->data,
					  pFrame->linesize,
					  0,
					  pCodecCtx->height,
					  pict->data,
					  pict->linesize);
				SDL_UpdateYUVTexture(texture, NULL,
						     pict->data[0], pict->linesize[0],
						     pict->data[1], pict->linesize[1],
						     pict->data[2], pict->linesize[2]);

				rect.x = 0;
				rect.y = 0;
				rect.w = pCodecCtx->width;
				rect.h = pCodecCtx->height;

				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, texture, NULL, &rect);
				SDL_RenderPresent(renderer);
				
				SDL_Delay(40);
			}
		}

		av_free_packet(&pkt);


		SDL_Event event;
		SDL_PollEvent(&event);
		switch (event.type) {
			case SDL_QUIT: 
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

	if (pCodecCtx) {
		avcodec_close(pCodecCtx);
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
		
	
}
