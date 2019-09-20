//
//  simplest_ffmpeg_helloworld.cpp
//  ffmpeg
//
//  Created by 秀健身 on 2019/9/19.
//  Copyright © 2019 秀健身. All rights reserved.
//

#include "simplest_ffmpeg_helloworld.hpp"


extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
}

struct URLProtocol;


/**
 Protocol support information
 */
char * urlProtocolInfo() {
    char *info = (char *)malloc(40000);
    memset(info, 0, 40000);
    
    av_register_all();
    
    struct URLProtocol *pup = NULL;
    
    // input
    struct URLProtocol **p_temp = &pup;
    avio_enum_protocols((void **)p_temp, 0);
    while ((*p_temp) != NULL) {
        sprintf(info, "%s[In ][%10s]\n", info, avio_enum_protocols((void **)p_temp, 0));
    }
    pup = NULL;
    // output
    avio_enum_protocols((void **)p_temp, 1);
    while ((*p_temp) != NULL) {
        sprintf(info, "%s[Out][%10s]\n", info, avio_enum_protocols((void **)p_temp, 1));
    }
    
    return info;
}


/**
 * AVFormat Support Information
 */
char * avformatInfo() {
    char *info = (char *)malloc(40000);
    memset(info, 0, 40000);
    
    av_register_all();
    
    AVInputFormat *if_temp = av_iformat_next(NULL);
    AVOutputFormat *of_temp = av_oformat_next(NULL);
    
    // Input
    while (if_temp != NULL) {
        sprintf(info, "%s[In ] %10s\n", info, if_temp->name);
        if_temp = if_temp->next;
    }
    
    // Output
    while (of_temp != NULL) {
        sprintf(info, "%s[Out] %10s\n", info, of_temp->name);
        of_temp = of_temp->next;
    }
    
    return info;
}

/**
 * AVCodec Support Information
 */
char * avcodecInfo() {
    char *info = (char *)malloc(40000);
    memset(info, 0, 40000);
    
    av_register_all();
    
    AVCodec *c_temp = av_codec_next(NULL);
    
    while (c_temp != NULL) {
        if (c_temp->decode != NULL) {
            sprintf(info, "%s[Dec]", info);
        } else {
            sprintf(info, "%s[Enc]", info);
        }
        
        switch (c_temp->type) {
            case AVMEDIA_TYPE_VIDEO:
                sprintf(info, "%s[Video]", info);
                break;
                
            case AVMEDIA_TYPE_AUDIO:
                sprintf(info, "%s[Audio]", info);
                break;
                
            default:
                sprintf(info, "%s[Other]", info);
                break;
        }
        sprintf(info, "%s %10s\n", info, c_temp->name);
        
        c_temp = c_temp->next;
    }
    
    return info;
}



/**
 * AVFilter Support Information
 */
char * avfilterInfo() {
    char *info = (char *)malloc(40000);
    memset(info, 0, 40000);
    
    avfilter_register_all();
    
    AVFilter *f_temp = (AVFilter *)avfilter_next(NULL);
    
    while (f_temp != NULL) {
        sprintf(info, "%s[%15s]\n", info, f_temp->name);
        f_temp = f_temp->next;
    }
    
    return info;
}


/**
 * Configuration Information
 */
char * configurationInfo() {
    char *info = (char *)malloc(40000);
    memset(info, 0, 40000);
    
    av_register_all();
    
    sprintf(info, "%s\n", avcodec_configuration());
    
    return info;
}

int printFFmpegInfo() {
    
    char *infostr = NULL;
    infostr = configurationInfo();
    printf("\n<<Configuration>>\n%s", infostr);
    free(infostr);
    
    infostr = urlProtocolInfo();
    printf("\n<<URLProtocol>>\n%s", infostr);
    free(infostr);
    
    infostr = avformatInfo();
    printf("\n<<AVFormat>>\n%s", infostr);
    free(infostr);
    
    infostr = avcodecInfo();
    printf("\n<<AVCodec>>\n%s", infostr);
    free(infostr);
    
    infostr = avfilterInfo();
    printf("\n<<AVFilter>>\n%s", infostr);
    free(infostr);
    
    return 0;
}

