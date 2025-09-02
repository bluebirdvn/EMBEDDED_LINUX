

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#define DEV_NAME       "ssd1306"
#define CLASS_NAME     "ssd1306_class"
#define MAX_LENGTH        256

/* SSD1306 dimensions */
#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64
#define SSD1306_PAGES   (SSD1306_HEIGHT/8)

/* Control bytes */
#define SSD1306_CTRL_CMD   0x00
#define SSD1306_CTRL_DATA  0x40

/* Device struct */
struct ssd1306 {
    struct i2c_client *client;
    struct gpio_desc *reset;
    struct cdev cdev;
    struct class *class;
    struct device *device;
    dev_t devt;

    u8 x_pos, y_pos;
    char message[MAX_LENGTH];
};

static struct ssd1306 *g_ssd1306;

/* ---------- 5x7 ASCII font (32..127) ---------- */
static const u8 font5x7[][5] = {
/* 0x20 ' ' */ {0x00,0x00,0x00,0x00,0x00},
/* 0x21 '!' */ {0x00,0x00,0x5F,0x00,0x00},
/* 0x22 '"'*/  {0x00,0x07,0x00,0x07,0x00},
/* 0x23 '#' */ {0x14,0x7F,0x14,0x7F,0x14},
/* 0x24 '$' */ {0x24,0x2A,0x7F,0x2A,0x12},
/* 0x25 '%' */ {0x23,0x13,0x08,0x64,0x62},
/* 0x26 '&' */ {0x36,0x49,0x55,0x22,0x50},
/* 0x27 '\''*/ {0x00,0x05,0x03,0x00,0x00},
/* 0x28 '(' */ {0x00,0x1C,0x22,0x41,0x00},
/* 0x29 ')' */ {0x00,0x41,0x22,0x1C,0x00},
/* 0x2A '*' */ {0x14,0x08,0x3E,0x08,0x14},
/* 0x2B '+' */ {0x08,0x08,0x3E,0x08,0x08},
/* 0x2C ',' */ {0x00,0x50,0x30,0x00,0x00},
/* 0x2D '-' */ {0x08,0x08,0x08,0x08,0x08},
/* 0x2E '.' */ {0x00,0x60,0x60,0x00,0x00},
/* 0x2F '/' */ {0x20,0x10,0x08,0x04,0x02},
/* 0x30 '0' */ {0x3E,0x51,0x49,0x45,0x3E},
/* 0x31 '1' */ {0x00,0x42,0x7F,0x40,0x00},
/* 0x32 '2' */ {0x42,0x61,0x51,0x49,0x46},
/* 0x33 '3' */ {0x21,0x41,0x45,0x4B,0x31},
/* 0x34 '4' */ {0x18,0x14,0x12,0x7F,0x10},
/* 0x35 '5' */ {0x27,0x45,0x45,0x45,0x39},
/* 0x36 '6' */ {0x3C,0x4A,0x49,0x49,0x30},
/* 0x37 '7' */ {0x01,0x71,0x09,0x05,0x03},
/* 0x38 '8' */ {0x36,0x49,0x49,0x49,0x36},
/* 0x39 '9' */ {0x06,0x49,0x49,0x29,0x1E},
/* 0x3A ':' */ {0x00,0x36,0x36,0x00,0x00},
/* 0x3B ';' */ {0x00,0x56,0x36,0x00,0x00},
/* 0x3C '<' */ {0x08,0x14,0x22,0x41,0x00},
/* 0x3D '=' */ {0x14,0x14,0x14,0x14,0x14},
/* 0x3E '>' */ {0x00,0x41,0x22,0x14,0x08},
/* 0x3F '?' */ {0x02,0x01,0x51,0x09,0x06},
/* 0x40 '@' */ {0x32,0x49,0x79,0x41,0x3E},
/* 0x41 'A' */ {0x7E,0x11,0x11,0x11,0x7E},
/* 0x42 'B' */ {0x7F,0x49,0x49,0x49,0x36},
/* 0x43 'C' */ {0x3E,0x41,0x41,0x41,0x22},
/* 0x44 'D' */ {0x7F,0x41,0x41,0x22,0x1C},
/* 0x45 'E' */ {0x7F,0x49,0x49,0x49,0x41},
/* 0x46 'F' */ {0x7F,0x09,0x09,0x09,0x01},
/* 0x47 'G' */ {0x3E,0x41,0x49,0x49,0x7A},
/* 0x48 'H' */ {0x7F,0x08,0x08,0x08,0x7F},
/* 0x49 'I' */ {0x00,0x41,0x7F,0x41,0x00},
/* 0x4A 'J' */ {0x20,0x40,0x41,0x3F,0x01},
/* 0x4B 'K' */ {0x7F,0x08,0x14,0x22,0x41},
/* 0x4C 'L' */ {0x7F,0x40,0x40,0x40,0x40},
/* 0x4D 'M' */ {0x7F,0x02,0x0C,0x02,0x7F},
/* 0x4E 'N' */ {0x7F,0x04,0x08,0x10,0x7F},
/* 0x4F 'O' */ {0x3E,0x41,0x41,0x41,0x3E},
/* 0x50 'P' */ {0x7F,0x09,0x09,0x09,0x06},
/* 0x51 'Q' */ {0x3E,0x41,0x51,0x21,0x5E},
/* 0x52 'R' */ {0x7F,0x09,0x19,0x29,0x46},
/* 0x53 'S' */ {0x46,0x49,0x49,0x49,0x31},
/* 0x54 'T' */ {0x01,0x01,0x7F,0x01,0x01},
/* 0x55 'U' */ {0x3F,0x40,0x40,0x40,0x3F},
/* 0x56 'V' */ {0x1F,0x20,0x40,0x20,0x1F},
/* 0x57 'W' */ {0x3F,0x40,0x38,0x40,0x3F},
/* 0x58 'X' */ {0x63,0x14,0x08,0x14,0x63},
/* 0x59 'Y' */ {0x07,0x08,0x70,0x08,0x07},
/* 0x5A 'Z' */ {0x61,0x51,0x49,0x45,0x43},
/* 0x5B '[' */ {0x00,0x7F,0x41,0x41,0x00},
/* 0x5C '\\'*/ {0x02,0x04,0x08,0x10,0x20},
/* 0x5D ']' */ {0x00,0x41,0x41,0x7F,0x00},
/* 0x5E '^' */ {0x04,0x02,0x01,0x02,0x04},
/* 0x5F '_' */ {0x40,0x40,0x40,0x40,0x40},
/* 0x60 '`' */ {0x00,0x01,0x02,0x04,0x00},
/* 0x61 'a' */ {0x20,0x54,0x54,0x54,0x78},
/* 0x62 'b' */ {0x7F,0x48,0x44,0x44,0x38},
/* 0x63 'c' */ {0x38,0x44,0x44,0x44,0x20},
/* 0x64 'd' */ {0x38,0x44,0x44,0x48,0x7F},
/* 0x65 'e' */ {0x38,0x54,0x54,0x54,0x18},
/* 0x66 'f' */ {0x08,0x7E,0x09,0x01,0x02},
/* 0x67 'g' */ {0x0C,0x52,0x52,0x52,0x3E},
/* 0x68 'h' */ {0x7F,0x08,0x04,0x04,0x78},
/* 0x69 'i' */ {0x00,0x44,0x7D,0x40,0x00},
/* 0x6A 'j' */ {0x20,0x40,0x44,0x3D,0x00},
/* 0x6B 'k' */ {0x7F,0x10,0x28,0x44,0x00},
/* 0x6C 'l' */ {0x00,0x41,0x7F,0x40,0x00},
/* 0x6D 'm' */ {0x7C,0x04,0x18,0x04,0x78},
/* 0x6E 'n' */ {0x7C,0x08,0x04,0x04,0x78},
/* 0x6F 'o' */ {0x38,0x44,0x44,0x44,0x38},
/* 0x70 'p' */ {0x7C,0x14,0x14,0x14,0x08},
/* 0x71 'q' */ {0x08,0x14,0x14,0x14,0x7C},
/* 0x72 'r' */ {0x7C,0x08,0x04,0x04,0x08},
/* 0x73 's' */ {0x48,0x54,0x54,0x54,0x20},
/* 0x74 't' */ {0x04,0x3F,0x44,0x40,0x20},
/* 0x75 'u' */ {0x3C,0x40,0x40,0x20,0x7C},
/* 0x76 'v' */ {0x1C,0x20,0x40,0x20,0x1C},
/* 0x77 'w' */ {0x3C,0x40,0x30,0x40,0x3C},
/* 0x78 'x' */ {0x44,0x28,0x10,0x28,0x44},
/* 0x79 'y' */ {0x0C,0x50,0x50,0x50,0x3C},
/* 0x7A 'z' */ {0x44,0x64,0x54,0x4C,0x44},
/* 0x7B '{' */ {0x00,0x08,0x36,0x41,0x00},
/* 0x7C '|' */ {0x00,0x00,0x7F,0x00,0x00},
/* 0x7D '}' */ {0x00,0x41,0x36,0x08,0x00},
/* 0x7E '~' */ {0x08,0x04,0x08,0x10,0x08},
/* 0x7F     */ {0x00,0x00,0x00,0x00,0x00},
};

