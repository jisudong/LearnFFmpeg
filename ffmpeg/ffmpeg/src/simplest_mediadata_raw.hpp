//
//  simplest_mediadata_raw.h
//  ffmpeg
//
//  Created by 秀健身 on 2019/9/5.
//  Copyright © 2019 秀健身. All rights reserved.
//

#ifndef simplest_mediadata_raw_hpp
#define simplest_mediadata_raw_hpp


/**
 分离YUV420P像素数据中的Y、U、V分量

 @param url 输入的YUV文件地址
 @param w 输入的YUV文件的宽
 @param h 输入的YUV文件的高
 @param num 要处理的帧数
 */
int simplest_yuv420_split(const char *url, int w, int h, int num);

/**
 分离YUV444P像素数据中的Y、U、V分量

 @param url 输入的YUV文件地址
 @param w 输入的YUV文件的宽
 @param h 输入的YUV文件的高
 @param num 要处理的帧数
 */
int simplest_yuv444_split(const char *url, int w, int h, int num);

/**
 YUV420文件变成灰度图

 @param url 输入的YUV文件地址
 @param w 输入的YUV文件的宽
 @param h 输入的YUV文件的高
 @param num 要处理的帧数
 */
int simplest_yuv420_gray(const char *url, int w, int h, int num);

/**
 将YUV420P像素数据的亮度减半

 @param url 输入的YUV文件地址
 @param w 输入的YUV文件的宽
 @param h 输入的YUV文件的高
 @param num 要处理的帧数
 */
int simplest_yuv420_halfy(const char *url, int w, int h, int num);

/**
 将YUV420P像素数据的周围加上边框

 @param url 输入的YUV文件地址
 @param w 输入的YUV文件的宽
 @param h 输入的YUV文件的高
 @param border 边框宽
 @param num 要处理的帧数
 */
int simplest_yuv420_border(const char *url, int w, int h, int border, int num);

/**
 生成YUV420P格式的灰阶测试图

 @param width 输出YUV的宽
 @param height 输出YUV的高
 @param ymin Y的最大值
 @param ymax Y的最小值
 @param barnum 灰阶的数量
 @param url_out 输出文件地址
 */
int simplest_yuv420_graybar(int width, int height, int ymin, int ymax, int barnum, const char *url_out);

/**
 计算两个YUV420P像素数据的PSNR

 @param url1 第一个输入YUV路径
 @param url2 第二个输入YUV路径
 @param w 输入YUV宽
 @param h 输入YUV高
 @param num 要处理的帧数
 */
int simplest_yuv420_psnr(const char *url1, const char *url2, int w, int h, int num);

/**
 分离RGB24像素数据中的R、G、B分量

 @param url 输入rgb文件路径
 @param w 输入rgb文件宽
 @param h 输入rgb文件高
 @param num 处理的帧数
 */
int simplest_rgb24_split(const char *url, int w, int h, int num);

/**
 将RGB24格式像素数据封装为BMP图像

 @param rgb24path 输入rgb文件路径
 @param width 输入rgb文件宽
 @param height 输入rgb文件高
 @param bmppath 输出bmp文件路径
 */
int simplest_rgb24_to_bmp(const char *rgb24path, int width, int height, const char *bmppath);

/**
 将RGB24格式像素数据转换为YUV420P格式像素数据

 @param url_in 输入rgb文件路径
 @param w 输入rgb文件宽
 @param h 输入rgb文件高
 @param num 帧率
 @param url_out 输出yuv文件路径
 */
int simplest_rgb24_to_yuv420(const char *url_in, int w, int h, int num, const char *url_out);

/**
 生成RGB24格式的彩条测试图

 @param width 输出rgb文件宽
 @param height 输出rgb文件高
 @param url_out 输出rgb文件路径
 */
int simplest_rgb24_colorbar(int width, int height, const char *url_out);


#endif /* simplest_mediadata_raw_h */
