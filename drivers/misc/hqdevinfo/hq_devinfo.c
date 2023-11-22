/********************************************
 ** Copyright (C) 2018 OPPO Mobile Comm Corp. Ltd.
 ** ODM_HQ_EDIT
 ** File: hq_devinfo.c
 ** Description: Source file for proc devinfo
 ** Version :1.0
 ** Date : 2018/12/5
 ** Author: duwenchao@ODM_HQ.BSP.system
 ********************************************/
#include <linux/mmc/host.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include "hq_devinfo.h"
#include <soc/oppo/device_info.h>

#define devinfo_name "devinfo"

extern char battery_name[20];
extern struct mmc_card *card_devinfo;
char emmc[32];
cam_buff_t cam_buff;
char buff[128];
extern void meta_work_around(void);
#ifdef ODM_HQ_EDIT
/* Sunshiyue@ODM.Multimedia.LCD  2019/9/25 add for lcd devinfo */
extern char *hq_lcm_name;
char lcd_buff[128];
/*Jiangrunran@ODM.BSP.TP 2019/10/11 add for tp devinfo*/
extern struct manufacture_info tp_manufac_devinfo;
char tp_buff[128];

/* Yuzhe.Peng@ODM.HQ.BSP.Sensors.Config 2019/10/15 add for sensor information */
char sensor_vendor[5][80];
struct sensor_devinfo hqsensorinfo[] = {
	{"lsm6ds3","ST"},
	{"bmi160","BOSCH"},
	{"bma253","BOSCH"},
	{"lis2doc","ST"},
	{"mmc3530","MICROCHIP"},
	{"memsicd5603x","MICROCHIP"},
	{"akm09918","AKM"},
	{"apds9922","AVAGOTECH"},
	{"STK3335_l","SensorTek"},
	{"stk3x3x_l","SensorTek"},
	{"STK3332_l","SensorTek"},
	{"lsm6ds3","ST"},
	{"stk3x3x_p", "SensorTek"},
	{"tsl2540", "AMS"},
};
#endif

static void match_sub_board(int boardid){
	int MB = boardid & 0xff;
	int KB = (boardid >> 10) & 0x3;
	if(0x0 == KB){
		if((0x0 == MB) || (0x3 == MB) || (0xc == MB) || (0xf == MB) || (0x3c == MB) || (0x3f == MB) || (0xc0 == MB) || (0xc3 == MB)){
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","sub-match");
		}else{
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","sub-unmatch");
		}
	}else if(0x2 == KB){
		if((0x30 == MB) || (0x33 == MB)){
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","sub-match");
		}else{
			sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","sub-unmatch");
		}
	}else{
		sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","UNKOWN");
	}
}

static void get_sub_board(void){
	char *ptr;
	ptr = strstr(saved_command_line, "board_id=");
	if(ptr != 0){
		ptr += strlen("board_id=");
		match_sub_board(simple_strtol(ptr, NULL, 10));
	}else{
		sprintf(buff, "Device version: %s\nDevice manufacture: %s","MTK","UNKOWN");
	}
}

