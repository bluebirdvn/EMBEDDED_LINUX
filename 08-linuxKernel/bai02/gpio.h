#ifndef GPIO_H
#define GPIO_H

#define GPIO_2_BASE_ADDR 0x481AC000
#define GPIO_CTRL 130
#define GPIO_OE 134
#define GPIO_CLEARDATAOUT 190
#define GPIO_SETDATAOUT 194
#define GPIO_67 67

int gpio_init(void);
void gpio_set(void);
void gpio_clear(void);
void gpio_exit(void);

#endif