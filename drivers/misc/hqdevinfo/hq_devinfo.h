/********************************************
 ** Copyright (C) 2018 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: hq_devinfo.h
 ** Description: Source file for proc devinfo
 ** Version :1.0
 ** Date : 2018/12/5
 ** Author: duwenchao@ODM_HQ.BSP.system
 ********************************************/
#ifndef _HQ_DEVINFO_HEAD_
#define _HQ_DEVINFO_HEAD_

#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/kfifo.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define HQ_DEVINFO_ATTR(name, fmt, args...) \
static int hq_##name##_show(struct seq_file *m, void *v){\
	char name_tmp[32];\
	strcpy(name_tmp, #name);\
	register_info(name_tmp);\
	seq_printf(m, fmt, args);\
	return 0;\
}\
static int hq_##name##_open(struct inode *inode, struct file *file){\
	return single_open(file, hq_##name##_show, inode->i_private);\
}\
static const struct file_operations name##_fops = {\
	.open = hq_##name##_open,\
	.read = seq_read,\
}

typedef struct{
	char camera_main_name[32];
	char camera_aux_name[32];
	char camera_micro_name[32];
	char camera_front_name[32];
}cam_buff_t;

#ifdef ODM_HQ_EDIT
/* Yuzhe.Peng@ODM.HQ.BSP.Sensors.Config 2019/10/15 add for sensor information */
struct sensor_devinfo {
	char *ic_name;
	char *vendor_name;
};

enum {
    HQ_ACCEL_DEVICE,
    HQ_LIGHT_DEVICE,
    HQ_PROX_DEVICE,
    HQ_MAG_DEVICE,
    HQ_RESERVE_DEVICE
};

void hq_register_sensor_info(int sensor_type, char ic_name[]);
#endif

#endif