/* ========== Low-level helpers ========== */
static int ssd1306_send(struct ssd1306 *lcd, u8 ctrl, u8 val)
{
    u8 buf[2] = { ctrl, val };
    int ret = i2c_master_send(lcd->client, buf, 2);
    if (ret < 0)
        dev_err(&lcd->client->dev, "I2C send failed: %d\n", ret);
    return ret;
}

static void ssd1306_send_cmd(struct ssd1306 *lcd, u8 cmd)
{
    ssd1306_send(lcd, SSD1306_CTRL_CMD, cmd);
}

static void ssd1306_send_data(struct ssd1306 *lcd, u8 data)
{
    ssd1306_send(lcd, SSD1306_CTRL_DATA, data);
}

/* ========== LCD primitives ========== */
static void ssd1306_set_cursor(struct ssd1306 *lcd, u8 x, u8 y)
{
    ssd1306_send_cmd(lcd, 0xB0 + y);                /* page address */
    ssd1306_send_cmd(lcd, 0x00 + (x & 0x0F));       /* lower column */
    ssd1306_send_cmd(lcd, 0x10 + ((x >> 4) & 0x0F));/* higher column */
    lcd->x_pos = x;
    lcd->y_pos = y;
}

static void ssd1306_clear(struct ssd1306 *lcd)
{
    int page, col;
    for (page = 0; page < SSD1306_PAGES; page++) {
        ssd1306_set_cursor(lcd, 0, page);
        for (col = 0; col < SSD1306_WIDTH; col++)
            ssd1306_send_data(lcd, 0x00);
    }
    lcd->x_pos = 0;
    lcd->y_pos = 0;
    ssd1306_set_cursor(lcd, 0, 0);
}

