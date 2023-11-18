#include <asm/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <soc/oppo/oppo_project.h>

#include <linux/io.h>
#include <linux/of.h>
#include <sec_boot_lib.h>
#include <linux/syscalls.h>

/////////////////////////////////////////////////////////////
static struct proc_dir_entry *oppoVersion = NULL;

static ProjectInfoCDTType *format = NULL;
static ProjectInfoCDTType projectInfo = {
	.nProject		= 0,
	.nModem			= 0,
	.nOperator		= 0,
	.nPCBVersion	= 0,
};

#ifdef VENDOR_EDIT
/*Bin.Li@BSP.Bootloader.Bootflows, 2019/05/09, Add for diff manifest*/
static const char* nfc_feature = "nfc_feature";
static const char* feature_src = "/vendor/etc/nfc/com.oppo.nfc_feature.xml";
#endif

#ifdef ODM_HQ_EDIT
/*liujia@ODM_HQ.BSP.driver.board_id 2019.10.24 modify devinfo*/
OPPOVERSION oppoversion;

static int getinfo_for_oppoversion(int board_id){
	int MB = board_id & 0xff;
	int KB = (board_id >> 10) & 0x3;
	int PB = (board_id >> 8) & 0x3;

	if(0x0 == PB){
		strcpy(oppoversion.pcbVersion, "PVT");
	}else if(0x3 == PB){
		strcpy(oppoversion.pcbVersion, "DVT");
	}else if(0x2 == PB){
		strcpy(oppoversion.pcbVersion, "EVT");
	}else{
		strcpy(oppoversion.pcbVersion, "UNKOWN");
	}

	if(0x2 == KB){
		strcpy(oppoversion.Kboard, "1");
	}else if(0x0 == KB){
		strcpy(oppoversion.Kboard, "0");
	}else{
		strcpy(oppoversion.Kboard, "unknow");
	}

	switch(MB){
		case 0x00:
			strcpy(oppoversion.operatorName, "92");
			strcpy(oppoversion.prjVersion, "19181");
			strcpy(oppoversion.modemType, "1");
			strcpy(oppoversion.Mboard, "0000");
			break;
		case 0x03:
			strcpy(oppoversion.operatorName, "92");
			strcpy(oppoversion.prjVersion, "19181");
			strcpy(oppoversion.modemType, "1");
			strcpy(oppoversion.Mboard, "0001");
			pr_err("Modify board_id : remove mobile customization\n");
			break;
		case 0x0c:
			strcpy(oppoversion.operatorName, "94");
			strcpy(oppoversion.prjVersion, "19581");
			strcpy(oppoversion.modemType, "3");
			strcpy(oppoversion.Mboard, "0010");
			break;
		case 0x0f:
			strcpy(oppoversion.operatorName, "95");
			strcpy(oppoversion.prjVersion, "19581");
			strcpy(oppoversion.modemType, "4");
			strcpy(oppoversion.Mboard, "0011");
			break;
		case 0x30:
			strcpy(oppoversion.operatorName, "96");
			strcpy(oppoversion.prjVersion, "19581");
			strcpy(oppoversion.modemType, "5");
			strcpy(oppoversion.Mboard, "0100");
			break;
		case 0x33:
			strcpy(oppoversion.operatorName, "97");
			strcpy(oppoversion.prjVersion, "19581");
			strcpy(oppoversion.modemType, "6");
			strcpy(oppoversion.Mboard, "0101");
			break;
		case 0x3c:
			strcpy(oppoversion.operatorName, "98");
			strcpy(oppoversion.prjVersion, "19581");
			strcpy(oppoversion.modemType, "7");
			strcpy(oppoversion.Mboard, "0110");
			break;
		case 0x3f:
			strcpy(oppoversion.operatorName, "99");
			strcpy(oppoversion.prjVersion, "19581");
			strcpy(oppoversion.modemType, "8");
			strcpy(oppoversion.Mboard, "0111");
			break;
		case 0xc0:
			strcpy(oppoversion.operatorName, "100");
			strcpy(oppoversion.prjVersion, "19581");
			strcpy(oppoversion.modemType, "9");
			strcpy(oppoversion.Mboard, "1000");
			break;
		case 0xc3:
			strcpy(oppoversion.operatorName, "101");
			strcpy(oppoversion.prjVersion, "19581");
			strcpy(oppoversion.modemType, "10");
			strcpy(oppoversion.Mboard, "1001");
			break;

		default:
			strcpy(oppoversion.operatorName, "unknow");
			strcpy(oppoversion.prjVersion, "unknow");
			strcpy(oppoversion.modemType, "unknow");
			strcpy(oppoversion.Mboard, "unknow");
			break;
	}
	return 0;
}

