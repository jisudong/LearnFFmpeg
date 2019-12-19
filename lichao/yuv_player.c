#include <stdio.h>
#include <string.h>

#include <SDL.h>

const int bpp = 12;
int screen_w = 500, screen_h = 500;

#define BLOCK_SIZE 4096000

#define REFRESH_EVENT (SDL_USEREVENT + 1)
#define QUIT_EVENT (SDL_USEREVENT + 2)

int thread_exit = 0;

int refresh_video_timer(void *udata) {

	thread_exit = 0;

	while (!thread_exit) {
		SDL_Event event;
		event.type = REFRESH_EVENT;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}

	thread_exit = 0;
	
	SDL_Event event;
	event.type = QUIT_EVENT;
	SDL_PushEvent(&event);

	return 0;
}

int main(int argc, char *argv[])
{
	FILE *video_fd = NULL;
	SDL_Event event;
	SDL_Rect rect;

	Uint32 pixformat = 0;

	SDL_Window *win = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Texture *texture = NULL;

	SDL_Thread *timer_thread = NULL;

	int w_width = 320, w_height = 180;
	const int video_width = 320, video_height = 180;

	char video_buf[video_width * video_height * bpp / 8];

	const char *path = "test_yuv420p_320x180.yuv";

	if (SDL_Init(SDL_INIT_VIDEO)) {
		fprintf( stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        	return -1;
	}

	win = SDL_CreateWindow("YUV Player", 
				SDL_WINDOWPOS_UNDEFINED, 
				SDL_WINDOWPOS_UNDEFINED, 
				w_width, w_height, 
				SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (!win) {
		fprintf(stderr, "Failed to create window, %s\n",SDL_GetError());
        	goto __FAIL;
	}

	renderer = SDL_CreateRenderer(win, -1, 0);

	pixformat = SDL_PIXELFORMAT_IYUV;

	texture = SDL_CreateTexture(renderer, 
				    pixformat,
				    SDL_TEXTUREACCESS_STREAMING,
				    video_width,
				    video_height);

	video_fd = fopen(path, "rb+");
	if (!video_fd) {
		fprintf(stderr, "Failed to open yuv file\n");
        goto __FAIL;
	}

	timer_thread = SDL_CreateThread(refresh_video_timer, NULL, NULL);
	while (1) {
		SDL_WaitEvent(&event);
		if (event.type == REFRESH_EVENT) {

			if (fread(video_buf, 1, video_width *video_height * bpp / 8, video_fd) != video_width *video_height * bpp / 8) {
				fseek(video_fd, 0, SEEK_SET);
				fread(video_buf, 1, video_width * video_height *bpp / 8, video_fd);
			}

			SDL_UpdateTexture(texture, NULL,video_buf , video_width);

			rect.x = 0;
			rect.y = 0;
			rect.w = w_width;
			rect.h = w_height;

			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, &rect);
			SDL_RenderPresent(renderer);
		} else if (event.type == SDL_WINDOWEVENT) {
			SDL_GetWindowSize(win, &w_width, &w_height);
		} else if (event.type == SDL_QUIT) {
			thread_exit = 1;
		} else if (event.type == QUIT_EVENT) {
			break;
		}
	}

__FAIL:
	if (video_fd) {
		fclose(video_fd);
	}
	
	SDL_Quit();

	return 0;
}