static void ssd1306_print_char(struct ssd1306 *lcd, char c)
{
    int i;
    if (c < 0x20 || c > 0x7F) c = '?';

    /* space before */
    for (i = 0; i < 5; i++)
        ssd1306_send_data(lcd, font5x7[c - 0x20][i]);
    /* space after */
    ssd1306_send_data(lcd, 0x00);

    lcd->x_pos += 6;
    if (lcd->x_pos >= SSD1306_WIDTH - 6) {
        lcd->x_pos = 0;
        lcd->y_pos++;
        if (lcd->y_pos >= SSD1306_PAGES)
            lcd->y_pos = 0;
        ssd1306_set_cursor(lcd, lcd->x_pos, lcd->y_pos);
    }
}

static void ssd1306_print_string(struct ssd1306 *lcd, const char *s)
{

    while (*s) {
        if (*s == '\n') {
            lcd->x_pos = 0;
            lcd->y_pos++;
            if (lcd->y_pos >= SSD1306_PAGES)
                lcd->y_pos = 0;
            ssd1306_set_cursor(lcd, lcd->x_pos, lcd->y_pos);
        } else if (*s == '\r') {
            /* ignore */
        } else {
            ssd1306_print_char(lcd, *s);
        }
        s++;
    }
}

/* ========== Init sequence ========== */
static void ssd1306_hw_init(struct ssd1306 *lcd)
{
    if (lcd->reset) {
        gpiod_set_value_cansleep(lcd->reset, 0);
        msleep(10);
        gpiod_set_value_cansleep(lcd->reset, 1);
        msleep(10);
    }

    /* Basic init sequence (typical SSD1306) */
    ssd1306_send_cmd(lcd, 0xAE); // Display off
    ssd1306_send_cmd(lcd, 0x20); ssd1306_send_cmd(lcd, 0x02); // Page mode
    // ssd1306_send_cmd(lcd, 0x00); // Horizontal addressing mode
    ssd1306_send_cmd(lcd, 0xB0); // Page Start Address
    ssd1306_send_cmd(lcd, 0xC8); // COM scan direction
    ssd1306_send_cmd(lcd, 0x00); // low col = 0
    ssd1306_send_cmd(lcd, 0x10); // hi col = 0
    ssd1306_send_cmd(lcd, 0x40); // start line = 0
    ssd1306_send_cmd(lcd, 0x81); ssd1306_send_cmd(lcd, 0x7F); // contrast
    ssd1306_send_cmd(lcd, 0xA1); // segment remap
    ssd1306_send_cmd(lcd, 0xA6); // normal display
    ssd1306_send_cmd(lcd, 0xA8); ssd1306_send_cmd(lcd, 0x3F); // multiplex
    ssd1306_send_cmd(lcd, 0xA4); // display RAM
    ssd1306_send_cmd(lcd, 0xD3); ssd1306_send_cmd(lcd, 0x00); // display offset
    ssd1306_send_cmd(lcd, 0xD5); ssd1306_send_cmd(lcd, 0x80); // clock
    ssd1306_send_cmd(lcd, 0xD9); ssd1306_send_cmd(lcd, 0xF1); // pre-charge
    ssd1306_send_cmd(lcd, 0xDA); ssd1306_send_cmd(lcd, 0x12); // COM pins
    ssd1306_send_cmd(lcd, 0xDB); ssd1306_send_cmd(lcd, 0x40); // vcomh
    ssd1306_send_cmd(lcd, 0x8D); ssd1306_send_cmd(lcd, 0x14); // enable charge pump
    ssd1306_send_cmd(lcd, 0xAF); // display ON

    ssd1306_clear(lcd);
}