static void register_info(char name[]){
	if(!strcmp(name, "emmc")){
		char manfid[8];
		int manfid_m = card_devinfo->cid.manfid;
		switch(manfid_m){
			case 0x15:
				strcpy(manfid, "SAMSUNG");
				break;
			case 0x45:
				strcpy(manfid, "SANDISK");
				break;
			case 0x13:
				strcpy(manfid, "MICRON");
				break;
			case 0x90:
				strcpy(manfid, "HYNIX");
				break;
			default:
				strcpy(manfid, "UNKOWN");
				break;
		}
		sprintf(buff, "Device version: %s\nDevice manufacture: %s\n",card_devinfo->cid.prod_name, manfid);
		return;
	}else if(!strcmp(name, "emmc_version")){
		
		if(card_devinfo->ext_csd.rev < 7){
			sprintf(buff, "Device version: 0x%x\nDevice manufacture: %s\n",card_devinfo->cid.prv,card_devinfo->cid.prod_name);
		}else{
			sprintf(buff, "Device version: 0x%02x,0x%llx\nDevice manufacture: %s\n", card_devinfo->cid.prv,*(unsigned long long*)card_devinfo->ext_csd.fwrev,card_devinfo->cid.prod_name );
		}
		return;
	}else if(!strcmp(name, "camera_main")){
		sprintf(buff, "Device version: %s\nDevice manufacture: %s",cam_buff.camera_main_name,"UNKOWN");
		return;
	}else if(!strcmp(name, "camera_aux")){
		sprintf(buff, "Device version: %s\nDevice manufacture: %s",cam_buff.camera_aux_name,"UNKOWN");
		return;
	}else if(!strcmp(name, "camera_micro")){
		sprintf(buff, "Device version: %s\nDevice manufacture: %s",cam_buff.camera_micro_name,"UNKOWN");
		return;
	}else if(!strcmp(name, "camera_front")){
		sprintf(buff, "Device version: %s\nDevice manufacture: %s",cam_buff.camera_front_name,"UNKOWN");
		return;
	}else if(!strcmp(name, "lcd")){/* Sunshiyue@ODM.Multimedia.LCD  2019/9/25 add for lcd devinfo */
		if(!strcmp(hq_lcm_name, "ili9881h_hdp_dsi_vdo_inx_al2350")){
			sprintf(lcd_buff, "Device version:%s\nDevice manufacture:%s\n","ili9881h","INX_ILI");
		}else if(!strcmp(hq_lcm_name, "nt36525b_hdp_dsi_vdo_hlt_al2350")){
			sprintf(lcd_buff, "Device version:%s\nDevice manufacture:%s\n","nt36525b","HLT_NVT");
		}else if(!strcmp(hq_lcm_name, "ili9881h_hdp_dsi_vdo_txd_al2350")){
			sprintf(lcd_buff, "Device version:%s\nDevice manufacture:%s\n","ili9881h","TXD_ILI");
		}else if(!strcmp(hq_lcm_name, "nt36525b_hdp_dsi_vdo_djn_al2350")){
			sprintf(lcd_buff, "Device version:%s\nDevice manufacture:%s\n","nt36525b","DJN_NVT");
		}else{
			sprintf(lcd_buff, "Device version:%s\nDevice manufacture:%s\n","UNKNOWN","UNKNOWN");
		}
		return;
	}else if(!strcmp(name, "sub_mainboard")){
		get_sub_board();
		return;
	}else if(!strcmp(name, "tp")){
			sprintf(tp_buff, "Device version:%s\nDevice manufacture:%s\nDevice fw_path:%s",
				    tp_manufac_devinfo.version,tp_manufac_devinfo.manufacture,tp_manufac_devinfo.fw_path);
			return;
	}
}

#ifdef ODM_HQ_EDIT
/* Yuzhe.Peng@ODM.HQ.BSP.Sensors.Config 2019/10/15 add for sensor information */
void hq_register_sensor_info(int sensor_type, char ic_name[]){
	int i;
	for (i = 0; i < sizeof(hqsensorinfo)/sizeof(struct sensor_devinfo); i++) {
			if(!strncmp(ic_name, hqsensorinfo[i].ic_name, strlen(ic_name))) {
				sprintf(sensor_vendor[sensor_type], "Device version:  %s\nDevice manufacture:  %s\n",
					hqsensorinfo[i].ic_name, hqsensorinfo[i].vendor_name);
				break;
			}
	}
	return;
}
EXPORT_SYMBOL_GPL(hq_register_sensor_info);
#endif


