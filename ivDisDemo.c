#include<stdio.h>
#include<libavutil/avutil.h>
#include<libavcodec/avcodec.h>
#include<libavformat/avformat.h>

int main(int argc, char *argv[])
{
	printf("ffmpeg test begin.\n");
	test();
	av_register_all();
	return 0;
}