/* ========== Char device ========== */
static int ssd1306_open(struct inode *inode, struct file *file)
{
    file->private_data = g_ssd1306;
    return 0;
}

static int ssd1306_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t ssd1306_write(struct file *file, const char __user *buf,
                             size_t len, loff_t *ppos)
{
    struct ssd1306 *lcd = file->private_data;
    int ret;

    if (len > MAX_LENGTH - 1) len = MAX_LENGTH - 1;

    memset(lcd->message, 0, MAX_LENGTH);
    ret = copy_from_user(lcd->message, buf, len);
    if (ret) return -EFAULT;

    if (*ppos == 0) {        
        ssd1306_clear(lcd);
        lcd->x_pos = 0;
        lcd->y_pos = 0;
        ssd1306_set_cursor(lcd, 0, 0);
    }

    ssd1306_print_string(lcd, lcd->message);

    *ppos += len;             
    return len;
}


static ssize_t ssd1306_read(struct file *file, char __user *buf,
                            size_t len, loff_t *ppos)
{
    struct ssd1306 *lcd = file->private_data;
    ssize_t to_copy = min(len, (size_t)(MAX_LENGTH - *ppos));

    if (to_copy <= 0)
        return 0;

    if (copy_to_user(buf, lcd->message + *ppos, to_copy))
        return -EFAULT;

    *ppos += to_copy;
	pr_info("[%s:%d] Read %zd bytes\n", __func__, __LINE__, to_copy);
    return to_copy;
}

