/*
 * drivers/media/i2c/soc_camera/xgold/veye290.c
 *
 * veye290 sensor driver
 *
 * Copyright (C) 2016 Fuzhou Rockchip Electronics Co., Ltd.
 * Copyright (C) 2012-2014 Intel Mobile Communications GmbH
 * Copyright (C) 2008 Texas Instruments.
 * Author: xumm <xumm@csoneplus.com>
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 *
 * Note:
 *
 * v0.1.0:
 *	1. Initialize version;
 */

#include <linux/i2c.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf-core.h>
#include <linux/slab.h>
#include <linux/of_gpio.h>
#include <linux/debugfs.h>
#include <media/v4l2-controls_rockchip.h>
#include "veye_camera_module.h"

#define VEYE_290_DRIVER_NAME "veye290"

/* product ID */
//ç³»ç»Ÿç›¸å…³å¯„å­˜å™¨,IDåœ°å€å’Œå€¼ã€‚
#define VEYE290_PID_MAGIC		0x06
#define VEYE290_PID_ADDR		0x0001
/*
#define INIT_END                              0x854A
#define SYS_STATUS                            0x8520
#define CONFCTL                               0x0004
#define VI_REP                                0x8576
#define MASK_VOUT_COLOR_SEL                   0xe0
#define MASK_VOUT_COLOR_RGB_FULL              0x00
#define MASK_VOUT_COLOR_RGB_LIMITED           0x20
#define MASK_VOUT_COLOR_601_YCBCR_FULL        0x40
#define MASK_VOUT_COLOR_601_YCBCR_LIMITED     0x60
#define MASK_VOUT_COLOR_709_YCBCR_FULL        0x80
#define MASK_VOUT_COLOR_709_YCBCR_LIMITED     0xa0
#define MASK_VOUT_COLOR_FULL_TO_LIMITED       0xc0
#define MASK_VOUT_COLOR_LIMITED_TO_FULL       0xe0
#define MASK_IN_REP_HEN                       0x10
#define MASK_IN_REP                           0x0f
*/
/* ======================================================================== */
/* Base sensor configs */
/* ======================================================================== */
/* MCLK:-NC  1920x1080  30fps   mipi 2lane   594Mbps/lane */
static struct veye_camera_module_reg veye_290_init_tab_1920_1080_30fps[] = {
	  
};

/* ======================================================================== */
static struct veye_camera_module_config veye_290_configs[] = {
	{
		.name = "1920x1080_25fps",
		.frm_fmt = {
			.width = 1920,
			.height = 1080,
			.code = MEDIA_BUS_FMT_UYVY8_2X8 //UYVYformat
		},
		.frm_intrvl = {
			.interval = {
				.numerator = 1,
				.denominator = 30
			}
		},
		.auto_exp_enabled = false,
		.auto_gain_enabled = false,
		.auto_wb_enabled = false,
		.reg_table = (void *)veye_290_init_tab_1920_1080_30fps,
		.reg_table_num_entries =
			sizeof(veye_290_init_tab_1920_1080_30fps) /
			sizeof(veye_290_init_tab_1920_1080_30fps[0]),
		.v_blanking_time_us = 1600,
		//not used yet
		.max_exp_gain_h = 16,
		.max_exp_gain_l = 0,
		.ignore_measurement_check = 1,
//		//2 lans 594M
		PLTFRM_CAM_ITF_MIPI_CFG(0, 2,594, 24000000)
	}
};

//static struct veye_priv veye_290_priv;

static struct veye_camera_module *to_veye_camera_module(struct v4l2_subdev *sd)
{
	return container_of(sd, struct veye_camera_module, sd);
}

/*--------------------------------------------------------------------------*/
static int veye_290_set_flip(
	struct veye_camera_module *cam_mod,
	struct veye_camera_module_reg reglist[],
	int len)
{
//TODO , we support mirror/filp
	//int mode = 0;
	
	return 0;
	
	/*mode = veye_camera_module_get_flip_mirror(cam_mod);

	if (mode == -1) {
		veye_camera_module_pr_debug(
			cam_mod,
			"dts don't set flip, return!\n");
		return 0;
	}

	return 0;*/
}

