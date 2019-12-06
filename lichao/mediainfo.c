#include <libavutil/log.h>
#include <libavformat/avformat.h>

int main(int argc, char* argv[])
{
	int ret;
	AVFormatContext *fmt_ctx = NULL;

	av_log_set_level(AV_LOG_INFO);

	av_register_all();

	ret = avformat_open_input(&fmt_ctx, "./Titanic.mp4", NULL, NULL);

	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "can't open file: %s\n", av_err2str(ret));
	
		return -1;
	}

	av_dump_format(fmt_ctx, 0, "./Titanic.mp4", 0);

	avformat_close_input(&fmt_ctx);

	return 0;
}