static const char * const devinfo_proc_list[] = {
	"Sensor_accel",
	"Sensor_als",
	"Sensor_ps",
	"Sensor_msensor",
	"audio_mainboard",
	"battery",
	"emmc",
	"emmc_version",
	"fastchg",
	"gauge",
	"lcd",
	"mainboard",
	"tp",
	"vooc",
	"camera_main",
	"camera_aux",
	"camera_micro",
	"camera_front",
	"sub_mainboard"
};

#ifdef ODM_HQ_EDIT
/* Yuzhe.Peng@ODM.HQ.BSP.Sensors.Config 2019/10/15 add for sensor information */
HQ_DEVINFO_ATTR(Sensor_accel, "%s\n",sensor_vendor[HQ_ACCEL_DEVICE]);
HQ_DEVINFO_ATTR(Sensor_als, "%s\n",sensor_vendor[HQ_LIGHT_DEVICE]);
HQ_DEVINFO_ATTR(Sensor_ps, "%s\n",sensor_vendor[HQ_PROX_DEVICE]);
HQ_DEVINFO_ATTR(Sensor_msensor, "%s\n",sensor_vendor[HQ_MAG_DEVICE]);
#endif

HQ_DEVINFO_ATTR(audio_mainboard, "%s\n","audio_mainboard");
HQ_DEVINFO_ATTR(battery, "%s\n",battery_name);
HQ_DEVINFO_ATTR(emmc, "%s\n",buff);
HQ_DEVINFO_ATTR(emmc_version, "%s\n",buff);
HQ_DEVINFO_ATTR(fastchg, "%s\n","gauge");
HQ_DEVINFO_ATTR(gauge, "%s\n","gauge");
HQ_DEVINFO_ATTR(lcd, "%s\n", lcd_buff);
HQ_DEVINFO_ATTR(mainboard, "%s\n","mainboard");
HQ_DEVINFO_ATTR(tp, "%s\n",tp_buff);
HQ_DEVINFO_ATTR(vooc, "%s\n","vooc");
HQ_DEVINFO_ATTR(camera_main, "%s",buff);
HQ_DEVINFO_ATTR(camera_aux, "%s",buff);
HQ_DEVINFO_ATTR(camera_micro, "%s",buff);
HQ_DEVINFO_ATTR(camera_front, "%s",buff);
HQ_DEVINFO_ATTR(sub_mainboard, "%s",buff);

static const struct file_operations *proc_fops_list[] = {
	&Sensor_accel_fops,
	&Sensor_als_fops,
	&Sensor_ps_fops,
	&Sensor_msensor_fops,
	&audio_mainboard_fops,
	&battery_fops,
	&emmc_fops,
	&emmc_version_fops,
	&fastchg_fops,
	&gauge_fops,
	&lcd_fops,
	&mainboard_fops,
	&tp_fops,
	&vooc_fops,
	&camera_main_fops,
	&camera_aux_fops,
	&camera_micro_fops,
	&camera_front_fops,
	&sub_mainboard_fops,
};

static int __init devinfo_init(void){
	struct proc_dir_entry *prEntry;
	struct proc_dir_entry *devinfo_dir;
	int i, num;

	devinfo_dir  = proc_mkdir(devinfo_name, NULL);

	if (!devinfo_dir) {
		pr_notice("[%s]: failed to create /proc/%s\n",__func__, devinfo_name);
		return -1;
	}

	num = ARRAY_SIZE(devinfo_proc_list);
	for (i = 0; i < num; i++) {
		prEntry = proc_create(devinfo_proc_list[i], 0444, devinfo_dir, proc_fops_list[i]);
		if (prEntry)
			continue;
		pr_notice("[%s]: failed to create /proc/devinfo/%s\n", __func__, devinfo_proc_list[i]);
	}

	meta_work_around();
	return 0;
}

core_initcall(devinfo_init);
MODULE_AUTHOR("Venco <duwenchao_hq@vanyol.com>");
MODULE_DESCRIPTION("Huaqin Devices Info Driver");
MODULE_LICENSE("GPL");
