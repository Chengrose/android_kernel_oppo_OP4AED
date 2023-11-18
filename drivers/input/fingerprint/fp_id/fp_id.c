#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/export.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <mt-plat/mtk_devinfo.h>

#define PROC_BOARD_ID_FILE "fp_id"
#define BOARD_ID_UNKNOW 1

static struct proc_dir_entry *entry = NULL;

extern char *saved_command_line;

static int board_id_proc_show(struct seq_file *file, void* data)
{
	char temp[40] = {0};
	pr_err("fp_id read");
	if(strstr(saved_command_line,"fingerprint=egis")){
		sprintf(temp, "%s\n", "et520");
		seq_printf(file, "%s\n", temp);
	}else if(strstr(saved_command_line,"fingerprint=silead")){
		sprintf(temp, "%s\n", "gsl6250");
		seq_printf(file, "%s\n", temp);
	}else{
		sprintf(temp, "%s\n", "unknown");
		seq_printf(file, "%s\n", temp);
	}
	return 0;
}

static int board_id_proc_open (struct inode* inode, struct file* file)
{
	pr_err("fp_id open");
	return single_open(file, board_id_proc_show, inode->i_private);
}

static const struct file_operations board_id_proc_fops =
{
	.open = board_id_proc_open,
	.read = seq_read,
};

static int __init proc_board_id_init(void)
{
	pr_err("fp_id init");
    
	
	entry = proc_create(PROC_BOARD_ID_FILE, 0644, NULL, &board_id_proc_fops);
	if (entry == NULL)
	{
		pr_err("[%s]: create_proc_entry entry failed\n", __func__);
	}

	return 0;
}

static void __exit proc_board_id_exit(void)
{
	printk("proc_fp_id_exit\n");
}

late_initcall(proc_board_id_init);
module_exit(proc_board_id_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Get fp Id driver");
