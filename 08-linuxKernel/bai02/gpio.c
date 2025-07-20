#include "gpio.h"
#include <linux/io.h>
#include <linux/module.h>

#define GPIO_SIZE 0x1000 
#define NUM_GPIO_PER_GPIOX 32
static void __iomem *gpio2_base = NULL;

int gpio_init(void)
{
    if (gpio2_base != NULL) {
        pr_err("GPIO _base inti it not NULL\n");
        return -1;
    }

    gpio2_base = ioremap(GPIO_2_BASE_ADDR, GPIO_SIZE);

    if (!gpio2_base) {
        pr_err("Can't anh xa to register\n");
        return -ENOMEM;
    }

    // config gpio67: OUTPUT
    u32 oe_val = ioread32(gpio2_base + GPIO_OE);;
    // config gpio67: OUTPUT, clear bit: OUPUT
    oe_val &= ~(1 << (GPIO_67 % NUM_GPIO_PER_GPIOX));

    iowrite32(oe_val, gpio2_base + GPIO_OE);

    pr_info("GPIO is configured to output\n");

    return 0;
}

void gpio_set(void)
{
    if (!gpio2_base) {
        pr_err("Can't anh xa to register\n");
        return;
    }

    iowrite32(1 << (GPIO_67 % NUM_GPIO_PER_GPIOX), gpio2_base + GPIO_SETDATAOUT);
    pr_info("SET GPIO 67 in high level\n");
}

void gpio_clear(void)
{
    if (!gpio2_base) {
        pr_err("Can't anh xa to register\n");
        return;
    }

    iowrite32(1 << (GPIO_67 % NUM_GPIO_PER_GPIOX), gpio2_base + GPIO_CLEARDATAOUT);
    pr_info("SET GPIO 67 in low level\n");
}

void gpio_exit(void)
{
    if (gpio2_base) {
        iounmap(gpio2_base);
        gpio2_base = NULL;
        pr_info("FREE gpio.\n");
    }
}


