obj-y  +=  oppo_project/
obj-$(CONFIG_OPPO_DEVICE_IFNO)  +=  oppo_devinfo/
obj-$(CONFIG_OPPO_CUSTOM)  += oppo_custom/
obj-$(CONFIG_OPPO_HYPNUS)     +=  oppo_hypnus/
obj-$(CONFIG_OPPO_HEALTHINFO)     +=  oppo_healthinfo/
obj-$(CONFIG_OPPO_ALS_CALI)   +=  oppo_als_cali_value/

#ifdef VENDOR_EDIT
#Weican.Zhao@PSW.NW.EM, 2019/02/12, Add for OPPO Critical log
#obj-$(CONFIG_OPPO_CRITICALLOG)  +=  oppo_criticallog/
obj-y     +=  oppo_criticallog/
#endif /* VENDOR_EDIT */

#ifdef VENDOR_EDIT
#//Kun.Hu@PSW.TECH 2018/11/15, Add phoenix base
#Kun Hu@TECH.BSP.Stability.PROJECT_PHOENIX, 2019/06/08, Remove phoenix for Self Stability Test Version
ifneq ($(SPECIAL_OPPO_CONFIG),1)
obj-y     +=  oppo_phoenix/
endif
#endif /* VENDOR_EDIT */

#ifdef VENDOR_EDIT
#* wen.luo@PSW.BSP.kernel, 2019/6/25, Add for parsar powoff/powon */
obj-y     +=  oppo_lastpmic/
#endif /* VENDOR_EDIT */