#include <stdio.h>
#include <libavutil/log.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

#define INBUF_SIZE 4096

#define WORD uint16_t
#define DWORD uint32_t
#define LONG int32_t

#pragma pack(2)
typedef struct tagBitmapFileHeader {
	WORD bfType;		//位图文件的类型，必须为BM
	DWORD bfSize;		//文件大小，以字节为单位	
	WORD bfReserved1;	//位图文件保留字，必须为0
	WORD bfReserved2;	//位图文件保留字，必须为0
	DWORD bfOffBits;	//位图文件头到数据的偏移量，以字节为单位
} BitmapFileHeader, *pBitmapFileHeader;

typedef struct tagBitmapInfoHeader {
	DWORD biSize;		//该结构大小，字节为单位
	LONG  biWidth;		//图形宽度以象素为单位
	LONG  biHeight;		//图形高度以象素为单位
	WORD  biPlanes;		//目标设备的级别，必须为1
	WORD  biBitCount;	//颜色深度，每个象素所需要的位数
	DWORD biCompression;	//位图的压缩类型
	DWORD biSizeImage;	//位图的大小，以字节为单位
	LONG  biXPelsPerMeter;	//位图水平分辨率，每米像素数
	LONG  biYPelsPerMeter;	//位图垂直分辨率，每米像素数
	DWORD biClrUsed;	//位图实际使用的颜色表中的颜色数
	DWORD biClrImportant;	//位图显示过程中重要的颜色数
} BitmapInfoHeader, *pBitmapInfoHeader;


void saveBMP(struct SwsContext *img_convert_ctx, AVFrame *frame, char *filename) 
{
	int w = frame->width;
	int h = frame->height;

	int numBytes = avpicture_get_size(AV_PIX_FMT_RGB24, w, h);
	uint8_t *buffer = (uint8_t *)av_malloc(numBytes * sizeof(uint8_t));

	AVFrame *pFrameRGB = av_frame_alloc();

	avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, w, h);

	sws_scale(img_convert_ctx, frame->data, frame->linesize, 0, h, pFrameRGB->data, pFrameRGB->linesize);

	BitmapInfoHeader header;
	header.biSize = sizeof(BitmapInfoHeader);
	header.biWidth = w;
	header.biHeight = -h;
	header.biBitCount = 24;
	header.biCompression = 0;
	header.biSizeImage = 0;
	header.biClrImportant = 0;
	header.biClrUsed = 0;
	header.biXPelsPerMeter = 0;
	header.biYPelsPerMeter = 0;
	header.biPlanes = 1;


	BitmapFileHeader bmpFileHeader = {0,};

	DWORD dwTotalWriten = 0;
	DWORD dwWriten;

	bmpFileHeader.bfType = 0x4d42; //'BM'
	bmpFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + numBytes;
	bmpFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader);

	FILE *pf = fopen(filename, "wb");
	if (!pf) {
		return;
	}

	fwrite(&bmpFileHeader, sizeof(BitmapFileHeader), 1, pf);
	fwrite(&header, sizeof(BitmapInfoHeader), 1, pf);
	fwrite(pFrameRGB->data[0], 1, numBytes, pf);

	fclose(pf);
	av_freep(&pFrameRGB[0]);
	av_free(pFrameRGB);
}

static int decode_write_frame(	const char *outfilename, 
			      	AVCodecContext *avctx,
				struct SwsContext *img_convert_ctx, 
				AVFrame *frame,
				int *frame_count, 
				AVPacket *pkt,
				int last)
{
	int len, got_frame;
	char buf[1024];

	len = avcodec_decode_video2(avctx, frame, &got_frame, pkt);
	if (len < 0) {
		fprintf(stderr, "Error while decoding frame %d\n", *frame_count);
		return len;
	}

	if (got_frame) {
		printf("Saving %sframe %3d\n", last ? "last " : "", *frame_count);
		fflush(stdout);

		snprintf(buf, sizeof(buf), "%s-%d.bmp", outfilename, *frame_count);

		saveBMP(img_convert_ctx, frame, buf);

		(*frame_count)++;
	}

	if (pkt->data) {
		pkt->size -= len;
		pkt->data += len;
	}

	return 0;
} 


int main(int argc, char **argv)
{
	int ret;
	FILE *f;
	const char *filename, *outfilename;
	
	AVFormatContext *fmt_ctx = NULL;

	const AVCodec *codec = NULL;
	AVCodecContext *c = NULL;

	AVStream *stream = NULL;
	int stream_index;

	int frame_count;
	AVFrame *frame;

	struct SwsContext *img_convert_ctx;

	AVPacket avpkt;

	if (argc <= 2) {
		fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        	exit(0);
	}
	filename    = argv[1];
   	outfilename = argv[2];

	av_register_all();

	if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
		fprintf(stderr, "Could not open source file %s\n", filename);
        	exit(1);
	}

	if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		fprintf(stderr, "Could not find stream information\n");
        	exit(1);
	}

	av_dump_format(fmt_ctx, 0, filename, 0);

	av_init_packet(&avpkt);

	ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                	av_get_media_type_string(AVMEDIA_TYPE_VIDEO), filename);
        	return ret;
	}

	stream_index = ret;
	stream = fmt_ctx->streams[stream_index];

	codec = avcodec_find_decoder(stream->codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "Failed to find %s codec\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        	return AVERROR(EINVAL);
	}

	c = avcodec_alloc_context3(NULL);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
        	exit(1);
	}

	if ((ret = avcodec_parameters_to_context(c, stream->codecpar)) < 0) {
		fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                av_get_media_type_string(AVMEDIA_TYPE_VIDEO));
        	return ret;
	}

	if (avcodec_open2(c, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec\n");
        	exit(1);
	}

	img_convert_ctx = sws_getContext(c->width, c->height,
					c->pix_fmt,
					c->width, c->height,
					AV_PIX_FMT_RGB24,
					SWS_BICUBIC,
					NULL, NULL, NULL);

	if (img_convert_ctx == NULL) {
		fprintf(stderr, "Cannot initialize the conversion context\n");
        	exit(1);
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
        	exit(1);
	}

	frame_count = 0;

	while (av_read_frame(fmt_ctx, &avpkt) >= 0) {
		if (avpkt.stream_index == stream_index) {
			if (decode_write_frame(outfilename, c, img_convert_ctx, frame, &frame_count, &avpkt, 0) < 0) {
				exit(1);
			}
		}
	}

	avpkt.data = NULL;
	avpkt.size = 0;
	decode_write_frame(outfilename, c, img_convert_ctx, frame, &frame_count, &avpkt, 1);

	fclose(f);
	avformat_close_input(&fmt_ctx);
	sws_freeContext(img_convert_ctx);
	avcodec_free_context(&c);
	av_frame_free(&frame);

	return 0;
}


























