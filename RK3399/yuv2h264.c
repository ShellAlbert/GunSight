//#define MODULE_TAG "mpi_enc_test"

#include <string.h>
#include "rk_mpi.h"

#include "mpp_env.h"
#include "mpp_mem.h"
#include "mpp_log.h"
#include "mpp_time.h"
#include "mpp_common.h"

#include "utils.h"

#define MAX_FILE_NAME_LENGTH        256

typedef struct {
    char            file_input[MAX_FILE_NAME_LENGTH];
    char            file_output[MAX_FILE_NAME_LENGTH];
    MppCodingType   type;
    RK_U32          width;
    RK_U32          height;
    MppFrameFormat  format;
    RK_U32          debug;
    RK_U32          num_frames;

    RK_U32          have_input;
    RK_U32          have_output;
} MpiEncTestCmd;

typedef struct {
    // global flow control flag
    RK_U32 frm_eos;
    RK_U32 pkt_eos;
    RK_U32 frame_count;
    RK_U64 stream_size;

    // src and dst
    FILE *fp_input;
    FILE *fp_output;

    // base flow context
    MppCtx ctx;
    MppApi *mpi;
    MppEncPrepCfg prep_cfg;
    MppEncRcCfg rc_cfg;
    MppEncCodecCfg codec_cfg;

    // input / output
    MppBuffer frm_buf;
    MppEncSeiMode sei_mode;

    // paramter for resource malloc
    RK_U32 width;
    RK_U32 height;
    RK_U32 hor_stride;
    RK_U32 ver_stride;
    MppFrameFormat fmt;
    MppCodingType type;
    RK_U32 num_frames;

    // resources
    size_t frame_size;
    /* NOTE: packet buffer may overflow */
    size_t packet_size;

    // rate control runtime parameter
    RK_S32 gop;
    RK_S32 fps;
    RK_S32 bps;
} MpiEncTestData;

static OptionInfo mpi_enc_cmd[] = {
    {"i",               "input_file",           "input bitstream file"},
    {"o",               "output_file",          "output bitstream file, "},
    {"w",               "width",                "the width of input picture"},
    {"h",               "height",               "the height of input picture"},
    {"f",               "format",               "the format of input picture"},
    {"t",               "type",                 "output stream coding type"},
    {"n",               "max frame number",     "max encoding frame number"},
    {"d",               "debug",                "debug flag"},
};




MPP_RET test_mpp_run(MpiEncTestData *p)
{

}