static const struct file_operations ssd1306_fops = {
    .owner   = THIS_MODULE,
    .open    = ssd1306_open,
    .release = ssd1306_release,
    .write   = ssd1306_write,
    .read    = ssd1306_read,
};

/* ========== I2C probe/remove ========== */
static int ssd1306_probe(struct i2c_client *client)
{
    struct ssd1306 *lcd;
    int ret;

    lcd = devm_kzalloc(&client->dev, sizeof(*lcd), GFP_KERNEL);
    if (!lcd)
        return -ENOMEM;

    lcd->client = client;
    i2c_set_clientdata(client, lcd);

    /* Optional reset GPIO */
    lcd->reset = devm_gpiod_get_optional(&client->dev, "reset", GPIOD_OUT_HIGH);

    /* Char device */
    ret = alloc_chrdev_region(&lcd->devt, 0, 1, DEV_NAME);
    if (ret < 0)
        return ret;

    lcd->class = class_create(CLASS_NAME);
    if (IS_ERR(lcd->class)) {
        ret = PTR_ERR(lcd->class);
        goto err_unreg;
    }

    lcd->device = device_create(lcd->class, NULL, lcd->devt, NULL, DEV_NAME);
    if (IS_ERR(lcd->device)) {
        ret = PTR_ERR(lcd->device);
        goto err_class;
    }

    cdev_init(&lcd->cdev, &ssd1306_fops);
    ret = cdev_add(&lcd->cdev, lcd->devt, 1);
    if (ret)
        goto err_dev;

    g_ssd1306 = lcd;
    ssd1306_hw_init(lcd);
    ssd1306_print_string(lcd, "Hello SSD1306\n");

    dev_info(&client->dev, "SSD1306 OLED registered at /dev/%s\n", DEV_NAME);
    return 0;

err_dev:
    device_destroy(lcd->class, lcd->devt);
err_class:
    class_destroy(lcd->class);
err_unreg:
    unregister_chrdev_region(lcd->devt, 1);
    return ret;
}

static void ssd1306_remove(struct i2c_client *client)
{
    struct ssd1306 *lcd = i2c_get_clientdata(client);
    ssd1306_print_string(lcd, "Goodbye SSD1306\n");
	msleep(1500);
    ssd1306_clear(lcd);
    cdev_del(&lcd->cdev);
    device_destroy(lcd->class, lcd->devt);
    class_destroy(lcd->class);
    unregister_chrdev_region(lcd->devt, 1);
}

/* ========== OF + I2C IDs ========== */
static const struct of_device_id ssd1306_of_match[] = {
    { .compatible = "solomon,ssd1306" },
    { }
};
MODULE_DEVICE_TABLE(of, ssd1306_of_match);

static const struct i2c_device_id ssd1306_id[] = {
    { "ssd1306", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, ssd1306_id);

static struct i2c_driver ssd1306_driver = {
    .driver = {
        .name = DEV_NAME,
        .of_match_table = ssd1306_of_match,
    },
    .probe = ssd1306_probe,
    .remove = ssd1306_remove,
    .id_table = ssd1306_id,
};
module_i2c_driver(ssd1306_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NGUYENVAN TUAN");
MODULE_DESCRIPTION("I2C Character Device Driver for SSD1306 OLED");