static int veye_290_write_aec(struct veye_camera_module *cam_mod)
{
	int ret = 0;
	return ret;
	/*
	veye_camera_module_pr_debug(cam_mod,
				  "exp_time=%d lines, gain=%d, flash_mode=%d\n",
				  cam_mod->exp_config.exp_time,
				  cam_mod->exp_config.gain,
				  cam_mod->exp_config.flash_mode);

	if (IS_ERR_VALUE(ret))
		veye_camera_module_pr_err(cam_mod,
					"failed with error (%d)\n", ret);
	return ret;*/
}
#if 0
//always enable stream output
int veye_290_enable_stream(struct veye_camera_module *cam_mod, bool enable)
{
	//u8 init_end, sys_status;
	//	u16 confctl;

	/*if (enable) {
		init_end = veye_camera_module_read8_reg(cam_mod, INIT_END);
		veye_camera_module_pr_debug(cam_mod,
					  "INIT_END(0x854A) = 0x%02x\n",
					  init_end);
		sys_status = veye_camera_module_read8_reg(cam_mod, SYS_STATUS);
		veye_camera_module_pr_debug(cam_mod,
					  "INIT_END(0x8520) = 0x%02x\n",
					  sys_status);
	}

	confctl = veye_camera_module_read16_reg(cam_mod, CONFCTL);
	veye_camera_module_pr_debug(cam_mod,
				  "CONFCTL(0x0004) = 0x%04x\n",
				  confctl);*/
	veye_camera_module_pr_debug(cam_mod,
				  "veye_290_enable_stream");
	return 0;
}
//always power on
int veye_290_s_power(struct veye_camera_module *cam_mod, bool enable)
{
	veye_camera_module_pr_debug(cam_mod,
				  "power %d\n",
				  enable);

	/*if (enable) {
		gpiod_direction_output(veye_290_priv.gpio_reset, 0);
		gpiod_direction_output(veye_290_priv.gpio_int, 0);
		gpiod_direction_output(veye_290_priv.gpio_stanby, 0);
		gpiod_direction_output(veye_290_priv.gpio_power18, 1);
		gpiod_direction_output(veye_290_priv.gpio_power33, 1);
		gpiod_direction_output(veye_290_priv.gpio_power, 1);
		gpiod_direction_output(veye_290_priv.gpio_stanby, 1);
		gpiod_direction_output(veye_290_priv.gpio_reset, 1);
	} else {
		gpiod_direction_output(veye_290_priv.gpio_power, 0);
		gpiod_direction_output(veye_290_priv.gpio_reset, 1);
		gpiod_direction_output(veye_290_priv.gpio_reset, 0);
	}*/

	return 0;
}
#endif
static int veye_290_g_fmt(
	struct v4l2_subdev *sd,
	struct v4l2_subdev_pad_config *cfg,
	struct v4l2_subdev_format *format)
{
	struct veye_camera_module *cam_mod =  to_veye_camera_module(sd);
	struct v4l2_mbus_framefmt *fmt = &format->format;
	/*u8 vi_rep = veye_camera_module_read8_reg(cam_mod, VI_REP);

	pltfrm_camera_module_pr_debug(&cam_mod->sd,
				      "read VI_REP return 0x%x\n", vi_rep);

	if (format->pad != 0)
		return -EINVAL;
	if (cam_mod->active_config) {
		fmt->code = cam_mod->active_config->frm_fmt.code;
		fmt->width = cam_mod->active_config->frm_fmt.width;
		fmt->height = cam_mod->active_config->frm_fmt.height;
		fmt->field = V4L2_FIELD_NONE;

		switch (vi_rep & MASK_VOUT_COLOR_SEL) {
		case MASK_VOUT_COLOR_RGB_FULL:
		case MASK_VOUT_COLOR_RGB_LIMITED:
			fmt->colorspace = V4L2_COLORSPACE_SRGB;
			break;
		case MASK_VOUT_COLOR_601_YCBCR_LIMITED:
		case MASK_VOUT_COLOR_601_YCBCR_FULL:
			fmt->colorspace = V4L2_COLORSPACE_SMPTE170M;
			break;
		case MASK_VOUT_COLOR_709_YCBCR_FULL:
		case MASK_VOUT_COLOR_709_YCBCR_LIMITED:
			fmt->colorspace = V4L2_COLORSPACE_REC709;
			break;
		default:
			fmt->colorspace = 0;
			break;
		}

		return 0;
	}
*/
	//pltfrm_camera_module_pr_err(&cam_mod->sd, "no active config\n");
	veye_camera_module_pr_debug(cam_mod,
				  "veye_290_g_fmt \n");
		if (format->pad != 0)
		return -EINVAL;
	if (cam_mod->active_config) {
		fmt->code = cam_mod->active_config->frm_fmt.code;
		fmt->width = cam_mod->active_config->frm_fmt.width;
		fmt->height = cam_mod->active_config->frm_fmt.height;
		fmt->field = V4L2_FIELD_NONE;
		fmt->colorspace = V4L2_COLORSPACE_SRGB;
	}
	return 0;
	//return -1;
}

