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
 
/*led IO�Ĵ�������*/
volatile unsigned long *gpbcon = NULL;
volatile unsigned long *gpbdat = NULL;
volatile unsigned long *gpbup = NULL;
/*���豸��*/
int major;
 
static int led_open(struct inode *inode, struct file *file)
{
    printk("first_drv_open\n");
    /*����GPBCON, GPBDAT, GPBUP�Ĵ���,����mini2440ԭ��ͼ��֪��GPB_IO��*/
    *gpbcon &= ~((0x3 << (5*2)) | (0x3 << (6*2)) | (0x3 << (7*2)) | (0x3 << (8*2)));
    *gpbcon |= (0x1 << (5*2)) | (0x1 << (6*2)) | (0x1 << (7*2)) | (0x1 << (8*2));
    *gpbdat |= (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);
    //��������ر�
    *gpbup |= (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8); 
    return 0;
}
 
static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    printk("first_drv_write1\n");
    int val;
    //�����ݴ��û��ռ俽�����ں˿ռ�
    copy_from_user(&val, buf, count);
 
    if (val == 1) {
        printk("first_drv_write2\n");
        //�����1�Ļ�,�͸����˿�д��͵�ƽ,����led�͵���,��֮����,����һ�����
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
    //��ȡ���豸��
    major = register_chrdev(0, "ledh_drv", &led_fops);
    //ͨ��class_create�Լ�device_create����/dev·���´����豸led
    led_class = class_create(THIS_MODULE, "ledhdrv");
    device_create(led_class, NULL, MKDEV(major, 0), NULL, "ledh");
 
    //�ڴ�ӳ��,����ioremap����,��Ӧ��GPBDAT, GPBUPֻ�����ӵ�λ����
   // gpbcon = (volatile unsigned long *)ioremap(0x56000010, 16);
    gpbcon = ioremap(0x56000010, 16);
    gpbdat = gpbcon + 1;
    gpbup = gpbdat + 1;
 
    return 0;
}
 
static void led_exit(void)
{
    //ע���豸��
    unregister_chrdev(major, "ledh_drv");
    //ɾ���豸
    device_destroy(led_class, MKDEV(major, 0));
    class_destroy(led_class);
    iounmap(gpbcon);
}
 
module_init(led_init);
module_exit(led_exit);

