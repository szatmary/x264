// cd .. && make; cd sz && gcc test.c ../libx264.a -o test && ./test
#include <assert.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>

#include "../x264.h"

int encode_frames(struct x264_t* h, int first_pts, int frame_dur)
{
    x264_param_t params;
    x264_encoder_parameters(h, &params);
    assert(params.i_csp == X264_CSP_I420);

    x264_picture_t pic_in, pic_out;
    x264_picture_init(&pic_in);
    x264_picture_init(&pic_out);

    x264_picture_alloc(&pic_in, params.i_csp, params.i_width, params.i_height);
    memset(pic_in.img.plane[0], 0, pic_in.img.i_stride[0] * params.i_height);
    memset(pic_in.img.plane[1], 0, pic_in.img.i_stride[1] * (params.i_height >> 1));
    memset(pic_in.img.plane[2], 0, pic_in.img.i_stride[2] * (params.i_height >> 1));

    int i_nals = 0;
    x264_nal_t* p_nals = 0;
    for (int i = 0; i < 25; ++i) {
        pic_in.i_dts = pic_in.i_pts = first_pts + (i * frame_dur);
        if (x264_encoder_encode(h, &p_nals, &i_nals, &pic_in, &pic_out) > 0) {
            printf("dts: %lld, pts: %lld\n", pic_out.i_dts, pic_out.i_pts);
            x264_picture_clean(&pic_out);
        }
    }

    while (x264_encoder_delayed_frames(h) > 0) {
        if (x264_encoder_encode(h, &p_nals, &i_nals, NULL, &pic_out) > 0) {
            printf("dts: %lld, pts: %lld\n", pic_out.i_dts, pic_out.i_pts);
            x264_picture_clean(&pic_out);
        }
    }

    x264_picture_clean(&pic_in);
    return 0;
}

int main(int argc, char** argv)
{

    struct x264_param_t param;
    x264_param_default(&param);
    x264_param_default_preset(&param, "medium", 0);

    param.i_fps_den = 1;
    param.i_fps_num = 60;
    param.i_timebase_num = 1;
    param.i_timebase_den = 1000;

    param.i_bframe_adaptive = X264_B_ADAPT_NONE;
    param.i_bframe_pyramid = X264_B_PYRAMID_NONE;

    param.i_width = 1920;
    param.i_height = 1080;
    param.i_csp = X264_CSP_I420;
    param.vui.b_fullrange = 1;

    param.b_stitchable = 1;
    param.b_vfr_input = 1;

    struct x264_t* h = x264_encoder_open(&param);
    printf("%d,%d\n", param.i_fps_num, param.i_fps_den);
    // x264_encoder_headers(x264enc, &nals, &i_nals);
    encode_frames(h, 0, 10);
    x264_encoder_close(h);
    h = x264_encoder_open(&param);
    printf("-------------\n");

    encode_frames(h, 40 * 8, 85);
    x264_encoder_close(h);
}
