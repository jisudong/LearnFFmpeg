//
//  simplest_mediadata_raw.cpp
//  ffmpeg
//
//  Created by 秀健身 on 2019/9/5.
//  Copyright © 2019 秀健身. All rights reserved.
//

#include <stdio.h>
#include <iostream>

/**
 Split Y, U, V planes in YUV420P file.
 
 @param url Location of Input YUV file.
 @param w   Width of Input YUV file.
 @param h   Height of Input YUV file.
 @param num Number of frames to process.
 */
int simplest_yuv420_split(const char *url, int w, int h, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp_y = fopen("output_420_y.y", "wb+");
    FILE *fp_u = fopen("output_420_u.y", "wb+");
    FILE *fp_v = fopen("output_420_v.y", "wb+");
    
    unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);
    
    for (int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3 / 2, fp);
        // Y
        fwrite(pic, 1, w * h, fp_y);
        // U
        fwrite(pic + w * h, 1, w * h / 4, fp_u);
        // V
        fwrite(pic + w * h * 5 / 4, 1, w * h / 4, fp_v);
    }
    
    free(pic);
    fclose(fp);
    fclose(fp_y);
    fclose(fp_u);
    fclose(fp_v);
    return 0;
}


/**
 分离YUV444P像素数据中的Y、U、V分量
 
 @param url 输入的YUV文件地址
 @param w 输入的YUV文件的宽
 @param h 输入的YUV文件的高
 @param num 要处理的帧数
 */
int simplest_yuv444_split(const char *url, int w, int h, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp_y = fopen("output_444_y.y", "wb+");
    FILE *fp_u = fopen("output_444_u.y", "wb+");
    FILE *fp_v = fopen("output_444_v.y", "wb+");
    
    unsigned char *pic = (unsigned char *)malloc(w * h * 3);
    
    for (int i = 0; i < num; i ++) {
        fread(pic, 1, w * h * 3, fp);
        // Y
        fwrite(pic, 1, w * h, fp_y);
        // U
        fwrite(pic + w * h, 1, w * h, fp_u);
        // V
        fwrite(pic + w * h * 2, 1, w * h, fp_v);
    }
    
    free(pic);
    fclose(fp);
    fclose(fp_y);
    fclose(fp_u);
    fclose(fp_v);
    
    return 0;
}


/**
 YUV420文件变成灰度图
 
 @param url 输入的YUV文件地址
 @param w 输入的YUV文件的宽
 @param h 输入的YUV文件的高
 @param num 要处理的帧数
 */
int simplest_yuv420_gray(const char *url, int w, int h, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_gray.yuv", "wb+");
    
    unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);
    
    for (int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3 / 2, fp);
        // gray
        memset(pic + w * h, 128, w * h / 2);
        fwrite(pic, 1, w * h * 3 / 2, fp1);
    }
    
    free(pic);
    fclose(fp);
    fclose(fp1);
    return 0;
}


/**
 将YUV420P像素数据的亮度减半
 
 @param url 输入的YUV文件地址
 @param w 输入的YUV文件的宽
 @param h 输入的YUV文件的高
 @param num 要处理的帧数
 */
int simplest_yuv420_halfy(const char *url, int w, int h, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_half.yuv", "wb+");
    
    unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);
    
    for (int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3 / 2, fp);
        // half
        for (int j = 0; j < w * h; j++) {
            unsigned char temp = pic[j] / 2;
            pic[j] = temp;
        }
        fwrite(pic, 1, w * h * 3/2, fp1);
    }
    
    free(pic);
    fclose(fp);
    fclose(fp1);
    return 0;
}


/**
 将YUV420P像素数据的周围加上边框
 
 @param url 输入的YUV文件地址
 @param w 输入的YUV文件的宽
 @param h 输入的YUV文件的高
 @param border 边框宽
 @param num 要处理的帧数
 */
int simplest_yuv420_border(const char *url, int w, int h, int border, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_border.yuv", "wb+");
    unsigned char *pic = (unsigned char *)malloc(w * h * 3 / 2);
    for (int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3 /2, fp);
        // Y
        for (int j = 0; j < h; j++) {
            for (int k = 0; k < w; k++) {
                if (k < border || k > (w - border) || j < border || j > (h - border)) {
                    pic[j * w + k] = 255;
                }
            }
        }
        fwrite(pic, 1, w * h * 3 / 2, fp1);
    }
    
    free(pic);
    fclose(fp);
    fclose(fp1);
    return 0;
}
