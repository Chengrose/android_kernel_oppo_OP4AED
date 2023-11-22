ifeq ($(CONFIG_OPPO_CHARGER_MTK),y)
subdir-ccflags-y += -Werror
subdir-ccflags-y += -Wno-unused-variable
endif

obj-y      += charger_ic/
obj-y      += gauge_ic/
obj-y      += vooc_ic/
obj-y      += adapter_ic/

obj-y	+= oppo_charger.o
obj-y	+= oppo_gauge.o
obj-y	+= oppo_vooc.o
obj-y	+= oppo_adapter.o
obj-y	+= oppo_short.o

#shengjiang@ODM.HQ.BSP.CHG.Basic 2019.11.12 add disable ATM hiz mode
ifeq (yes,$(strip $(HQ_COMPILE_FACTORY_VERSION)))
   subdir-ccflags-y += -DHQ_COMPILE_FACTORY_VERSION
endif