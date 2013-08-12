#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>


static void mini2440_led_platform_device_release(struct device * dev)
{
    return ;
}


static struct resource mini2440_led_resource[] = {
        [0] = {
                .start = 0x56000010,
                .end   = 0x56000010 + 12,
                .flags = IORESOURCE_MEM
        },
};

static struct platform_device mini2440_platform_device_led = {
        .name           = "mini2440_led_platform_device_driver",
        .id             = -1,
        .num_resources  = ARRAY_SIZE(mini2440_led_resource),
        .resource       = mini2440_led_resource,
        .dev            = {
              .release  = mini2440_led_platform_device_release,
        },
};


static int __init mini2440_led_platform_device_init(void)
{
    printk("mini2440_led_platform_device add ok!\n");
	return platform_device_register(&mini2440_platform_device_led);
}


static void __exit mini2440_led_platform_device_exit(void)
{
    printk("mini2440_led_platform_device remove ok!\n");
	platform_device_unregister(&mini2440_platform_device_led);
}


MODULE_AUTHOR("xxxx");
MODULE_LICENSE("GPL");
module_init(mini2440_led_platform_device_init);
module_exit(mini2440_led_platform_device_exit);


