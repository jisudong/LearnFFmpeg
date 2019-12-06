#include <libavutil/log.h>
#include <libavformat/avformat.h>

int main(int argc, char* argv[])
{
	int ret;

	AVIODirContext *ctx = NULL;
	AVIODirEntry *entry = NULL;
	av_log_set_level(AV_LOG_INFO);

	ret = avio_open_dir(&ctx, "./", NULL);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "can't open dir:%s\n", av_err2str(ret));
		goto __fail;
	}

	while (1) {
		ret = avio_read_dir(ctx, &entry);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "can't read dir:%s\n", av_err2str(ret));
			return -1;
		}
		
		if (!entry) {
			break;
		}

		av_log(NULL, AV_LOG_INFO, "%12"PRId64" %s \n", entry->size, entry->name);

		avio_free_directory_entry(&entry);
	}
__fail:
	avio_close_dir(&ctx);

	return 0;
}
