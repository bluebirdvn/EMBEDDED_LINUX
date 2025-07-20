#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "gpio.h"


#define AUTHOR "Tuan"
#define DESCRIPTION "control gpio 67 in beaglebone board"
#define VERSION "1.0.0"


static int __init hello_init(void) 
{
    printk(KERN_INFO "GPIO 67 initilizing...\n");
    
    gpio_init();

    gpio_set();
    
    return 0;

}

static void __exit hello_exit(void) 
{
   gpio_clear();

   gpio_exit();

    printk(KERN_INFO "goodbye.\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_VERSION(VERSION);