//
//  simplest_mediadata_raw.cpp
//  ffmpeg
//
//  Created by 秀健身 on 2019/9/5.
//  Copyright © 2019 秀健身. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <math.h>

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

/**
 生成YUV420P格式的灰阶测试图
 
 @param width 输出YUV的宽
 @param height 输出YUV的高
 @param ymin Y的最大值
 @param ymax Y的最小值
 @param barnum 灰阶的数量
 @param url_out 输出文件地址
 */
int simplest_yuv420_graybar(int width, int height, int ymin, int ymax, int barnum, const char *url_out) {
    int barwidth;
    float lum_inc;
    unsigned char lum_temp;
    int uv_width, uv_height;
    FILE *fp = NULL;
    unsigned char *data_y = NULL;
    unsigned char *data_u = NULL;
    unsigned char *data_v = NULL;
    int t = 0, i = 0, j = 0;
    
    barwidth = width / barnum;
    lum_inc = ((float)(ymax - ymin)) / ((float)(barnum - 1));
    uv_width = width / 2;
    uv_height = height / 2;
    
    data_y = (unsigned char *)malloc(width * height);
    data_u = (unsigned char *)malloc(uv_width * uv_height);
    data_v = (unsigned char *)malloc(uv_width * uv_height);
    
    if ((fp = fopen(url_out, "wb+")) == NULL) {
        printf("Error: Cannot create file!");
        return -1;
    }
    
    // Output Info
    printf("Y, U, V value from picture's left to right:\n");
    
    for (t = 0; t < (width / barwidth); t++) {
        lum_temp = ymin + (char)(t * lum_inc);
        printf("%3d, 128, 128\n", lum_temp);
    }
    
    // Gen Data
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            t = i / barwidth;
            lum_temp = ymin + (char)(t * lum_inc);
            data_y[j * width + i] = lum_temp;
        }
    }
    for (j = 0; j < uv_height; j++) {
        for (i = 0; i < uv_width; i++) {
            data_u[j * uv_width + i] = 128;
        }
    }
    for (j = 0; j < uv_height; j++) {
        for (i = 0; i < uv_width; i++) {
            data_v[j * uv_width + i] = 128;
        }
    }
    
    fwrite(data_y, width * height, 1, fp);
    fwrite(data_u, uv_width * uv_height, 1, fp);
    fwrite(data_v, uv_width * uv_height, 1, fp);
    fclose(fp);
    free(data_y);
    free(data_u);
    free(data_v);
    return 0;
}


/**
 计算两个YUV420P像素数据的PSNR
 
 @param url1 第一个输入YUV路径
 @param url2 第二个输入YUV路径
 @param w 输入YUV宽
 @param h 输入YUV高
 @param num 要处理的帧数
 */
int simplest_yuv420_psnr(const char *url1, const char *url2, int w, int h, int num) {
    FILE *fp1 = fopen(url1, "rb+");
    FILE *fp2 = fopen(url2, "rb+");
    unsigned char *pic1 = (unsigned char *)malloc(w * h);
    unsigned char *pic2 = (unsigned char *)malloc(w * h);
    
    for (int i = 0; i < num; i++) {
        fread(pic1, 1, w * h, fp1);
        fread(pic2, 1, w * h, fp2);
        
        double mse_sum = 0, mse = 0, psnr = 0;
        for (int j = 0; j < w * h; j++) {
            mse_sum += pow((double)(pic1[j] - pic2[j]), 2);
        }
        mse = mse_sum / (w * h);
        psnr = 10 * log10(255.0 * 255.0 / mse);
        printf("%5.3f\n", psnr);
        
        fseek(fp1, w * h / 2, SEEK_CUR);
        fseek(fp2, w * h / 2, SEEK_CUR);
    }
    free(pic1);
    free(pic2);
    fclose(fp1);
    fclose(fp2);
    return 0;
}