static void set_oppoversion_init(void){
	strcpy(oppoversion.modemType, "unknow");
	strcpy(oppoversion.pcbVersion, "unknow");
	strcpy(oppoversion.operatorName, "unknow");
	strcpy(oppoversion.Kboard, "unknow");
	strcpy(oppoversion.Mboard, "unknow");
	strcpy(oppoversion.prjVersion, "unknow");
}

static unsigned int round_kbytes_to_readable_mbytes(unsigned int k){
	unsigned int r_size_m = 0;
	unsigned int in_mega = k/1024;

	if(in_mega > 64*1024){ //if memory is larger than 64G
		r_size_m = 128*1024; // It should be 128G
	}else if(in_mega > 32*1024){  //larger than 32G
		r_size_m = 64*1024; //should be 64G
	}else if(in_mega > 16*1024){
		r_size_m = 32*1024;
	}else if(in_mega > 8*1024){
		r_size_m = 16*1024;
	}else if(in_mega > 6*1024){
		r_size_m = 8*1024;
	}else if(in_mega > 4*1024){
		r_size_m = 6*1024;  //RAM may be 6G
	}else if(in_mega > 3*1024){
		r_size_m = 4*1024;
	}else if(in_mega > 2*1024){
		r_size_m = 3*1024; //RAM may be 3G
	}else if(in_mega > 1024){
		r_size_m = 2*1024;
	}else if(in_mega > 512){
		r_size_m = 1024;
	}else if(in_mega > 256){
		r_size_m = 512;
	}else if(in_mega > 128){
		r_size_m = 256;
	}else{
		k = 0;
	}
	return r_size_m;
}

#endif


static unsigned int init_project_version(void)
{
	struct device_node *np = NULL;
	int ret = 0;
#ifdef ODM_HQ_EDIT
/*liujia@ODM_HQ.BSP.driver.board_id 2019.10.24 modify devinfo*/
	char *ptr;
#endif

    printk("init_project_version start\n");
	if(format == NULL)
		format = &projectInfo;

#ifdef ODM_HQ_EDIT
/*liujia@ODM_HQ.BSP.driver.board_id 2019.10.24 modify devinfo*/
	ptr = strstr(saved_command_line, "board_id=");
	if(ptr != 0){
		ptr += strlen("board_id=");
		getinfo_for_oppoversion(simple_strtol(ptr, NULL, 10));
	}else{
		set_oppoversion_init();
	}
#endif

	np = of_find_node_by_name(NULL, "oppo_project");
	if(!np){
		printk("init_project_version error1");
		return 0;
	}

	ret = of_property_read_u32(np,"nProject",&(format->nProject));
	if(ret)
	{
		printk("init_project_version error2");
		return 0;
	}

	ret = of_property_read_u32(np,"nModem",&(format->nModem));
	if(ret)
	{
		printk("init_project_version error3");
		return 0;
	}

	ret = of_property_read_u32(np,"nOperator",&(format->nOperator));
	if(ret)
	{
		printk("init_project_version error4");
		return 0;
	}

	ret = of_property_read_u32(np,"nPCBVersion",&(format->nPCBVersion));
	if(ret)
	{
		printk("init_project_version error5");
		return 0;
	}
	printk("KE Version Info :Project(%d) Modem(%d) Operator(%d) PCB(%d)\n",
		format->nProject,format->nModem,format->nOperator,format->nPCBVersion);

	return format->nProject;
}

unsigned int get_project(void)
{
	if(format)
		return format->nProject;
	else
		return init_project_version();
	return 0;
}

EXPORT_SYMBOL(get_project);

unsigned int is_project(OPPO_PROJECT project )
{
	return (get_project() == project?1:0);
}

unsigned int get_PCB_Version(void)
{
	if(format)
		return format->nPCBVersion;
	return 0;
}

