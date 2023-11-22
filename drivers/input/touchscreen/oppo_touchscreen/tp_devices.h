/***************************************************
 * File:tp_devices.h
 * VENDOR_EDIT
 * Copyright (c)  2008- 2030  Oppo Mobile communication Corp.ltd.
 * Description:
 *             tp dev
 * Version:1.0:
 * Date created:2016/09/02
 * Author: Tong.han@Bsp.Driver
 * TAG: BSP.TP.Init
 *
 * -------------- Revision History: -----------------
 *  <author >  <data>  <version>  <desc>
 ***************************************************/
#ifndef OPPO_TP_DEVICES_H
#define OPPO_TP_DEVICES_H
//device list define
typedef enum tp_dev{
    TP_INNOLUX,
    TP_HLT,
    TP_TXD,
    TP_DJN,
    TP_UNKNOWN,
}tp_dev;

struct tp_dev_name {
    tp_dev type;
    char name[32];
};

#endif

