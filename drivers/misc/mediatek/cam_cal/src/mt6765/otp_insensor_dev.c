/***********************************************************
 * ** Copyright (C), 2008-2016, OPPO Mobile Comm Corp., Ltd.
 * ** ODM_HQ_EDIT
 * ** File: - otp_insensor_dev.c
 * ** Description: Source file for CBufferList.
 * **           To allocate and free memory block safely.
 * ** Version: 1.0
 * ** Date : 2018/12/07
 * ** Author: YanKun.Zhai@Mutimedia.camera.driver.otp
 * **
 * ** ------------------------------- Revision History: -------------------------------
 * **   <author>History<data>      <version >version       <desc>
 * **  YanKun.Zhai 2018/12/07     1.0     build this module
 * **
 * ****************************************************************/

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/slab.h>

#include "cam_cal.h"
#include "cam_cal_define.h"
#include "cam_cal_list.h"
#include "eeprom_i2c_dev.h"

#include "kd_camera_typedef.h"
//#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#define LOG_TAG "OTP_InSensor"
#define LOG_ERR(format,...) pr_err(LOG_TAG "Line:%d  FUNC: %s:  "format,__LINE__,__func__,## __VA_ARGS__)
#define LOG_INFO(format,...) pr_info(LOG_TAG "  %s:  "format,__func__,## __VA_ARGS__)
#define LOG_DEBUG(format,...) pr_debug(LOG_TAG "  %s:  "format,__func__,## __VA_ARGS__)

#define ERROR_I2C       1
#define ERROR_CHECKSUM  2
#define ERROR_READ_FLAG 3




static DEFINE_SPINLOCK(g_spinLock);

static struct i2c_client *g_pstI2CclientG;
/* add for linux-4.4 */
#ifndef I2C_WR_FLAG
#define I2C_WR_FLAG		(0x1000)
#define I2C_MASK_FLAG	(0x00ff)
#endif

extern struct i2c_client *g_pstI2Cclients[I2C_DEV_IDX_MAX] ;


#define MAX_EEPROM_BYTE 0x1FFF
#define CHECKSUM_FLAG_ADDR MAX_EEPROM_BYTE-1
#define READ_FLAG_ADDR MAX_EEPROM_BYTE-2

#define OTP_DATA_GOOD_FLAG 0x88
char g_otp_buf[3][MAX_EEPROM_BYTE] = {{0},{0},{0}};




#define MAX_NAME_LENGTH 20
#define MAX_GROUP_NUM 8
#define MAX_GROUP_ADDR_NUM 3

typedef struct
{
    int group_flag_page;
    int group_flag_addr;
    int group_flag;
    int group_start_page;
    int group_start_addr;
    int group_end_page;
    int group_end_addr;
    int group_checksum_page;
    int group_checksum_addr;
}GROUP_ADDR_INFO;

typedef struct
{
    char group_name[MAX_NAME_LENGTH];
    int group_flag_page;
    int group_flag_addr;
    GROUP_ADDR_INFO group_addr_info[MAX_GROUP_ADDR_NUM];
}GROUP_INFO;


typedef struct
{
    char module_name[MAX_NAME_LENGTH];
    int is_page_read;
    int page_start_addr;
    int page_end_addr;
    int group_num;
    int group_addr_info_num;
    int group_checksum;
    GROUP_INFO group_info[MAX_GROUP_NUM];
    int  (* readFunc) (u8 page,u16 addr, u8 *data);
}OTP_MAP;

char read_module_id(void);
int s5k4h7yx_read_data(u8 page,u16 addr,u8 *data);




