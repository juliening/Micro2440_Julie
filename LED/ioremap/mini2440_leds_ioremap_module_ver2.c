#include<linux/miscdevice.h>
#include<linux/init.h>
#include<linux/module.h>
#include<linux/delay.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/io.h>


#define DEVICE_NAME "NINY_LED"
//static void __iomem *led_base_addr;
static void __iomem *led_base_addr;

#define GPBCON (*(volatile unsigned long*)(led_base_addr+0x00))//S3C2410_GPBCON
#define GPBDAT (*(volatile unsigned long*)(led_base_addr+0x04))//S3C2410_GPBDAT
#define GPBUP (*(volatile unsigned long*)(led_base_addr+0x08))//S3C2410_GPBUP
static int s3c2440_leds_ioctl(
struct inode *inode,
struct file *file,
unsigned int cmd,
unsigned int arg)
{
switch(cmd){
case 0:
GPBDAT &=~(1<<arg);
break;
case 1:
GPBDAT |= (1<<arg);
break;

default:
return -EINVAL;
}
}


static struct file_operations dev_fops = {
.owner  = THIS_MODULE,
.ioctl  = s3c2440_leds_ioctl,
};
static struct miscdevice misc ={
.minor  = MISC_DYNAMIC_MINOR,
.name = DEVICE_NAME,
.fops = &dev_fops,
};
static int __init dev_init(void)
{
int ret;
int led_cfg_output;
led_base_addr=ioremap(0x56000010,0x60);
if(led_base_addr==NULL){
printk(KERN_ERR"Faild to remap register block\n");
return -ENOENT;
}
led_cfg_output = (GPBCON&0xFFFFFFFF)|(3<<10|3<<12|3<<14|3<<16)&(~(1<<11|1<<13|1<<15|1<<17));
GPBCON  = led_cfg_output;
GPBUP =  0x1E1;
led_cfg_output = GPBDAT&0x21F;
printk("led_cfg_output:%x\n",&led_cfg_output);
GPBDAT = led_cfg_output;
ret = misc_register(&misc);
printk("GPBCTL:%x\n",GPBCON);
printk("GPBDAT:%x\n",GPBDAT);
printk("GPBUP:%x\n",GPBUP);
printk(DEVICE_NAME"\tinitialized\n");
return ret;
}

static void __exit dev_exit(void)
{
int led_cfg_output;
misc_deregister(&misc);
led_cfg_output = GPBDAT|0x1E0;
GPBDAT = led_cfg_output;
iounmap(0x56000010);
}

module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SKY");

