//
//  main.cpp
//  ffmpeg
//
//  Created by 秀健身 on 2019/9/5.
//  Copyright © 2019 秀健身. All rights reserved.
//

#include <iostream>
#include "simplest_mediadata_raw.hpp"


int main(int argc, const char * argv[]) {
    // insert code here...
    
    simplest_yuv420_split("lena_256x256_yuv420p.yuv", 256, 256, 1);
    
    simplest_yuv444_split("lena_256x256_yuv444p.yuv", 256, 256, 1);
    
    simplest_yuv420_gray("lena_256x256_yuv420p.yuv", 256, 256, 1);
    
    simplest_yuv420_halfy("lena_256x256_yuv420p.yuv", 256, 256, 1);
    
    simplest_yuv420_border("lena_256x256_yuv420p.yuv", 256, 256, 20, 1);
    
    simplest_yuv420_graybar(640, 360, 0, 255, 10, "graybar_640x360.yuv");
    
    return 0;
}