OTP_MAP s5k4h7yx_hlt_otp_map = {
        .module_name = "S5K4H7YX_HLT",
        .is_page_read = 1,
        .page_start_addr = 0x0A04,
        .page_end_addr = 0x0A43,
        .group_num = 8,
        .group_addr_info_num = 2,
        .readFunc = s5k4h7yx_read_data,
        .group_checksum = 2,
        .group_info = {
                        {"info",
                            -1,-1,
                            {
                                {21,0x0A10,1,21,0x0A04,21,0x0A0F,21,0x0A11},//12
                                {23,0x0A10,1,23,0x0A04,23,0x0A0F,23,0x0A11},
                            },
                        },
                        {"lsc",
                            0,0x0A3D,
                            {
                                {-1,-1,1,1,0x0A04,6,0x0A2B,-1,-1},//360
                                {-1,-1,3,6,0x0A2C,12,0x0A13,-1,-1},
                            },
                        },
                        {"awb_5100",
                            -1,-1,
                            {
                                 {21,0x0A26,1,21,0x0A12,21,0x0A21,21,0x0A27},//16
                                 {23,0x0A26,1,23,0x0A12,23,0x0A21,23,0x0A27},
                            },
                        },
                        {"awb_4000",
                            -1,-1,
                            {
                                 {21,0x0A3E,1,21,0x0A2A,21,0x0A39,21,0x0A3F},//16
                                 {23,0x0A3E,1,23,0x0A2A,23,0x0A39,23,0x0A3F},
                            },
                        },
                        {"awb_3100",
                            -1,-1,
                            {
                                 {22,0x0A18,1,22,0x0A04,22,0x0A13,22,0x0A19},//16
                                 {24,0x0A18,1,24,0x0A04,24,0x0A13,24,0x0A19},
                            },
                        },
                        {"lightsource_5100",
                            -1,-1,
                            {
                                 {21,0x0A28,1,21,0x0A22,21,0x0A25,21,0x0A29},//4
                                 {23,0x0A28,1,23,0x0A22,23,0x0A25,23,0x0A29},
                            },
                        },
                        {"lightsource_4000",
                            -1,-1,
                            {
                                 {21,0x0A40,1,21,0x0A3A,21,0x0A3D,21,0x0A41},//4
                                 {23,0x0A40,1,23,0x0A3A,23,0x0A3D,23,0x0A41},
                            },
                        },
                        {"lightsource_3100",
                            -1,-1,
                            {
                                 {22,0x0A1A,1,22,0x0A14,22,0x0A17,22,0x0A1B},//4
                                 {24,0x0A1A,1,24,0x0A14,24,0x0A17,24,0x0A1B},
                            },
                        },
            },
};

static int read_reg16_data8(u16 addr, u8 *data)
{
    int  i4RetValue = 0;
    char puSendCmd[2] = {(char)(addr >> 8), (char)(addr & 0xff)};

    spin_lock(&g_spinLock);
        g_pstI2CclientG->addr =
            g_pstI2CclientG->addr & (I2C_MASK_FLAG | I2C_WR_FLAG);
    spin_unlock(&g_spinLock);

    i4RetValue = i2c_master_send(g_pstI2CclientG,
        puSendCmd, 2);
    if (i4RetValue != 2) {
        pr_err("I2C send failed!!, Addr = 0x%x\n", addr);
        return -1;
    }
    i4RetValue = i2c_master_recv(g_pstI2CclientG,
        (char *)data, 1);
    if (i4RetValue != 1) {
        pr_err("I2C read failed!!\n");
        return -1;
    }
    spin_lock(&g_spinLock);
    g_pstI2CclientG->addr = g_pstI2CclientG->addr & I2C_MASK_FLAG;
    spin_unlock(&g_spinLock);

    return 0;
}
static int write_reg16_data8(u16 addr, u8 data)
{

    int  i4RetValue = 0;
    char puSendCmd[3] = {(char)(addr >> 8), (char)(addr & 0xff),
                        (char)(data & 0xff)};
    spin_lock(&g_spinLock);
    g_pstI2CclientG->addr =
            g_pstI2CclientG->addr & (I2C_MASK_FLAG | I2C_WR_FLAG);
    spin_unlock(&g_spinLock);


    i4RetValue = i2c_master_send(g_pstI2CclientG, puSendCmd, 3);

    if (i4RetValue != 3) {
        pr_err("I2C send failed!!, Addr = 0x%x\n",addr);
        return -1;
    }
    spin_lock(&g_spinLock);
    g_pstI2CclientG->addr = g_pstI2CclientG->addr & I2C_MASK_FLAG;
    spin_unlock(&g_spinLock);

    return 0;
}

