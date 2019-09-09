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

#pragma mark - RGB、YUV像素数据处理

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


/**
 分离RGB24像素数据中的R、G、B分量
 
 @param url 输入rgb文件路径
 @param w 输入rgb文件宽
 @param h 输入rgb文件高
 @param num 处理的帧数
 */
int simplest_rgb24_split(const char *url, int w, int h, int num) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_r.yuv", "wb+");
    FILE *fp2 = fopen("output_g.yuv", "wb+");
    FILE *fp3 = fopen("output_b.yuv", "wb+");
    
    unsigned char *pic = (unsigned char *)malloc(w * h * 3);
    for (int i = 0; i < num; i++) {
        fread(pic, 1, w * h * 3, fp);
        
        for (int j = 0; j < w * h * 3; j = j + 3) {
            // R
            fwrite(pic + j, 1, 1, fp1);
            // G
            fwrite(pic + j + 1, 1, 1, fp2);
            // B
            fwrite(pic + j + 2, 1, 1, fp3);
        }
    }
    free(pic);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    fclose(fp3);
    return 0;
}


/**
 将RGB24格式像素数据封装为BMP图像
 
 @param rgb24path 输入rgb文件路径
 @param width 输入rgb文件宽
 @param height 输入rgb文件高
 @param bmppath 输出bmp文件路径
 */
int simplest_rgb24_to_bmp(const char *rgb24path, int width, int height, const char *bmppath) {
    typedef struct {
        long imageSize;
        long blank;
        long startPosition;
    } BmpHead;
    
    typedef struct {
        long Length;
        long width;
        long height;
        unsigned short colorPlane;
        unsigned short bitColor;
        long zipFormat;
        long realSize;
        long xPels;
        long yPels;
        long colorUse;
        long colorImportant;
    } InfoHead;
    
    int i = 0, j = 0;
    BmpHead m_BMPHeader = { 0 };
    InfoHead m_BMPInfoHeader = { 0 };
    char bfType[2] = { 'B', 'M' };
    int header_size = sizeof(bfType) + sizeof(BmpHead) + sizeof(InfoHead);
    unsigned char *rgb24_buffer = NULL;
    FILE *fp_rgb24 = NULL, *fp_bmp = NULL;
    
    if ((fp_rgb24 = fopen(rgb24path, "rb")) == NULL) {
        printf("Error: Cannot open input RGB24 file.\n");
        return -1;
    }
    if ((fp_bmp = fopen(bmppath, "wb")) == NULL) {
        printf("Error: Cannot open output BMP file.\n");
        return -1;
    }
    
    rgb24_buffer = (unsigned char *)malloc(width * height * 3);
    fread(rgb24_buffer, 1, width * height * 3, fp_rgb24);
    
    m_BMPHeader.imageSize = 3 * width * height + header_size;
    m_BMPHeader.startPosition = header_size;
    
    m_BMPInfoHeader.Length = sizeof(InfoHead);
    m_BMPInfoHeader.width = width;
    //BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
    m_BMPInfoHeader.height = -height;
    m_BMPInfoHeader.colorPlane = 1;
    m_BMPInfoHeader.bitColor = 24;
    m_BMPInfoHeader.realSize = 3 * width * height;
    
    fwrite(bfType, 1, sizeof(bfType), fp_bmp);
    fwrite(&m_BMPHeader, 1, sizeof(m_BMPHeader), fp_bmp);
    fwrite(&m_BMPInfoHeader, 1, sizeof(m_BMPInfoHeader), fp_bmp);
    
    //BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
    //It saves pixel data in Little Endian
    //So we change 'R' and 'B'
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            char temp = rgb24_buffer[(j * width + i) * 3 + 2];
            rgb24_buffer[(j * width + i) * 3 + 2] = rgb24_buffer[(j * width + i) * 3 + 0];
            rgb24_buffer[(j * width + i) * 3 + 0] = temp;
        }
    }
    
    fwrite(rgb24_buffer, 3 * width * height, 1, fp_bmp);
    fclose(fp_rgb24);
    fclose(fp_bmp);
    free(rgb24_buffer);
    printf("Finish generate %s!\n", bmppath);
    return 0;
}


