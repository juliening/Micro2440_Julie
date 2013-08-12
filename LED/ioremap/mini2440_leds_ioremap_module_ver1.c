
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/system.h>


#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/list.h>
#include <linux/pci.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/unistd.h>

#include <linux/ioport.h>


#define DEVICE_NAME "leds_char"
#define GLOBALMEM_MAJOR 210    /*预设的globalmem的主设备号*/
#define GLOBALMEM_SIZE 0x1000
static int globalmem_major = GLOBALMEM_MAJOR;


#define GPBCON_PHY_ADDR 0x56000010
#define GPBDAT_PHY_ADDR 0x56000014

#define LED1_ON   ~(1<<5) //低电平点亮LED
#define LED2_ON   ~(1<<6)
#define LED3_ON   ~(1<<7)
#define LED4_ON   ~(1<<8)
#define LED1_OFF   (1<<5)
#define LED2_OFF   (1<<6)
#define LED3_OFF   (1<<7)
#define LED4_OFF   (1<<8)

#define GPBCON 0x56000010 //寄存器地址（物理地址）
#define GPBDAT 0x56000014
static volatile unsigned long *gpbcon_addr; //经过ioremap映射后的虚拟地址
static volatile unsigned long *gpbdat_addr;

/*globalmem设备结构体*/
struct globalmem_dev {
	struct cdev cdev; /*cdev结构体*/
	unsigned char mem[GLOBALMEM_SIZE]; /*全局内存*/
};

struct globalmem_dev *globalmem_devp; /*设备结构体指针*/

static void Led_port_init(void)
{
   //设置GPB5-GPB8为输出端口
   *gpbcon_addr &= ~((3<<10)|(3<<12)|(3<<14)|(3<<16)); 
   *gpbcon_addr |= (1<<10)|(1<<12)|(1<<14)|(1<<16);
   
   //全亮
   *gpbdat_addr &= LED1_ON & LED2_ON & LED3_ON & LED4_ON;
}
static void led_turn_on(unsigned int led_nu)
{
 switch (led_nu)
 {
  case 1:
   *gpbdat_addr &= LED1_ON;
   break;
  case 2:
   *gpbdat_addr &= LED2_ON;
   break;
  case 3:
   *gpbdat_addr &= LED3_ON;
   break;
  case 4:
   *gpbdat_addr &= LED4_ON;
   break;
  default:
   break;
 }
}
static void led_turn_off(unsigned int led_nu)
{
 switch (led_nu)
 {
  case 1:
   *gpbdat_addr |= LED1_OFF;
   break;
  case 2:
   *gpbdat_addr |= LED2_OFF;
   break;
  case 3:
   *gpbdat_addr |= LED3_OFF;
   break;
  case 4:
   *gpbdat_addr |= LED4_OFF;
   break;
  default:
   break;
 }
}


/*static unsigned long led_table [] = {
	S3C2410_GPB(5),
	S3C2410_GPB(6),
	S3C2410_GPB(7),
	S3C2410_GPB(8),
};

static unsigned int led_cfg_table [] = {
	S3C2410_GPIO_OUTPUT,
	S3C2410_GPIO_OUTPUT,
	S3C2410_GPIO_OUTPUT,
	S3C2410_GPIO_OUTPUT,
};*/

static int sbc2440_leds_ioctl(
	struct inode *inode, 
	struct file *file, 
	unsigned int cmd, 
	unsigned long arg)
{
	switch(cmd) {
	case 0:
	case 1:
		if (arg > 4) {
			return -EINVAL;
		}
		#if 0
		if(arg == 0) //GPB5
		{
			if(cmd == 1) //on
			{
				*GPBDAT_Virtural_Addr &= GPB_LED5_ON;
			}
			else	//off
			{
				*GPBDAT_Virtural_Addr |= GPB_LED5_OFF;
			}
		
		}
		else if(arg == 1) //GPB6
		{
			if(cmd == 1)
			{
				*GPBDAT_Virtural_Addr &= GPB_LED6_ON;
			
			}
			else
			{
				*GPBDAT_Virtural_Addr |= GPB_LED6_OFF;
			}
		
		}
		else if(arg == 2)	//GPB7
		{
			if(cmd == 1)
			{
				*GPBDAT_Virtural_Addr &= GPB_LED7_ON;
			}
			else
			{
				*GPBDAT_Virtural_Addr |= GPB_LED7_OFF;
			}

		}
		else if(arg == 3) // GPB8
		{
			if(cmd == 1)
			{
				*GPBDAT_Virtural_Addr &= GPB_LED8_ON;
			
			}
			else
			{
				*GPBDAT_Virtural_Addr &= GPB_LED8_OFF;

			}

		}	
		#endif
		//s3c2410_gpio_setpin(led_table[arg], !cmd);
		return 0;
	default:
		return -EINVAL;
	}
}


