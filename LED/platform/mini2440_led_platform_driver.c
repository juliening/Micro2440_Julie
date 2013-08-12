#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>

#define GLOBAL_LED_MAJOR  250

static unsigned int global_led_major = GLOBAL_LED_MAJOR; 
static struct cdev *led_cdev = NULL; 
static struct class *led_class = NULL;

static volatile unsigned long *gpbcon = NULL;
static volatile unsigned long *gpbdat = NULL; 
static volatile unsigned long *gpbup = NULL;


#if 0
static int mini2440_led_open(struct inode * inode,struct file * file)
{
	printk("mini2440_open[kernel_space]\n");
	*gpfcon &=~((0x3<<0) | (0x3<<8) |(0x3<<10) |(0x3<<12)|(0x3<<14));
*gpfcon |= (0x1<<0) | (0x1<<8) |(0x1<<10) |(0x1<<12)|(0x1<<14);
return 0;
}
#endif

static int mini2440_led_open(struct inode * inode,struct file * file)
{
	printk("mini2440_open[kernel_space]\n");
	*gpbcon &= ~((0x3 << (5*2)) | (0x3 << (6*2)) | (0x3 << (7*2)) | (0x3 << (8*2)));
	*gpbcon |= (0x1 << (5*2)) | (0x1 << (6*2)) | (0x1 << (7*2)) | (0x1 << (8*2));
	*gpbdat |= (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);

	*gpbup |= (1 << 5) | (1 << 6) | (1 << 7) | (1 << 8);

	
	return  0;
}



static ssize_t mini2440_led_read(struct file * file,const char __user * in,size_t size,loff_t * off)
{
	printk("mini2440_read[kernel_space]\n");
	return 0;
}


#if 0
static ssize_t mini2440_led_write(struct file * file,const char __user * in,size_t size,loff_t * off)
{
    int ret;
	char ker_buf;
	printk("mini2440_write[kernel_space]\n");
	ret = copy_from_user(&ker_buf,in,size);
	printk("ker_buf =%d\n",ker_buf);
	if(ker_buf)
	{
		*gpfdat &=~((0x1<<4)|(0x1<<5)|(0x1<<6)|(0x1<<7));
       	*gpfdat |= (0x1<<0);
	}
	else
	{
		*gpfdat |=(0x1<<4)|(0x1<<5)|(0x1<<6)|(0x1<<7);
		*gpfdat &= ~(0x1<<0);
	}
	return 0;
}
#endif


static ssize_t mini2440_led_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
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



struct file_operations led_fops = {
.owner = THIS_MODULE,
.open  = mini2440_led_open,
.read  = mini2440_led_read,
.write = mini2440_led_write,
};

static int __devinit mini2440_led_probe(struct platform_device *pdev)
{
	int ret;
	int err;
	dev_t devno;
	struct resource *pIORESOURCE_MEM;
	devno = MKDEV(global_led_major,0);
	printk(KERN_ALERT"mini2440_led_probe!\n");
	if (devno) {
	ret = register_chrdev_region(devno,1,"mini2440_led_platfor_driver");
	} else {
	ret = alloc_chrdev_region(&devno,0,1,"mini2440_led_platfor_driver");
	global_led_major = MAJOR(devno);
	}
	if (ret < 0) {
	return ret;
	}
	led_cdev = cdev_alloc();
	cdev_init(led_cdev,&led_fops);
	led_cdev->owner = THIS_MODULE;
	err = cdev_add(led_cdev,devno,1);
	led_class = class_create(THIS_MODULE,"mini2440_led_platfor_driver");
	device_create(led_class,NULL,MKDEV(global_led_major,0),NULL,"platfor_driver_for_mini2440_led");
	pIORESOURCE_MEM = platform_get_resource(pdev,IORESOURCE_MEM,0);
	gpbcon = ioremap(pIORESOURCE_MEM->start,pIORESOURCE_MEM->end - pIORESOURCE_MEM->start);
	gpbdat = gpbcon + 1;
	gpbup  = gpbcon + 2;
	if (err) {
	printk(KERN_NOTICE"Error %d adding led_cdev",err);
	return -1;
	} else {
	printk(KERN_NOTICE"platform_driver_for_mini2440_led init ok!\n");
	return 0;
	}
}



static int __devexit mini2440_led_remove(struct platform_device *pdev)
{
	printk("mini2440_led_remove!\n");
	cdev_del(led_cdev);
	iounmap(gpbcon);
	unregister_chrdev_region(MKDEV(global_led_major,0),1);
	device_destroy(led_class, MKDEV(global_led_major,0));
	class_destroy(led_class);
	return 0;
}


static struct platform_driver mini2440_led_platform_driver = {
	.probe  = mini2440_led_probe,
	.remove = __devexit_p(mini2440_led_remove),
	.driver = {
				.name = "mini2440_led_platform_device_driver",
				.owner = THIS_MODULE,
			}
};

static int __init mini2440_led_platform_driver_init(void)
{
    printk("platform_driver_for_mini2440_led init\n");
	return platform_driver_register(&mini2440_led_platform_driver);
}

static void __exit mini2440_led_platform_driver_exit(void)
{
    printk("platform_driver_for_mini2440_led exit\n");
	platform_driver_unregister(&mini2440_led_platform_driver);
}

MODULE_AUTHOR("xxx");
MODULE_LICENSE("GPL");
module_param(global_led_major,int,S_IRUGO);
module_init(mini2440_led_platform_driver_init);
module_exit(mini2440_led_platform_driver_exit);