static int veye_290_g_ctrl(struct veye_camera_module *cam_mod, u32 ctrl_id)
{
	int ret = 0;

	veye_camera_module_pr_debug(cam_mod, "\n");

	switch (ctrl_id) {
	case V4L2_CID_GAIN:
	case V4L2_CID_EXPOSURE:
	case V4L2_CID_FLASH_LED_MODE:
		/* nothing to be done here */
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (IS_ERR_VALUE(ret))
		veye_camera_module_pr_debug(cam_mod,
					  "failed with error (%d)\n",
					  ret);
	return ret;
}

/*--------------------------------------------------------------------------*/

static int veye_290_filltimings(
	struct veye_camera_module_custom_config *custom)
{
	return 0;
}

/*--------------------------------------------------------------------------*/

static int veye_290_g_timings(
	struct veye_camera_module *cam_mod,
	struct veye_camera_module_timings *timings)
{
int ret = 0;
	unsigned int vts;

	if (IS_ERR_OR_NULL(cam_mod->active_config))
		goto err;

	*timings = cam_mod->active_config->timings;

	vts = (!cam_mod->vts_cur) ?
		timings->frame_length_lines :
		cam_mod->vts_cur;
	if (cam_mod->frm_intrvl_valid)
		timings->vt_pix_clk_freq_hz =
			cam_mod->frm_intrvl.interval.denominator
			* vts
			* timings->line_length_pck;
	else
		timings->vt_pix_clk_freq_hz =
		cam_mod->active_config->frm_intrvl.interval.denominator *
		vts * timings->line_length_pck;

	timings->frame_length_lines = vts;

	return ret;
err:
	veye_camera_module_pr_err(cam_mod,
				"failed with error (%d)\n",
				ret);
//	return 0;
	return ret;
}

/*--------------------------------------------------------------------------*/

static int veye_290_s_ctrl(struct veye_camera_module *cam_mod, u32 ctrl_id)
{
	int ret = 0;

	veye_camera_module_pr_debug(cam_mod, "\n");

	switch (ctrl_id) {
	case V4L2_CID_GAIN:
	case V4L2_CID_EXPOSURE:
		ret = veye_290_write_aec(cam_mod);
		break;
	case V4L2_CID_FLASH_LED_MODE:
		/* nothing to be done here */
		break;
	case V4L2_CID_FOCUS_ABSOLUTE:
		/* todo*/
		break;
	/*
	 * case RK_V4L2_CID_FPS_CTRL:
	 * if (cam_mod->auto_adjust_fps)
	 * ret = TC358749xbg_auto_adjust_fps(
	 * cam_mod,
	 * cam_mod->exp_config.exp_time);
	 * break;
	 */
	default:
		ret = -EINVAL;
		break;
	}

	if (IS_ERR_VALUE(ret))
		veye_camera_module_pr_err(cam_mod,
					"failed with error (%d)\n",
					ret);
	return ret;
}

/*--------------------------------------------------------------------------*/

static int veye_290_s_ext_ctrls(
	struct veye_camera_module *cam_mod,
	struct veye_camera_module_ext_ctrls *ctrls)
{
	int ret = 0;

	if ((ctrls->ctrls[0].id == V4L2_CID_GAIN ||
		ctrls->ctrls[0].id == V4L2_CID_EXPOSURE))
		ret = veye_290_write_aec(cam_mod);
	else
		ret = -EINVAL;

	if (IS_ERR_VALUE(ret))
		veye_camera_module_pr_debug(cam_mod,
					  "failed with error (%d)\n",
					  ret);

	return ret;
}
/*--------------------------------------------------------------------------*/

static int veye_290_start_streaming(struct veye_camera_module *cam_mod)
{
	int ret = 0;

	veye_camera_module_pr_debug(cam_mod,
		"active config=%s\n", cam_mod->active_config->name);
	veye_camera_module_pr_debug(cam_mod, "\n");
	//Æô¶¯ÂëÁ÷£¬ÅäÖÃÎªmipiµÄÁ¬ÐøÄ£Ê½
	ret = veye_camera_module_write8_reg(cam_mod, 0x0B, 0x01);

	if (IS_ERR_VALUE(ret))
		goto err;

	return 0;
err:
	veye_camera_module_pr_err(cam_mod, "failed with error (%d)\n", ret);
	return ret;
}

/*--------------------------------------------------------------------------*/

static int veye_290_stop_streaming(struct veye_camera_module *cam_mod)
{
	veye_camera_module_pr_debug(cam_mod,
		"active config=%s\n", cam_mod->active_config->name);
	int ret = 0;

	veye_camera_module_pr_debug(cam_mod, "\n");

	ret = veye_camera_module_write8_reg(cam_mod, 0x0B, 0x00);

	if (IS_ERR_VALUE(ret))
		goto err;

	return 0;
err:
	veye_camera_module_pr_err(cam_mod, "failed with error (%d)\n", ret);
	return ret;
}

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/

static int veye_290_check_camera_id(struct veye_camera_module *cam_mod)
{
	u16 pid;
	int ret = 0;

	veye_camera_module_pr_debug(cam_mod, "in\n");

	pid = veye_camera_module_read8_reg(cam_mod, VEYE290_PID_ADDR);
	veye_camera_module_pr_err(cam_mod,
				"read pid return 0x%x\n",
				pid);

	if (pid == VEYE290_PID_MAGIC) {
		veye_camera_module_pr_debug(cam_mod,
					  "successfully detected camera ID 0x%02x\n",
					  pid);
	//set to ·ÇÁ¬ÐøÄ£Ê½
	 veye_camera_module_write8_reg(cam_mod, 0x0B, 0x00);
	
	} else {
		veye_camera_module_pr_err(cam_mod,
					"wrong camera ID, expected 0x%02x, detected 0x%02x\n",
					VEYE290_PID_MAGIC,
					pid);
		ret = -EINVAL;
		goto err;
	}

	return 0;
err:
	veye_camera_module_pr_err(cam_mod,
				"failed with error (%d)\n",
				ret);
	return ret;
}
static int veye_290_auto_adjust_fps(struct veye_camera_module *cam_mod,
	u32 exp_time)
{
	int ret = 0;
	//u32 vts;

	/*if ((exp_time + veye290_COARSE_INTG_TIME_MAX_MARGIN)
		> cam_mod->vts_min)
		vts = exp_time + ov13850_COARSE_INTG_TIME_MAX_MARGIN;
	else
		vts = cam_mod->vts_min;
	ret = ov_camera_module_write_reg(cam_mod,
		ov13850_TIMING_VTS_LOW_REG, vts & 0xFF);
	ret |= ov_camera_module_write_reg(cam_mod,
		ov13850_TIMING_VTS_HIGH_REG,
		(vts >> 8) & 0xFF);

	if (IS_ERR_VALUE(ret)) {
		ov_camera_module_pr_err(cam_mod,
			"failed with error (%d)\n", ret);
	} else {
		ov_camera_module_pr_info(cam_mod,
			"updated vts = %d,vts_min=%d\n",
			vts, cam_mod->vts_min);
		cam_mod->vts_cur = vts;
	}*/
cam_mod->vts_cur = exp_time;
	return ret;
}
/* ======================================================================== */
int tc_camera_veye290_module_s_ctrl(
	struct v4l2_subdev *sd,
	struct v4l2_control *ctrl)
{
	return 0;
}

/* ======================================================================== */

int tc_camera_veye290_module_s_ext_ctrls(
	struct v4l2_subdev *sd,
	struct v4l2_ext_controls *ctrls)
{
	return 0;
}

long tc_camera_veye290_module_ioctl(
	struct v4l2_subdev *sd,
	unsigned int cmd,
	void *arg)
{
	return 0;
}

/* ======================================================================== */
/* This part is platform dependent */
/* ======================================================================== */

static struct v4l2_subdev_core_ops veye_290_camera_module_core_ops = {
	.g_ctrl = veye_camera_module_g_ctrl,
	.s_ctrl = veye_camera_module_s_ctrl,
	.s_ext_ctrls = veye_camera_module_s_ext_ctrls,
	.s_power = veye_camera_module_s_power,
	.ioctl = veye_camera_module_ioctl
};

static struct v4l2_subdev_video_ops veye_290_camera_module_video_ops = {
	.s_frame_interval = veye_camera_module_s_frame_interval,
	.g_frame_interval = veye_camera_module_g_frame_interval,
	.s_stream = veye_camera_module_s_stream
};

static struct v4l2_subdev_pad_ops veye_290_camera_module_pad_ops = {
	.enum_frame_interval = veye_camera_module_enum_frameintervals,
	.get_fmt = veye_290_g_fmt,
	.set_fmt = veye_camera_module_s_fmt,
};

static struct v4l2_subdev_ops veye_290_camera_module_ops = {
	.core = &veye_290_camera_module_core_ops,
	.video = &veye_290_camera_module_video_ops,
	.pad = &veye_290_camera_module_pad_ops
};

//static struct veye_camera_module veye_290;

static struct veye_camera_module_custom_config veye_290_custom_config = {
	.start_streaming = veye_290_start_streaming,
	.stop_streaming = veye_290_stop_streaming,
	.s_ctrl = veye_290_s_ctrl,
	.g_ctrl = veye_290_g_ctrl,
	.s_ext_ctrls = veye_290_s_ext_ctrls,
	.g_timings = veye_290_g_timings,
	.set_flip = veye_290_set_flip,
	.check_camera_id = veye_290_check_camera_id,
	.s_vts = veye_290_auto_adjust_fps,
	//.enable_stream = veye_290_enable_stream,
	//.s_power = veye_290_s_power,
	.configs = veye_290_configs,
	.num_configs = ARRAY_SIZE(veye_290_configs),
	.power_up_delays_ms = {5, 30, 30},
	/*
	*0: Exposure time valid fileds;
	*1: Exposure gain valid fileds;
	*(2 fileds == 1 frames)
	*/
	.exposure_valid_frame = {4, 4}
};

static int test_parse_dts(
	struct veye_camera_module *cam_mod,
	struct i2c_client *client)
{
	//struct device *dev = &client->dev;
	//struct device_node *np = of_node_get(client->dev.of_node);

	veye_camera_module_pr_debug(cam_mod, "enter!\n");

/*	if (!dev)
		veye_camera_module_pr_debug(cam_mod, "dev is NULL!");

	if (!np)
		veye_camera_module_pr_debug(cam_mod, "np is NULL");

	veye_290_priv.gpio_int = devm_gpiod_get_optional(dev, "int",
							    GPIOD_OUT_HIGH);
	veye_290_priv.gpio_power = devm_gpiod_get_optional(dev, "power",
							      GPIOD_OUT_HIGH);
	veye_290_priv.gpio_power18 = devm_gpiod_get_optional(dev, "power18",
								GPIOD_OUT_HIGH);
	veye_290_priv.gpio_power33 = devm_gpiod_get_optional(dev, "power33",
								GPIOD_OUT_HIGH);
	veye_290_priv.gpio_csi_ctl = devm_gpiod_get_optional(dev, "csi-ctl",
								GPIOD_OUT_LOW);
	veye_290_priv.gpio_reset = devm_gpiod_get_optional(dev, "reset",
							      GPIOD_OUT_HIGH);
	veye_290_priv.gpio_stanby = devm_gpiod_get_optional(dev, "stanby",
							       GPIOD_OUT_LOW);*/
	return 0;
}

static int test_deparse_dts(struct i2c_client *client)
{
/*	if (veye_290_priv.gpio_int) {
		gpiod_direction_input(veye_290_priv.gpio_int);
		gpiod_put(veye_290_priv.gpio_int);
	}

	if (veye_290_priv.gpio_reset) {
		gpiod_direction_input(veye_290_priv.gpio_reset);
		gpiod_put(veye_290_priv.gpio_reset);
	}

	if (veye_290_priv.gpio_stanby) {
		gpiod_direction_input(veye_290_priv.gpio_stanby);
		gpiod_put(veye_290_priv.gpio_stanby);
	}

	if (veye_290_priv.gpio_csi_ctl) {
		gpiod_direction_input(veye_290_priv.gpio_csi_ctl);
		gpiod_put(veye_290_priv.gpio_csi_ctl);
	}

	if (veye_290_priv.gpio_power) {
		gpiod_direction_input(veye_290_priv.gpio_power);
		gpiod_put(veye_290_priv.gpio_power);
	}

	if (veye_290_priv.gpio_power18) {
		gpiod_direction_input(veye_290_priv.gpio_power18);
		gpiod_put(veye_290_priv.gpio_power18);
	}

	if (veye_290_priv.gpio_power33) {
		gpiod_direction_input(veye_290_priv.gpio_power33);
		gpiod_put(veye_290_priv.gpio_power33);
	}
*/
	return 0;
}

static ssize_t veye_290_debugfs_reg_write(
	struct file *file,
	const char __user *buf,
	size_t count,
	loff_t *ppos)
{
	struct veye_camera_module *cam_mod =
		((struct seq_file *)file->private_data)->private;

	char kbuf[30];
	char rw;
	u32 reg;
	u32 reg_value;
	int len;
	int ret;
	int nbytes = min(count, sizeof(kbuf) - 1);

	if (copy_from_user(kbuf, buf, nbytes))
		return -EFAULT;

	kbuf[nbytes] = '\0';
	veye_camera_module_pr_debug(cam_mod, "kbuf is %s\n", kbuf);
	ret = sscanf(kbuf, " %c %x %x %d", &rw, &reg, &reg_value, &len);
	veye_camera_module_pr_err(cam_mod, "ret = %d!\n", ret);
	if (ret != 4) {
		veye_camera_module_pr_err(cam_mod, "sscanf failed!\n");
		return 0;
	}

	if (rw == 'w') {
		if (len == 1) {
			veye_camera_module_write8_reg(cam_mod, reg, reg_value);
			veye_camera_module_pr_err(cam_mod,
						"%s(%d): write reg 0x%02x ---> 0x%x!\n",
						__func__, __LINE__,
						reg, reg_value);
		} else if (len == 2) {
			veye_camera_module_write16_reg(cam_mod, reg, reg_value);
			veye_camera_module_pr_err(cam_mod,
						"%s(%d): write reg 0x%02x ---> 0x%x!\n",
						__func__, __LINE__,
						reg, reg_value);
		} else if (len == 4) {
			veye_camera_module_write32_reg(cam_mod, reg, reg_value);
			veye_camera_module_pr_err(cam_mod,
						"%s(%d): write reg 0x%02x ---> 0x%x!\n",
						__func__, __LINE__,
						reg, reg_value);
		} else {
			veye_camera_module_pr_err(cam_mod,
						"len %d is err!\n",
						len);
		}
	} else if (rw == 'r') {
		if (len == 1) {
			reg_value = veye_camera_module_read8_reg(cam_mod, reg);
			veye_camera_module_pr_err(cam_mod,
						"%s(%d): read reg 0x%02x ---> 0x%x!\n",
						__func__, __LINE__,
						reg, reg_value);
		} else if (len == 2) {
			reg_value = veye_camera_module_read16_reg(cam_mod, reg);
			veye_camera_module_pr_err(cam_mod,
						"%s(%d): read reg 0x%02x ---> 0x%x!\n",
						__func__, __LINE__,
						reg, reg_value);
		} else if (len == 4) {
			reg_value = veye_camera_module_read32_reg(cam_mod, reg);
			veye_camera_module_pr_err(cam_mod,
						"%s(%d): read reg 0x%02x ---> 0x%x!\n",
						__func__, __LINE__,
						reg, reg_value);
		} else {
			veye_camera_module_pr_err(cam_mod,
						"len %d is err!\n",
						len);
		}
	} else {
		veye_camera_module_pr_err(cam_mod,
					"%c command is not support\n",
					rw);
	}

	return count;
}

static int veye_290_debugfs_reg_show(struct seq_file *s, void *v)
{
	int i;
	u32 value;
	int reg_table_num_entries =
			sizeof(veye_290_init_tab_1920_1080_30fps) /
			sizeof(veye_290_init_tab_1920_1080_30fps[0]);
	struct veye_camera_module *cam_mod = s->private;

	for (i = 0; i < reg_table_num_entries; i++) {
		switch (veye_290_init_tab_1920_1080_30fps[i].len) {
		case 1:
			value = veye_camera_module_read8_reg(cam_mod,
							   veye_290_init_tab_1920_1080_30fps[i].reg);
			break;
		case 2:
			value =  veye_camera_module_read16_reg(cam_mod,
							     veye_290_init_tab_1920_1080_30fps[i].reg);
			break;
		case 4:
			value =  veye_camera_module_read32_reg(cam_mod,
							     veye_290_init_tab_1920_1080_30fps[i].reg);
			break;
		default:
			printk(KERN_ERR "%s(%d): command no support!\n", __func__, __LINE__);
			break;
		}
		printk(KERN_ERR "%s(%d): reg 0x%04x ---> 0x%x\n", __func__, __LINE__,
		       veye_290_init_tab_1920_1080_30fps[i].reg, value);
	}
	return 0;
}

static int veye_290_debugfs_open(struct inode *inode, struct file *file)
{
	struct specific_sensor *spsensor = inode->i_private;

	return single_open(file, veye_290_debugfs_reg_show, spsensor);
}

static const struct file_operations veye_290_debugfs_fops = {
	.owner			= THIS_MODULE,
	.open			= veye_290_debugfs_open,
	.read			= seq_read,
	.write			= veye_290_debugfs_reg_write,
	.llseek			= seq_lseek,
	.release		= single_release
};

static struct dentry *debugfs_dir;

static int veye_290_probe(
	struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct veye_camera_module *priv;

	dev_info(&client->dev, "probing...\n");

	priv = devm_kzalloc(&client->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	v4l2_i2c_subdev_init(&priv->sd, client, &veye_290_camera_module_ops);
	priv->sd.flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;

	veye_290_filltimings(&veye_290_custom_config);
	priv->custom = veye_290_custom_config;

	mutex_init(&priv->lock);
	dev_info(&client->dev, "probing successful\n");
	
	/*veye_290_filltimings(&veye_290_custom_config);
	v4l2_i2c_subdev_init(&veye_290.sd, client,
			     &veye_290_camera_module_ops);

	veye_290.custom = veye_290_custom_config;*/

	test_parse_dts(&priv, client);
	debugfs_dir = debugfs_create_dir("veye290", NULL);
	if (IS_ERR(debugfs_dir))
		printk(KERN_ERR "%s(%d): create debugfs dir failed!\n",
		       __func__, __LINE__);
	else
		debugfs_create_file("register", S_IRUSR,
				    debugfs_dir, &priv,
				    &veye_290_debugfs_fops);

	dev_info(&client->dev, "probing successful\n");
	return 0;
}

/* ======================================================================== */

static int veye_290_remove(
	struct i2c_client *client)
{
	struct veye_camera_module *cam_mod = i2c_get_clientdata(client);

	dev_info(&client->dev, "remtcing device...\n");

	if (!client->adapter)
		return -ENODEV;	/* our client isn't attached */

	debugfs_remove_recursive(debugfs_dir);
	test_deparse_dts(client);
	mutex_destroy(&cam_mod->lock);
	veye_camera_module_release(cam_mod);

	dev_info(&client->dev, "remtced\n");
	return 0;
}

static const struct i2c_device_id veye_290_id[] = {
	{ VEYE_290_DRIVER_NAME, 0 },
	{ }
};

static const struct of_device_id veye_290_of_match[] = {
	{.compatible = "veye,veye290-v4l2-i2c-subdev"},
	{},
};

MODULE_DEVICE_TABLE(i2c, veye_290_id);

static struct i2c_driver veye_290_i2c_driver = {
	.driver = {
		.name = VEYE_290_DRIVER_NAME,
		.of_match_table = veye_290_of_match
	},
	.probe = veye_290_probe,
	.remove = veye_290_remove,
	.id_table = veye_290_id,
};

module_i2c_driver(veye_290_i2c_driver);

MODULE_DESCRIPTION("SoC Camera driver for veye290");
MODULE_AUTHOR("Benjo");
MODULE_LICENSE("GPL");
