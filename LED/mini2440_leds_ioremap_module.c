#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
//#include <asm/irq.h>
//#include <asm/arch/regs-gpio.h>
//#include <asm/hardware.h>
#include <asm/io.h>
#include <mach/irqs.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/device.h>
 
MODULE_LICENSE("GPL");
 
struct class *led_class;
//struct class_device *led_dev;
 
/*led IO寄存器定义*/
volatile unsigned long *gpbcon = NULL;
volatile unsigned long *gpbdat = NULL;
volatile unsigned long *gpbup = NULL;
/*主设备号*/
int major;
 
static int led_open(struct inode *inode, struct file *file)
{
    printk("first_drv_open\n");
    /*配置GPBCON, GPBDAT, GPBUP寄存器,根据mini2440原理图可知是GPB_IO口*/
    *gpbcon &= ~((0x3 << (5*2)) | (0x3 << (6*2)) | (0x3 << (7*2)) | (0x3 << (8*2)));
    *gpbcon |= (0x1 << (5*2)) | (0x1 << (6*2)) | (0x1 << (7*2)) | (0x1 << (8*2));
    *gpbdat |= (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);
    //上拉电阻关闭
    *gpbup |= (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8); 
    return 0;
}
 
static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    printk("first_drv_write1\n");
    int val;
    //将数据从用户空间拷贝至内核空间
    copy_from_user(&val, buf, count);
 
    if (val == 1) {
        printk("first_drv_write2\n");
        //如果是1的话,就给各端口写入低电平,这样led就点亮,反之则灭,如下一种情况
        *gpbdat &= ~((1 << 5) | (1 << 6) | (1 << 7) | (1 << 8));
    } else {
        printk("first_drv_write3\n");
        *gpbdat |= (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);
    }
    return 0;
}
 
static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .write = led_write,
};
 
static int led_init(void)
{
    //获取主设备号
    major = register_chrdev(0, "ledh_drv", &led_fops);
    //通过class_create以及device_create来在/dev路径下创建设备led
    led_class = class_create(THIS_MODULE, "ledhdrv");
    device_create(led_class, NULL, MKDEV(major, 0), NULL, "ledh");
 
    //内存映射,调用ioremap函数,对应的GPBDAT, GPBUP只是增加单位而已
   // gpbcon = (volatile unsigned long *)ioremap(0x56000010, 16);
    gpbcon = ioremap(0x56000010, 16);
    gpbdat = gpbcon + 1;
    gpbup = gpbdat + 1;
 
    return 0;
}
 
static void led_exit(void)
{
    //注销设备号
    unregister_chrdev(major, "ledh_drv");
    //删除设备
    device_destroy(led_class, MKDEV(major, 0));
    class_destroy(led_class);
    iounmap(gpbcon);
}
 
module_init(led_init);
module_exit(led_exit);

