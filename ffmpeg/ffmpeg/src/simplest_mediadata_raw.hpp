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


#endif /* simplest_mediadata_raw_h */
