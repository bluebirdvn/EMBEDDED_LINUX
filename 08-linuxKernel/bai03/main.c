#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "gpio.h"

#define AUTHOR "Tuan"
#define DESCRIPTION "init and exit kernel module"
#define VERSION "1.0.0"

#define NUM_PAGE 1

struct gpio_dev {
    int size;
    char *kmalloc_ptr;
    dev_t dev_num;
    struct class *gpio_class;
    struct cdev gpio_dev;
};

static struct gpio_dev gpio67;

static int      m_open(struct inode *inode, struct file *file);
static int      m_release(struct inode *inode, struct file *file);
static ssize_t  m_read(struct file *filp, char __user *user_buf, size_t size,loff_t * offset);
static ssize_t  m_write(struct file *filp, const char *user_buf, size_t size, loff_t * offset);

static struct file_operations fops = {
    .owner  = THIS_MODULE,
    .read   = m_read,
    .write  = m_write,
    .open   = m_open,
    .release= m_release,
};

static int m_open(struct inode *inode, struct file *file) 
{
    printk(KERN_INFO "System call open() called.\n");
    return 0;
}

static int m_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "System call closed() called.\n");
    return 0;
}

static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size,loff_t * offset)
{
    size_t read;

    pr_info(KERN_INFO "System call read() called.\n");

    read = (size > gpio67.size - *offset) ? (gpio67.size - *offset) : size;

    if (copy_to_user(user_buf, gpio67.kmalloc_ptr + *offset, read)) {
        return -EFAULT;
    }

    *offset += read;

    return read;
}

static ssize_t m_write(struct file *filp, const char __user *user_buf, size_t size,loff_t * offset)
{
    size_t write;

    pr_info(KERN_INFO "System call write() called.\n");

    write = (size + *offset > NUM_PAGE*PAGE_SIZE) ? (NUM_PAGE*PAGE_SIZE - *offset) : size;

    memset(gpio67.kmalloc_ptr, 0, NUM_PAGE*PAGE_SIZE);

    if (copy_from_user(gpio67.kmalloc_ptr + *offset, user_buf, write) != 0) {
        return -EFAULT;
    }

    pr_info(KERN_INFO "Data from user: %s", gpio67.kmalloc_ptr);

    gpio67.kmalloc_ptr[strcspn(gpio67.kmalloc_ptr, "\n")] = 0;

    // Check data from user to set/clear gpio67
    if (strcmp(gpio67.kmalloc_ptr, "on") == 0) {
        gpio_set();
    } else if (strcmp(gpio67.kmalloc_ptr, "off") == 0) {
        gpio_clear();
    } else {
        pr_warn("Invalid command: %s\n", gpio67.kmalloc_ptr);
    }

    *offset += write;

    gpio67.size = *offset;

    return write;
}

static int __init hello_init(void) 
{
    printk(KERN_INFO "hello world.\n");
    
    if (alloc_chrdev_region(&gpio67.dev_num, 0, 1, "gpio67-cdev") < 0) {
        pr_err("Failed to alloc chrdev to gpio67.\n");
        return -1;
    }

    cdev_init(&gpio67.gpio_dev, &fops);

    if (cdev_add(&gpio67.gpio_dev, gpio67.dev_num, 1) < 0) {
        pr_err("Can't add gpio 67 device to system");
        goto rm_device_numb;
    }

    gpio67.gpio_class = class_create("gpio67-class");
    if (gpio67.gpio_class == NULL) {
        pr_err("Can't create struct class for gpio 67.\n");
        goto rm_device_numb;
    }

    if (device_create(gpio67.gpio_class, NULL, gpio67.dev_num, NULL, "gpio67-device") == NULL) {
        pr_err("Can't create device gpio67.\n");
        goto rm_gpio_class;
    }

    if ((gpio67.kmalloc_ptr = kmalloc(1024, GFP_KERNEL)) == 0) {
        pr_err("Can't allocate memory in kernel.\n");
        goto rm_device;
    }

    if (gpio_init() < 0) {
        pr_err("Failed to intit hardware.\n");
        goto rm_buffer;
    }
    
    pr_info("Major: %d, Minor: %d\n", MAJOR(gpio67.dev_num), MINOR(gpio67.dev_num));
    
    return 0;
rm_buffer:
    kfree(gpio67.kmalloc_ptr);
rm_device:
    device_destroy(gpio67.gpio_class, gpio67.dev_num);
rm_gpio_class:
    class_destroy(gpio67.gpio_class);
rm_device_numb:
    unregister_chrdev_region(gpio67.dev_num, 1);

    return -1;
}

static void __exit hello_exit(void) 
{
    gpio_clear();
    gpio_exit();
    
   if (gpio67.kmalloc_ptr)
        kfree(gpio67.kmalloc_ptr);

    if (gpio67.gpio_class && gpio67.dev_num)
        device_destroy(gpio67.gpio_class, gpio67.dev_num);

    if (gpio67.gpio_class)
        class_destroy(gpio67.gpio_class);

    cdev_del(&gpio67.gpio_dev);
    unregister_chrdev_region(gpio67.dev_num, 1);

    printk(KERN_INFO "goodbye.\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESCRIPTION);
MODULE_VERSION(VERSION);