unsigned int get_Modem_Version(void)
{
	if(format)
		return format->nModem;
	return 0;
}

unsigned int get_Operator_Version(void)
{
	if (format == NULL) {
		init_project_version();
	}
	if(format)
		return format->nOperator;
	return 0;
}

#ifdef VENDOR_EDIT
/*Bin.Li@BSP.Bootloader.Bootflows, 2019/05/09, Add for diff manifest*/
static int __init update_feature(void)
{
	mm_segment_t fs;
	fs = get_fs();
	pr_err("update_feature, Operator Version [%d]", get_Operator_Version());
	set_fs(KERNEL_DS);
	if (oppoVersion) {
		if (get_Operator_Version() == OPERATOR_19305_CARRIER) {
			proc_symlink(nfc_feature, oppoVersion, feature_src);
		}
	}
	set_fs(fs);
	return 0;
}
late_initcall(update_feature);
#endif

static ssize_t prjVersion_read_proc(struct file *file, char __user *buf,
		size_t count,loff_t *off)
{
	char page[256] = {0};
	int len = 0;

#ifdef ODM_HQ_EDIT
/*liujia@ODM_HQ.BSP.driver.board_id 2019.10.24 modify devinfo*/
		len = sprintf(page,"%s",oppoversion.prjVersion);
#else
		len = sprintf(page,"%d",get_PCB_Version());
#endif

	if(len > *off)
		len -= *off;
	else
		len = 0;

	if(copy_to_user(buf,page,(len < count ? len : count))){
		return -EFAULT;
	}
	*off += len < count ? len : count;
	return (len < count ? len : count);
}

static struct file_operations prjVersion_proc_fops = {
	.read = prjVersion_read_proc,
	.write = NULL,
};


static ssize_t pcbVersion_read_proc(struct file *file, char __user *buf,
		size_t count,loff_t *off)
{
	char page[256] = {0};
	int len = 0;
#ifdef ODM_HQ_EDIT
/*liujia@ODM_HQ.BSP.driver.board_id 2019.10.24 modify devinfo*/
	len = sprintf(page,"%s",oppoversion.pcbVersion);
#else
	len = sprintf(page,"%d",get_PCB_Version());
#endif
	if(len > *off)
	   len -= *off;
	else
	   len = 0;

	if(copy_to_user(buf,page,(len < count ? len : count))){
	   return -EFAULT;
	}
	*off += len < count ? len : count;
	return (len < count ? len : count);

}

static struct file_operations pcbVersion_proc_fops = {
	.read = pcbVersion_read_proc,
};


static ssize_t operatorName_read_proc(struct file *file, char __user *buf,
		size_t count,loff_t *off)
{
	char page[256] = {0};
	int len = 0;

#ifdef ODM_HQ_EDIT
/*liujia@ODM_HQ.BSP.driver.board_id 2019.10.24 modify devinfo*/
	len = sprintf(page,"%s",oppoversion.operatorName);
#else
	len = sprintf(page,"%d",get_PCB_Version());
#endif

	if(len > *off)
	   len -= *off;
	else
	   len = 0;

	if(copy_to_user(buf,page,(len < count ? len : count))){
	   return -EFAULT;
	}
	*off += len < count ? len : count;
	return (len < count ? len : count);

}

static struct file_operations operatorName_proc_fops = {
	.read = operatorName_read_proc,
};


static ssize_t modemType_read_proc(struct file *file, char __user *buf,
		size_t count,loff_t *off)
{
	char page[256] = {0};
	int len = 0;

#ifdef ODM_HQ_EDIT
/*liujia@ODM_HQ.BSP.driver.board_id 2019.10.24 modify devinfo*/
	len = sprintf(page,"%s",oppoversion.modemType);
#else
	len = sprintf(page,"%d",get_PCB_Version());
#endif

	if(len > *off)
	   len -= *off;
	else
	   len = 0;

	if(copy_to_user(buf,page,(len < count ? len : count))){
	   return -EFAULT;
	}
	*off += len < count ? len : count;
	return (len < count ? len : count);

}

static struct file_operations modemType_proc_fops = {
	.read = modemType_read_proc,
};