/*文件操作结构体*/
static const struct file_operations globalmem_fops = {
	.owner = THIS_MODULE,
	.ioctl = sbc2440_leds_ioctl,
	
};

int globalmem_open(struct inode *inode, struct file *filp)
{
	/*将设备结构体指针赋值给文件私有数据指针*/
	filp->private_data = globalmem_devp;
	return 0;
}


/*初始化并注册cdev*/
static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
	int err, devno = MKDEV(globalmem_major, index);

	cdev_init(&dev->cdev, &globalmem_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
		printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}


void leds_ioremap_init(void)
{
	/*Set GPB port5,port6,port7,port8 as output ports */
	/* And output the initialized value */
	*GPBCON_Virtural_Addr &= ~((3<<10) | (3<<12) | (3<<14) | (3<<16)); 
	*GPBCON_Virtural_Addr |= (1<<10) | (1<<12) | (1<<14) | (1<<16);

	GPBDAT_Virtural_Addr =  GPB_LED5_OFF | GPB_LED6_OFF | GPB_LED7_OFF
							| GPB_LED8_OFF;

} 
static int __init dev_init(void)
{
	int ret;
	int i;
	
	GPBCON_Virtural_Addr = ioremap(GPBCON_PHY_ADDR,60);

	if(GPBCON_Virtural_Addr == NULL)
	{
		printk(KERN_ERR"Faild to remap register block:GPBCON_Virtural_Addr\n");
	}
	GPBDAT_Virtural_Addr = ioremap(GPBDAT_PHY_ADDR,4);
	if(GPBDAT_Virtural_Addr == NULL)
	{
		printk(KERN_ERR"Faild to remap register block:GPBDAT_Virtural_Addr\n");
	}
	if((GPBCON_Virtural_Addr != NULL) && (GPBDAT_Virtural_Addr != NULL))
	{
		leds_ioremap_init();
	}
	/*for (i = 0; i < 4; i++) {
		s3c2410_gpio_cfgpin(led_table[i], led_cfg_table[i]);
		s3c2410_gpio_setpin(led_table[i], 0);
	}*/

	dev_t devno = MKDEV(globalmem_major, 0);

	/* 申请设备号*/
	if (globalmem_major)
		ret = register_chrdev_region(devno, 1, "leds_char");
	else { /* 动态申请设备号 */
		ret = alloc_chrdev_region(&devno, 0, 1, "leds_char");
		globalmem_major = MAJOR(devno);
	}
	if (ret < 0)
		return ret;
	
	/* 动态申请设备结构体的内存*/
	globalmem_devp = kmalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
	if (!globalmem_devp) {    /*申请失败*/
		ret =  - ENOMEM;
		//goto fail_malloc;
	}

	memset(globalmem_devp, 0, sizeof(struct globalmem_dev));

	globalmem_setup_cdev(globalmem_devp, 0);

	printk (DEVICE_NAME"\tinitialized\n");
	ret = 0;
	return ret;
}

static void __exit dev_exit(void)
{
	//misc_deregister(&misc);
	cdev_del(&globalmem_devp->cdev);   /*注销cdev*/
	kfree(globalmem_devp);     /*释放设备结构体内存*/
	unregister_chrdev_region(MKDEV(globalmem_major, 0), 1); /*释放设备号*/

	iounmap(GPBCON_Virtural_Addr);
	iounmap(GPBDAT_Virtural_Addr);
}

module_init(dev_init);
module_exit(dev_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("FriendlyARM Inc.");