unsigned char clip_value(unsigned char x, unsigned char min_val, unsigned char max_val) {
    if (x > max_val) {
        return max_val;
    } else if (x < min_val) {
        return min_val;
    } else {
        return x;
    }
}
// RGB to YUV420
bool rgb24_to_yuv420(unsigned char *rgbBuf, int w, int h, unsigned char *yuvBuf) {
    unsigned char *ptrY, *ptrU, *ptrV, *ptrRGB;
    memset(yuvBuf, 0, w * h * 3 / 2);
    ptrY = yuvBuf;
    ptrU = yuvBuf + w * h;
    ptrV = ptrU + (w * h / 4);
    unsigned char y, u, v, r, g, b;
    for (int j = 0; j < h; j++) {
        ptrRGB = rgbBuf + w * j * 3;
        for (int i = 0; i < w; i++) {
            r = *(ptrRGB++);
            g = *(ptrRGB++);
            b = *(ptrRGB++);
            y = (unsigned char)((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;
            u = (unsigned char)((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
            v = (unsigned char)((112 * r - 94 * g -  18 * b + 128) >> 8) + 128;
            *(ptrY++) = clip_value(y, 0, 255);
            if (j % 2 == 0 && i % 2 == 0) {
                *(ptrU++) = clip_value(u, 0, 255);
            } else {
                if (i % 2 == 0) {
                    *(ptrV++) = clip_value(v, 0, 255);
                }
            }
        }
    }
    return true;
}

/**
 将RGB24格式像素数据转换为YUV420P格式像素数据
 
 @param url_in 输入rgb文件路径
 @param w 输入rgb文件宽
 @param h 输入rgb文件高
 @param num 帧率
 @param url_out 输出yuv文件路径
 */
int simplest_rgb24_to_yuv420(const char *url_in, int w, int h, int num, const char *url_out) {
    FILE *fp = fopen(url_in, "rb+");
    FILE *fp1 = fopen(url_out, "wb+");
    
    unsigned char *pic_rgb24 = (unsigned char *)malloc(w * h * 3);
    unsigned char *pic_yuv420 = (unsigned char *)malloc(w * h * 3 / 2);
    
    for (int i = 0; i < num; i++) {
        fread(pic_rgb24, 1, w * h * 3, fp);
        rgb24_to_yuv420(pic_rgb24, w, h, pic_yuv420);
        fwrite(pic_yuv420, 1, w * h * 3 / 2, fp1);
    }
    
    free(pic_rgb24);
    free(pic_yuv420);
    fclose(fp);
    fclose(fp1);
    return 0;
}


/**
 生成RGB24格式的彩条测试图
 
 @param width 输出rgb文件宽
 @param height 输出rgb文件高
 @param url_out 输出rgb文件路径
 */
int simplest_rgb24_colorbar(int width, int height, const char *url_out) {
    unsigned char *data = NULL;
    int barwidth;
    char filename[100] = {0};
    FILE *fp = NULL;
    int i = 0, j = 0;
    
    data = (unsigned char *)malloc(width * height * 3);
    barwidth = width / 8;
    
    if ((fp = fopen(url_out, "wb+")) == NULL) {
        printf("Error: Cannot create file!");
        return -1;
    }
    
    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            int barnum = i / barwidth;
            switch (barnum) {
                case 0:
                    {
                        data[(j * width + i) * 3 + 0] = 255;
                        data[(j * width + i) * 3 + 1] = 255;
                        data[(j * width + i) * 3 + 2] = 255;
                    }
                    break;
                case 1:
                {
                    data[(j * width + i) * 3 + 0] = 255;
                    data[(j * width + i) * 3 + 1] = 255;
                    data[(j * width + i) * 3 + 2] = 0;
                }
                    break;
                case 2:
                {
                    data[(j * width + i) * 3 + 0] = 0;
                    data[(j * width + i) * 3 + 1] = 255;
                    data[(j * width + i) * 3 + 2] = 255;
                }
                    break;
                case 3:
                {
                    data[(j * width + i) * 3 + 0] = 0;
                    data[(j * width + i) * 3 + 1] = 255;
                    data[(j * width + i) * 3 + 2] = 0;
                }
                    break;
                case 4:
                {
                    data[(j * width + i) * 3 + 0] = 255;
                    data[(j * width + i) * 3 + 1] = 0;
                    data[(j * width + i) * 3 + 2] = 255;
                }
                    break;
                case 5:
                {
                    data[(j * width + i) * 3 + 0] = 255;
                    data[(j * width + i) * 3 + 1] = 0;
                    data[(j * width + i) * 3 + 2] = 0;
                }
                    break;
                case 6:
                {
                    data[(j * width + i) * 3 + 0] = 0;
                    data[(j * width + i) * 3 + 1] = 0;
                    data[(j * width + i) * 3 + 2] = 255;
                }
                    break;
                case 7:
                {
                    data[(j * width + i) * 3 + 0] = 0;
                    data[(j * width + i) * 3 + 1] = 0;
                    data[(j * width + i) * 3 + 2] = 0;
                }
                    break;
                    
                default:
                    break;
            }
        }
    }
    fwrite(data, width * height * 3, 1, fp);
    fclose(fp);
    free(data);
    return 0;
}



#pragma mark - PCM音频采样数据处理

/**
 分离PCM16LE双声道音频采样数据的左声道和右声道
 
 @param url 输入pcm文件路径
 */
int simplest_pcm16le_split(const char *url) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_l.pcm", "wb+");
    FILE *fp2 = fopen("output_r.pcm", "wb+");
    
    unsigned char *sample = (unsigned char *)malloc(4);
    while (!feof(fp)) {
        fread(sample, 1, 4, fp);
        // L
        fwrite(sample, 1, 2, fp1);
        // R
        fwrite(sample + 2, 1, 2, fp2);
    }
    
    free(sample);
    fclose(fp);
    fclose(fp1);
    fclose(fp2);
    return 0;
}


/**
 将PCM16LE双声道音频采样数据中左声道的音量降一半
 
 @param url 输入pcm文件路径
 */
int simplest_pcm16le_halfvolumeleft(const char *url) {
    FILE *fp = fopen(url, "rb+");
    FILE *fp1 = fopen("output_halfleft.pcm", "wb+");
    
    int cnt = 0;
    unsigned char *sample = (unsigned char *)malloc(4);
    while (!feof(fp)) {
        short *samplenum = NULL;
        fread(sample, 1, 4, fp);
        
        samplenum = (short *)sample;
        *samplenum = *samplenum / 2;
        // L
        fwrite(sample, 1, 2, fp1);
        // R
        fwrite(sample + 2, 1, 2, fp1);
        
        cnt++;
    }
    
    printf("Sample Cnt:%d\n",cnt);
    
    free(sample);
    fclose(fp);
    fclose(fp1);
    return 0;
}