static ssize_t secureType_read_proc(struct file *file, char __user *buf,
		size_t count,loff_t *off)
{
	char page[256] = {0};
	int len = 0;
	int secure_oem_config = 0;
	secure_oem_config = sec_schip_enabled();
	printk(KERN_EMERG "secure_oem_config %d\n", secure_oem_config);

	len = sprintf(page,"%d", secure_oem_config);

	if(len > *off)
	   len -= *off;
	else
	   len = 0;

	if(copy_to_user(buf,page,(len < count ? len : count))){
	   return -EFAULT;
	}
	*off += len < count ? len : count;
	return (len < count ? len : count);

}


static struct file_operations secureType_proc_fops = {
	.read = secureType_read_proc,
};

/*Yang.Tan@BSP.Fingerprint.Secure 2018/12/17 Add serialID for fastboot unlock*/
#define SERIALNO_LEN 16
extern char *saved_command_line;
static ssize_t serialID_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;
        char * ptr;
        char serialno[SERIALNO_LEN+1] = {0};

        ptr = strstr(saved_command_line, "androidboot.serialno=");
        ptr += strlen("androidboot.serialno=");
        strncpy(serialno, ptr, SERIALNO_LEN);
        serialno[SERIALNO_LEN] = '\0';

        len = sprintf(page, "0x%s", serialno);
        if (len > *off) {
                len -= *off;
        }
        else{
                len = 0;
        }

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }

        *off += len < count ? len : count;
        return (len < count ? len : count);
}

struct file_operations serialID_proc_fops = {
        .read = serialID_read_proc,
};

#ifdef ODM_HQ_EDIT
/*liujia@ODM_HQ.BSP.driver.board_id 2019.10.24 modify devinfo*/

static ssize_t kboard_read_proc(struct file *file, char __user *buf,
		size_t count,loff_t *off)
{
	char page[256] = {0};
	int len = 0;

	len = sprintf(page,"%s",oppoversion.Kboard);

	if(len > *off)
		len -= *off;
	else
		len = 0;

	if(copy_to_user(buf,page,(len < count ? len : count))){
		return -EFAULT;
	}
	*off += len < count ? len : count;
	return (len < count ? len : count);
}

static struct file_operations kboard_proc_fops = {
	.read = kboard_read_proc,
	.write = NULL,
};

static ssize_t mainboard_read_proc(struct file *file, char __user *buf,
		size_t count,loff_t *off)
{
	char page[256] = {0};
	int len = 0;

	len = sprintf(page,"%s",oppoversion.Mboard);

	if(len > *off)
		len -= *off;
	else
		len = 0;

	if(copy_to_user(buf,page,(len < count ? len : count))){
		return -EFAULT;
	}
	*off += len < count ? len : count;
	return (len < count ? len : count);
}

static struct file_operations mainboard_proc_fops = {
	.read = mainboard_read_proc,
	.write = NULL,
};

static ssize_t bootMode_read_proc(struct file *file, char __user *buf,
                size_t count, loff_t *off)
{
        char page[256] = {0};
        int len = 0;
        char * ptr;

	ptr = strstr(saved_command_line, "boot_reason=");
	if(ptr != 0){
		ptr += strlen("boot_reason=");
		len = sprintf(page, "%ld", simple_strtol(ptr, NULL, 10));
	}else{
		len = sprintf(page, "%s","UNKOWN");
	}
        if (len > *off) {
                len -= *off;
        }
        else{
                len = 0;
        }

        if (copy_to_user(buf, page, (len < count ? len : count))) {
                return -EFAULT;
        }

        *off += len < count ? len : count;
        return (len < count ? len : count);
}

struct file_operations bootMode_proc_fops = {
        .read = bootMode_read_proc,
};

#define K(x) ((x) << (PAGE_SHIFT - 10))

static ssize_t ramSize_read_proc(struct file *file, char __user *buf,
		size_t count,loff_t *off)
{
	char page[256] = {0};
	int len = 0;
	struct sysinfo i;
	si_meminfo(&i);

	if(round_kbytes_to_readable_mbytes(K(i.totalram)) >= 1024){
		len = sprintf(page,"%d",round_kbytes_to_readable_mbytes(K(i.totalram))/1024);
	}else{
		len = sprintf(page,"%dMB",round_kbytes_to_readable_mbytes(K(i.totalram)));
	}

	if(len > *off)
		len -= *off;
	else
		len = 0;

	if(copy_to_user(buf,page,(len < count ? len : count))){
		return -EFAULT;
	}
	*off += len < count ? len : count;
	return (len < count ? len : count);
}