/*static int write_reg16_data16(u16 addr, u16 data)
{
    int  i4RetValue = 0;
    char puSendCmd[4] = {(char)(addr >> 8), (char)(addr & 0xff),
                        (char)(data >> 8),(char)(data & 0xff)};

    spin_lock(&g_spinLock);
    g_pstI2CclientG->addr =
        g_pstI2CclientG->addr & (I2C_MASK_FLAG | I2C_WR_FLAG);
    spin_unlock(&g_spinLock);

    i4RetValue = i2c_master_send(g_pstI2CclientG, puSendCmd, 4);

    if (i4RetValue != 4) {
        pr_err("I2C send failed!!, Addr = 0x%x\n",addr);
        return -1;
    }
    spin_lock(&g_spinLock);
    g_pstI2CclientG->addr = g_pstI2CclientG->addr & I2C_MASK_FLAG;
    spin_unlock(&g_spinLock);

    return 0;
}*/

static int parse_otp_map_data(OTP_MAP * map,char * data)
{
    int i = 0, j = 0;
    int addr = 0,size = 0,page = 0, curr_addr = 0;
    int  ret = 0;
    char readByte = 0;
    char readByte2 = 0;
    int readByte_checksum = 0;
    int checksum = -1;

    LOG_INFO("module: %s group_num:%d ......",map->module_name,  map->group_num);

    for( i = 0 ; i < map->group_num; i++){

        checksum = 0;
        size = 0;

        LOG_INFO("i:%d, groupinfo: %s,start_addr 0x%04x(%04d)",i, map->group_info[i].group_name,curr_addr,curr_addr);

        if( map->group_info[i].group_flag_addr >= 0 ){

            ret = map->readFunc(map->group_info[i].group_flag_page,map->group_info[i].group_flag_addr,&readByte);
            if(ret < 0){
                LOG_ERR("   read flag error addr 0x%04x",map->group_info[i].group_flag_addr);
                return -ERROR_I2C;
            }
            if((0 == map->group_info[i].group_flag_page) && (0x0A3D == map->group_info[i].group_flag_addr)){
                LOG_INFO("special txd s5k4h7yx read lsc flag %d",readByte);
                readByte = readByte & 0x03;
            }
            for( j = 0; j < map->group_addr_info_num ; j++){

                if(readByte == map->group_info[i].group_addr_info[j].group_flag){
                    LOG_INFO("   found flag ,group index %d flag %d", j, readByte);
                    break;
                }
            }
        }else if((map->is_page_read) && (map->group_info[i].group_flag_addr < 0)){

            for( j = 0; j < map->group_addr_info_num ; j++){
                ret = map->readFunc(map->group_info[i].group_addr_info[j].group_flag_page,
                                        map->group_info[i].group_addr_info[j].group_flag_addr,
                                            &readByte);
                if(ret < 0){
                     LOG_ERR("   read flag error p:0x%x addr 0x%04x",
                                   map->group_info[i].group_addr_info[j].group_flag_page,
                                       map->group_info[i].group_addr_info[j].group_flag_addr);
                 }
                if(readByte == map->group_info[i].group_addr_info[j].group_flag){
                     LOG_INFO("   found flag ,group index %d flag %d", j, readByte);
                     break;
                 }
             }
        }

        if(j ==  map->group_addr_info_num){

            LOG_ERR("   Can't found group flag 0x%x",readByte);
            return -ERROR_READ_FLAG;
        }

        if(!map->is_page_read){
            for( addr = map->group_info[i].group_addr_info[j].group_start_addr;
                    addr <= map->group_info[i].group_addr_info[j].group_end_addr;
                        addr++ ){

                ret = map->readFunc(-1,addr,data);
                if(ret < 0){
                    LOG_ERR("   read data  error");
                }
                LOG_DEBUG("    group: %s, addr: 0x%04x, viraddr: 0x%04x(%04d),  data: 0x%04x(%04d) ",
                            map->group_info[i].group_name,addr,curr_addr,curr_addr,*data,*data);

                checksum += *data;
                curr_addr++;
                size++;
                data++;
            }
        }else if(1 == map->is_page_read){
            page = map->group_info[i].group_addr_info[j].group_start_page;

            if( page == map->group_info[i].group_addr_info[j].group_end_page){

                for(addr = map->group_info[i].group_addr_info[j].group_start_addr;
                        addr <= map->group_info[i].group_addr_info[j].group_end_addr;
                            addr++){

                    ret = map->readFunc(page,addr,data);
                    if(ret < 0){
                        LOG_ERR("   read data  error");
                    }
                    LOG_DEBUG("    group: %s,page %d addr: 0x%04x, viraddr: 0x%04x(%04d),  data: 0x%04x(%04d) ",
                                       map->group_info[i].group_name,page,addr,curr_addr,curr_addr,*data,*data);

                    checksum += *data;
                    curr_addr++;
                    size++;
                    data++;
                }

            }else{
                page = map->group_info[i].group_addr_info[j].group_start_page;
                for( addr = map->group_info[i].group_addr_info[j].group_start_addr;
                        addr <= map->page_end_addr;
                         addr++){

                    ret = map->readFunc(page,addr,data);
                    if(ret < 0){
                        LOG_ERR("   read data  error");
                    }
                    LOG_DEBUG("    group: %s,page %d addr: 0x%04x, viraddr: 0x%04x(%04d),  data: 0x%04x(%04d) ",
                                  map->group_info[i].group_name,page,addr,curr_addr,curr_addr,*data,*data);

                    checksum += *data;
                    curr_addr++;
                    size++;
                    data++;
                }

                for(page++;page < map->group_info[i].group_addr_info[j].group_end_page;page++){

                    for( addr = map->page_start_addr;
                            addr <= map->page_end_addr;
                              addr++){

                        ret = map->readFunc(page,addr,data);
                        if(ret < 0){
                            LOG_ERR("   read data  error");
                        }
                        LOG_DEBUG("    group: %s,page %d addr: 0x%04x, viraddr: 0x%04x(%04d),  data: 0x%04x(%04d) ",
                           map->group_info[i].group_name,page,addr,curr_addr,curr_addr,*data,*data);

                        checksum += *data;
                        curr_addr++;
                        size++;
                        data++;
                    }

                }

                page = map->group_info[i].group_addr_info[j].group_end_page;
                for( addr = map->page_start_addr;
                        addr <=  map->group_info[i].group_addr_info[j].group_end_addr;
                         addr++){
                    ret = map->readFunc(page,addr,data);
                    if(ret < 0){
                        LOG_ERR("   read data  error");
                    }
                    LOG_DEBUG("    group: %s,page %d addr: 0x%04x, viraddr: 0x%04x(%04d),  data: 0x%04x(%04d) ",
                         map->group_info[i].group_name,page,addr,curr_addr,curr_addr,*data,*data);

                    checksum += *data;
                    curr_addr++;
                    size++;
                    data++;

                }

            }
        }/*map->is_page_read*/

       if(map->group_checksum == 0){
            checksum = (checksum % 0xFF )+1;
            ret = map->readFunc(map->group_info[i].group_addr_info[j].group_checksum_page,
                                map->group_info[i].group_addr_info[j].group_checksum_addr,
                                    &readByte);
            if(checksum == readByte){
                LOG_INFO("groupinfo: %s, checksum OK c(%04d) r(%04d)",map->group_info[i].group_name,checksum,readByte);
            }else{
                LOG_ERR("groupinfo: %s, checksum ERROR ret=%d, checksum=%04d readByte=%04d",map->group_info[i].group_name,ret,checksum,readByte);
                ret = -ERROR_CHECKSUM;
                }
       }else if(map->group_checksum == 2){
            checksum = (checksum % 0xFF );
            ret = map->readFunc(map->group_info[i].group_addr_info[j].group_checksum_page,map->group_info[i].group_addr_info[j].group_checksum_addr,&readByte);
            if (0 > map->group_info[i].group_addr_info[j].group_checksum_addr){
                LOG_INFO("333 group_checksum_addr < 0, No need checksum !");
            } else if(checksum == readByte){
                LOG_INFO("333 groupinfo: %s, checksum OK c(%04d) r(%04d)",map->group_info[i].group_name,checksum,readByte);
            }else{
                LOG_ERR("333 groupinfo: %s, checksum ERROR ret=%d, checksum=%04d readByte=%04d",map->group_info[i].group_name,ret,checksum,readByte);
                ret = -ERROR_CHECKSUM;
                }
       }else if(map->group_checksum == 1){
            checksum = checksum % 0xFFFF;
            readByte_checksum = 0;
            ret = map->readFunc(map->group_info[i].group_addr_info[j].group_checksum_page,
                                map->group_info[i].group_addr_info[j].group_checksum_addr,
                                    &readByte2);
            ret = map->readFunc(map->group_info[i].group_addr_info[j].group_checksum_page,
                                map->group_info[i].group_addr_info[j].group_checksum_addr+1,
                                    &readByte);
            readByte_checksum = (readByte2 << 8) | readByte;

            if(checksum == readByte_checksum){
                LOG_INFO("groupinfo: %s, checksum OK c(%04d) r(%04d)",map->group_info[i].group_name,checksum,readByte_checksum);
            }else{
                LOG_ERR("groupinfo: %s, checksum ERROR ret=%d, checksum=%04d readByte=%04d",
                                map->group_info[i].group_name,ret,checksum,readByte_checksum);
                ret = -ERROR_CHECKSUM;
                }

        }
        LOG_INFO("groupinfo: %s,end_addr 0x%04x(%04d) size 0x%04x(%04d)",
                        map->group_info[i].group_name,curr_addr-1,curr_addr-1,size,size);

    }

    return ret;
}