int main(int argc, char **argv)
{
    MPP_RET ret = MPP_OK;
    MpiEncTestCmd  cmd_ctx;
    MpiEncTestCmd* cmd = &cmd_ctx;

    memset((void*)cmd, 0, sizeof(*cmd));

    //prepare parameters.
    strcpy(cmd->file_input,"a.yuv");
    cmd->have_input = 1;
    strcpy(cmd->file_output,"a.h264");
    cmd->have_output = 1;
    cmd->debug=1;
    cmd->width=640;
    cmd->height=480;
    cmd->format=MPP_FMT_YUV422_YUYV;
    cmd->type=MPP_VIDEO_CodingAVC;
    cmd->num_frames=10;

    ////////////////////////////////////////////////


    mpp_log("cmd parse result:\n");
    mpp_log("input  file name: %s\n", cmd->file_input);
    mpp_log("output file name: %s\n", cmd->file_output);
    mpp_log("width      : %d\n", cmd->width);
    mpp_log("height     : %d\n", cmd->height);
    mpp_log("format     : %d\n", cmd->format);
    mpp_log("type       : %d\n", cmd->type);
    mpp_log("debug flag : %x\n", cmd->debug);

    ///////////////////////////////////////////////
    mpp_env_set_u32("mpi_debug", cmd->debug);

    //    ret = mpi_enc_test(cmd);


    MpiEncTestData *p = NULL;
    p = mpp_calloc(MpiEncTestData, 1);
    if (!p)
    {
        mpp_err_f("create MpiEncTestData failed\n");
        return -1;
    }

    // get paramter from cmd
    p->width        = cmd->width;
    p->height       = cmd->height;
    p->hor_stride   = MPP_ALIGN(cmd->width, 16);
    p->ver_stride   = MPP_ALIGN(cmd->height, 16);
    p->fmt          = cmd->format;
    p->type         = cmd->type;

    mpp_log("hor_stride      : %d\n", p->hor_stride);
    mpp_log("ver_stride     : %d\n", p->ver_stride);

    if (cmd->type == MPP_VIDEO_CodingMJPEG)
    {
        cmd->num_frames = 1;
    }
    p->num_frames=cmd->num_frames;

    if (cmd->have_input)
    {
        p->fp_input = fopen(cmd->file_input, "rb");
        if (NULL == p->fp_input)
        {
            mpp_err("failed to open input file %s\n", cmd->file_input);
            mpp_err("create default yuv image for test\n");
        }
    }

    if (cmd->have_output)
    {
        p->fp_output = fopen(cmd->file_output, "w+b");
        if (NULL == p->fp_output)
        {
            mpp_err("failed to open output file %s\n", cmd->file_output);
            ret = MPP_ERR_OPEN_FILE;
        }
    }

    // update resource parameter
    if (p->fmt <= MPP_FMT_YUV420SP_VU)
    {
        p->frame_size = p->hor_stride * p->ver_stride * 3 / 2;
    }
    else if (p->fmt <= MPP_FMT_YUV422_UYVY)
    {
        // NOTE: yuyv and uyvy need to double stride
        p->hor_stride *= 2;
        p->frame_size = p->hor_stride * p->ver_stride;
    } else{
        p->frame_size = p->hor_stride * p->ver_stride * 4;
    }
    p->packet_size  = p->width * p->height;


    //    ret = test_ctx_init(&p, cmd);
    //    if (ret)
    //    {
    //        mpp_err_f("test data init failed ret %d\n", ret);
    //        return -1;
    //    }
    /////////////////////////////////////////////
    ret = mpp_buffer_get(NULL, &p->frm_buf, p->frame_size);
    if (ret) {
        mpp_err_f("failed to get buffer for input frame ret %d\n", ret);
        return -1;
    }

    mpp_log("mpi_enc_test encoder test start w %d h %d type %d\n",p->width, p->height, p->type);

    // encoder demo
    ret = mpp_create(&p->ctx, &p->mpi);
    if (ret)
    {
        mpp_err("mpp_create failed ret %d\n", ret);
        return -1;
    }

    ret = mpp_init(p->ctx, MPP_CTX_ENC, p->type);
    if (ret)
    {
        mpp_err("mpp_init failed ret %d\n", ret);
        return -1;
    }

    //    ret = test_mpp_setup(p);
    //    if (ret) {
    //        mpp_err_f("test mpp setup failed ret %d\n", ret);
    //         return -1;
    //    }
    MppApi *mpi;
    MppCtx ctx;
    MppEncCodecCfg *codec_cfg;
    MppEncPrepCfg *prep_cfg;
    MppEncRcCfg *rc_cfg;

    mpi = p->mpi;
    ctx = p->ctx;
    codec_cfg = &p->codec_cfg;
    prep_cfg = &p->prep_cfg;
    rc_cfg = &p->rc_cfg;

    /* setup default parameter */
    p->fps = 30;
    p->gop = 60;
    p->bps = p->width * p->height / 8 * p->fps;

    prep_cfg->change = MPP_ENC_PREP_CFG_CHANGE_INPUT |MPP_ENC_PREP_CFG_CHANGE_ROTATION |MPP_ENC_PREP_CFG_CHANGE_FORMAT;
    prep_cfg->width         = p->width;
    prep_cfg->height        = p->height;
    prep_cfg->hor_stride    = p->hor_stride;
    prep_cfg->ver_stride    = p->ver_stride;
    prep_cfg->format        = p->fmt;
    prep_cfg->rotation      = MPP_ENC_ROT_0;
    ret = mpi->control(ctx, MPP_ENC_SET_PREP_CFG, prep_cfg);
    if (ret)
    {
        mpp_err("mpi control enc set prep cfg failed ret %d\n", ret);
        return -1;
    }

    rc_cfg->change  = MPP_ENC_RC_CFG_CHANGE_ALL;
    rc_cfg->rc_mode = MPP_ENC_RC_MODE_CBR;
    rc_cfg->quality = MPP_ENC_RC_QUALITY_MEDIUM;

    if (rc_cfg->rc_mode == MPP_ENC_RC_MODE_CBR)
    {
        /* constant bitrate has very small bps range of 1/16 bps */
        rc_cfg->bps_target   = p->bps;
        rc_cfg->bps_max      = p->bps * 17 / 16;
        rc_cfg->bps_min      = p->bps * 15 / 16;
    } else if (rc_cfg->rc_mode ==  MPP_ENC_RC_MODE_VBR)
    {
        if (rc_cfg->quality == MPP_ENC_RC_QUALITY_CQP)
        {
            /* constant QP does not have bps */
            rc_cfg->bps_target   = -1;
            rc_cfg->bps_max      = -1;
            rc_cfg->bps_min      = -1;
        } else {
            /* variable bitrate has large bps range */
            rc_cfg->bps_target   = p->bps;
            rc_cfg->bps_max      = p->bps * 17 / 16;
            rc_cfg->bps_min      = p->bps * 1 / 16;
        }
    }

    /* fix input / output frame rate */
    rc_cfg->fps_in_flex      = 0;
    rc_cfg->fps_in_num       = p->fps;
    rc_cfg->fps_in_denorm    = 1;
    rc_cfg->fps_out_flex     = 0;
    rc_cfg->fps_out_num      = p->fps;
    rc_cfg->fps_out_denorm   = 1;

    rc_cfg->gop              = p->gop;
    rc_cfg->skip_cnt         = 0;

    mpp_log("mpi_enc_test bps %d fps %d gop %d\n",rc_cfg->bps_target, rc_cfg->fps_out_num, rc_cfg->gop);
    ret = mpi->control(ctx, MPP_ENC_SET_RC_CFG, rc_cfg);
    if (ret)
    {
        mpp_err("mpi control enc set rc cfg failed ret %d\n", ret);
        return -1;
    }

    codec_cfg->coding = p->type;
    switch (codec_cfg->coding)
    {
    case MPP_VIDEO_CodingAVC :
    {
        codec_cfg->h264.change = MPP_ENC_H264_CFG_CHANGE_PROFILE |
                MPP_ENC_H264_CFG_CHANGE_ENTROPY |
                MPP_ENC_H264_CFG_CHANGE_TRANS_8x8;
        /*
         * H.264 profile_idc parameter
         * 66  - Baseline profile
         * 77  - Main profile
         * 100 - High profile
         */
        codec_cfg->h264.profile  = 100;
        /*
         * H.264 level_idc parameter
         * 10 / 11 / 12 / 13    - qcif@15fps / cif@7.5fps / cif@15fps / cif@30fps
         * 20 / 21 / 22         - cif@30fps / half-D1@@25fps / D1@12.5fps
         * 30 / 31 / 32         - D1@25fps / 720p@30fps / 720p@60fps
         * 40 / 41 / 42         - 1080p@30fps / 1080p@30fps / 1080p@60fps
         * 50 / 51 / 52         - 4K@30fps
         */
        codec_cfg->h264.level    = 40;
        codec_cfg->h264.entropy_coding_mode  = 1;
        codec_cfg->h264.cabac_init_idc  = 0;
        codec_cfg->h264.transform8x8_mode = 1;
    }
        break;
    case MPP_VIDEO_CodingMJPEG :
    {
        codec_cfg->jpeg.change  = MPP_ENC_JPEG_CFG_CHANGE_QP;
        codec_cfg->jpeg.quant   = 10;
    }
        break;
    case MPP_VIDEO_CodingVP8 :
    case MPP_VIDEO_CodingHEVC :
    default :
    {
        mpp_err_f("support encoder coding type %d\n", codec_cfg->coding);
    }
        break;
    }
    ret = mpi->control(ctx, MPP_ENC_SET_CODEC_CFG, codec_cfg);
    if (ret)
    {
        mpp_err("mpi control enc set codec cfg failed ret %d\n", ret);
        return -1;
    }

    /* optional */
    p->sei_mode = MPP_ENC_SEI_MODE_ONE_FRAME;
    ret = mpi->control(ctx, MPP_ENC_SET_SEI_CFG, &p->sei_mode);
    if (ret)
    {
        mpp_err("mpi control enc set sei cfg failed ret %d\n", ret);
        return -1;
    }

    ////////////////////////////////////////////////////////////////
//    ret = test_mpp_run(p);
//    if (ret) {
//        mpp_err_f("test mpp run failed ret %d\n", ret);
//        return -1;
//    }


    if (p->type == MPP_VIDEO_CodingAVC)
    {
        MppPacket packet = NULL;
        ret = mpi->control(ctx, MPP_ENC_GET_EXTRA_INFO, &packet);
        if (ret) {
            mpp_err("mpi control enc get extra info failed\n");
            return -1;
        }

        /* get and write sps/pps for H.264 */
        if (packet)
        {
            void *ptr   = mpp_packet_get_pos(packet);
            size_t len  = mpp_packet_get_length(packet);

            if (p->fp_output)
            {
                fwrite(ptr, 1, len, p->fp_output);
		printf("***********************************write sps/pps to file:%d\n",len);
            }
            packet = NULL;
        }
    }

   int count=0;
    while (!p->pkt_eos)
    {
	printf("test how many this loop runs? %d\n",count);
	fclose(p->fp_input);
	p->fp_input = fopen(cmd->file_input, "rb");
        MppFrame frame = NULL;
        MppPacket packet = NULL;
        void *buf = mpp_buffer_get_ptr(p->frm_buf);

        if (p->fp_input)
        {
	   printf("read_yuv_image:%d,%d,%d,%d,%d\n",p->width, p->height,p->hor_stride, p->ver_stride, p->fmt);
            ret = read_yuv_image(buf, p->fp_input, p->width, p->height,p->hor_stride, p->ver_stride, p->fmt);

            if (ret == MPP_NOK  || feof(p->fp_input)) {
                mpp_log("found last frame. feof %d\n", feof(p->fp_input));
                //p->frm_eos = 1;
		printf("HERE SET EOS=1\n");
            } else if (ret == MPP_ERR_VALUE)
            {
                break;
            }

/*
	    fseek(p->fp_input,0,SEEK_SET);
	    count++;
	    if(count>=10)
		{
			p->frm_eos=1;
			printf("10 so set end of stream to 1\n");
		}
*/
        } else {
            ret = fill_yuv_image(buf, p->width, p->height, p->hor_stride,p->ver_stride, p->fmt, p->frame_count);
            if (ret)
            {
                break;
            }
        }

        ret = mpp_frame_init(&frame);
        if (ret)
        {
            mpp_err_f("mpp_frame_init failed\n");
            break;
        }

        mpp_frame_set_width(frame, p->width);
        mpp_frame_set_height(frame, p->height);
        mpp_frame_set_hor_stride(frame, p->hor_stride);
        mpp_frame_set_ver_stride(frame, p->ver_stride);
        mpp_frame_set_fmt(frame, p->fmt);
        mpp_frame_set_buffer(frame, p->frm_buf);
        mpp_frame_set_eos(frame, p->frm_eos);
	printf("width:%d,height:%d,hor_stride:%d,ver_stride:%d,eos:%d\n",p->width,p->height,p->hor_stride,p->ver_stride,p->frm_eos);
        ret = mpi->encode_put_frame(ctx, frame);
        if (ret)
        {
            mpp_err("mpp encode put frame failed\n");
            break;
        }

        ret = mpi->encode_get_packet(ctx, &packet);
        if (ret)
        {
            mpp_err("mpp encode get packet failed\n");
            break;
        }

        if (packet)
        {
            // write packet to file here
            void *ptr   = mpp_packet_get_pos(packet);
            size_t len  = mpp_packet_get_length(packet);

            p->pkt_eos = mpp_packet_get_eos(packet);

            if (p->fp_output)
            {
                fwrite(ptr, 1, len, p->fp_output);
		printf("***********************************write h264 data  to file:%d\n",len);
            }

            mpp_packet_deinit(&packet);

            mpp_log_f("encoded frame %d size %d\n", p->frame_count, len);
            p->stream_size += len;
            p->frame_count++;

            if (p->pkt_eos) {
                mpp_log("found last packet\n");
                mpp_assert(p->frm_eos);
            }
        }else{
		printf("not got a valid pkt.\n");
	}

	printf("num_frames:%d,frame_count:%d\n",p->num_frames,p->frame_count);
        if (p->num_frames && p->frame_count >= p->num_frames)
        {
            printf("encode max %d frames\n", p->frame_count);
            break;
        }
	printf("check eos flag...\n");
        if (p->frm_eos && p->pkt_eos)
        {
		printf("check eos flag...\n");
            break;
        }
    }
    ///////////////////////////////////////////////////////
   printf("reset mpi\n");
    ret = p->mpi->reset(p->ctx);
    if (ret) {
        mpp_err("mpi->reset failed\n");
        return -1;
    }


    if (p->ctx)
    {
        mpp_destroy(p->ctx);
        p->ctx = NULL;
    }

    if (p->frm_buf)
    {
        mpp_buffer_put(p->frm_buf);
        p->frm_buf = NULL;
    }

    if (MPP_OK == ret)
        mpp_log("mpi_enc_test success total frame %d bps %lld\n",
                p->frame_count, (RK_U64)((p->stream_size * 8 * p->fps) / p->frame_count));
    else
        mpp_err("mpi_enc_test failed ret %d\n", ret);

    //    test_ctx_deinit(&p);
    if (p)
    {
        if (p->fp_input)
        {
            fclose(p->fp_input);
            p->fp_input = NULL;
        }
        if (p->fp_output)
        {
            fclose(p->fp_output);
            p->fp_output = NULL;
        }
 //       MPP_FREE(p);
    }
    //////////////////////////////////////////////
    mpp_env_set_u32("mpi_debug", 0x0);
    return ret;
}