static struct file_operations ramSize_proc_fops = {
	.read = ramSize_read_proc,
	.write = NULL,
};

#endif

static int __init oppo_project_init(void)
{
	int ret = 0;
	struct proc_dir_entry *pentry;

	if(oppoVersion) //if oppoVersion is not null means this proc dir has inited
	{
		return ret;
	}
        //#ifdef VENDO_EDIT
        //#Haoran.Zhang@PSW.TECH.OppoFeature.Customize.1881073, 2018/08/15, Add for use avb odm image
        proc_mkdir("euclid",NULL);
        proc_mkdir("euclid/custom", NULL);
        proc_mkdir("euclid/version", NULL);
        proc_mkdir("euclid/product", NULL);
        //endif

	oppoVersion =  proc_mkdir("oppoVersion", NULL);

	if(!oppoVersion) {
		pr_err("can't create oppoVersion proc\n");
		goto ERROR_INIT_VERSION;
	}
	pentry = proc_create("prjVersion", S_IRUGO, oppoVersion, &prjVersion_proc_fops);
	if(!pentry) {
		pr_err("create prjVersion proc failed.\n");
		goto ERROR_INIT_VERSION;
	}
	pentry = proc_create("pcbVersion", S_IRUGO, oppoVersion, &pcbVersion_proc_fops);
	if(!pentry) {
		pr_err("create pcbVersion proc failed.\n");
		goto ERROR_INIT_VERSION;
	}
	pentry = proc_create("operatorName", S_IRUGO, oppoVersion, &operatorName_proc_fops);
	if(!pentry) {
		pr_err("create operatorName proc failed.\n");
		goto ERROR_INIT_VERSION;
	}
	pentry = proc_create("modemType", S_IRUGO, oppoVersion, &modemType_proc_fops);
	if(!pentry) {
		pr_err("create modemType proc failed.\n");
		goto ERROR_INIT_VERSION;
	}
	pentry = proc_create("secureType", S_IRUGO, oppoVersion, &secureType_proc_fops);
	if(!pentry) {
		pr_err("create secureType proc failed.\n");
		goto ERROR_INIT_VERSION;
	}
/*Yang.Tan@BSP.Fingerprint.Secure 2018/12/17 Add serialID for fastboot unlock*/
	pentry = proc_create("serialID", S_IRUGO, oppoVersion, &serialID_proc_fops);
	if(!pentry) {
		pr_err("create serialID proc failed.\n");
		goto ERROR_INIT_VERSION;
	}

#ifdef ODM_HQ_EDIT
/*liujia@ODM_HQ.BSP.driver.board_id 2019.10.24 modify devinfo*/

pentry = proc_create("kboard", S_IRUGO, oppoVersion, &kboard_proc_fops);
if(!pentry) {
	pr_err("create kboard proc failed.\n");
	goto ERROR_INIT_VERSION;
}
pentry = proc_create("mainboard", S_IRUGO, oppoVersion, &mainboard_proc_fops);
if(!pentry) {
	pr_err("create mainboard proc failed.\n");
	goto ERROR_INIT_VERSION;
}
pentry = proc_create("bootMode", S_IRUGO, oppoVersion, &bootMode_proc_fops);
if(!pentry) {
	pr_err("create bootMode proc failed.\n");
	goto ERROR_INIT_VERSION;
}
pentry = proc_create("ramSize", S_IRUGO, oppoVersion, &ramSize_proc_fops);
if(!pentry) {
	pr_err("create ramSize proc failed.\n");
	goto ERROR_INIT_VERSION;
}

#endif


	return ret;
ERROR_INIT_VERSION:
	//remove_proc_entry("oppoVersion", NULL);
	return -ENOENT;
}

static int __init boot_oppo_project_core(void)
{
	init_project_version();
	oppo_project_init();
	return 0;
}

core_initcall(boot_oppo_project_core);

MODULE_DESCRIPTION("OPPO project version");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Joshua <gyx@oppo.com>");