char read_module_id(void)
{
    char module_id = 0;
    g_pstI2CclientG = g_pstI2Cclients[CAM_CAL_SENSOR_IDX_SUB];
    g_pstI2CclientG->addr = 0x20 >> 1;
    s5k4h7yx_read_data(21,0x0A04,&module_id);
    return module_id;
}



int s5k4h7yx_read_data(u8 page,u16 addr,u8 *data)
{
    u8 get_byte = 0;
    int ret = 0;

    write_reg16_data8(0x0A02,page);
    write_reg16_data8(0x0A00,0x01);

    do{
        mdelay(1);
        ret = read_reg16_data8(0x0A01,&get_byte);
    }while((get_byte & 0x01) != 1);

    ret = read_reg16_data8(addr,data);
    write_reg16_data8(0x0A00,0x00);
    return ret;
}

unsigned int s5k4h7yx_read_region(struct i2c_client *client, unsigned int addr,unsigned char *data,unsigned int size)
{
    int ret = 0;
	// yinlili@ODM_HQ Cam.Algo  20200225 modified for coverity issue:67597
	unsigned int reg_addr=0;
	reg_addr = addr;
    LOG_DEBUG("s5k4h7yx_read_region addr:0x%x, size:%d", reg_addr, size);
    if(NULL != data && g_otp_buf[CAM_CAL_SENSOR_IDX_SUB][READ_FLAG_ADDR] ){
        LOG_INFO("from mem addr 0x%x,size %d",reg_addr,size);
        memcpy((void *)data, &g_otp_buf[CAM_CAL_SENSOR_IDX_SUB][reg_addr],size);
        return size;
    }
    if(client != NULL){

        g_pstI2CclientG = client;

    }else if(g_pstI2Cclients[CAM_CAL_SENSOR_IDX_SUB] != NULL){

        g_pstI2CclientG = g_pstI2Cclients[CAM_CAL_SENSOR_IDX_SUB];

        g_pstI2CclientG->addr = 0x20 >> 1;
    } else {
        LOG_ERR("s5k4h7yx_read_region no client.");
        return -1;
    }
    ret = parse_otp_map_data(&s5k4h7yx_hlt_otp_map, &g_otp_buf[CAM_CAL_SENSOR_IDX_SUB][0]);
    if(!ret){
        g_otp_buf[CAM_CAL_SENSOR_IDX_SUB][CHECKSUM_FLAG_ADDR] = OTP_DATA_GOOD_FLAG;
        g_otp_buf[CAM_CAL_SENSOR_IDX_SUB][READ_FLAG_ADDR] = 1;
    }
    if(NULL != data){
        memcpy((void *)data, &g_otp_buf[CAM_CAL_SENSOR_IDX_SUB][addr], size);
    }
    return ret;
}
