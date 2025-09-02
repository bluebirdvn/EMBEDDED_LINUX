

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h> 
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/spi/spi.h>

#define DEVNUM_NAME        "nokia5110_devnum"
#define CDEV_NAME_DEVICE   "nokia5110"
#define CDEV_NAME_CLASS    "nokia5110_class"

#define MAX_LENGTH         256

/* LCD dimensions */
#define NOKIA5110_WIDTH    84
#define NOKIA5110_HEIGHT   48
#define NOKIA5110_NUM_BANK 6 /* 48/8 */

/* LCD modes for DC pin */
#define NOKIA5110_MODE_CMD   0
#define NOKIA5110_MODE_DATA  1

/* LCD commands (PCD8544) */
#define LCD_CMD_EXTENDED     0x21  /* Extended instruction set */
#define LCD_CMD_CONTRAST     0xB9  /* Vop (contrast), tuned default */
#define LCD_CMD_TEMP_COEF    0x04  /* Temperature coefficient */
#define LCD_CMD_BIAS         0x14  /* Bias system 1:48 */
#define LCD_CMD_NORMAL       0x0C  /* Display control: normal mode */
#define LCD_CMD_INVERSE      0x0D  /* Display control: inverse */
#define LCD_CMD_BASIC        0x20  /* Basic instruction set */
#define LCD_CMD_DISPLAY_ON   0x0C  /* Display ON in basic set */
#define LCD_CMD_SET_X        0x80  /* X address (0..83) */
#define LCD_CMD_SET_Y        0x40  /* Y address (0..5) */

/* Device structure */
typedef struct {
	struct spi_device *spi_dev;

	dev_t  devt;
	struct class   *class;
	struct device  *device;
	struct cdev     cdev;

	/* GPIO descriptors - better than raw GPIO numbers */
	struct gpio_desc *rst_gpio;
	struct gpio_desc *dc_gpio;

	/* Current cursor position */
	u8 x_pos;
	u8 y_pos;

	char message[MAX_LENGTH];
} nokia5110_t;

static nokia5110_t *module_nokia5110;

/* Prototypes */
static void nokia5110_init(void);
static void nokia5110_clear_screen(void);
static int  nokia5110_send_byte(bool is_data, u8 data);
static void nokia5110_print_char(char c);
static void nokia5110_print_string(const char *str);
static void nokia5110_set_position(u8 x, u8 y);
static void nokia5110_cleanup(void);
static int  nokia5110_create_device_file(nokia5110_t *module);
static void nokia5110_destroy_device_file(nokia5110_t *module);

/* File ops */
static int     nokia5110_open(struct inode *inodep, struct file *filep);
static int     nokia5110_release(struct inode *inodep, struct file *filep);
static ssize_t nokia5110_write(struct file *filep, const char __user *buf,
                               size_t len, loff_t *offset);
static ssize_t nokia5110_read(struct file *filep, char __user *buf,
                              size_t len, loff_t *off);

static const struct file_operations fops = {
	.owner   = THIS_MODULE,
	.open    = nokia5110_open,
	.release = nokia5110_release,
	.write   = nokia5110_write,
	.read    = nokia5110_read,
};

/* ---------- 5x7 ASCII font (32..127) ---------- */
static const u8 ASCII[][5] = {
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

/* ----------------- Helper functions ----------------- */

static void nokia5110_set_position(u8 x, u8 y)
{
	int ret;

	if (x >= NOKIA5110_WIDTH || y >= NOKIA5110_NUM_BANK)
		return;

	ret = nokia5110_send_byte(NOKIA5110_MODE_CMD, LCD_CMD_SET_X | x);

	if (ret < 0)
		pr_err("[%s:%d] Failed to set X\n", __func__, __LINE__);

	ret = nokia5110_send_byte(NOKIA5110_MODE_CMD, LCD_CMD_SET_Y | y);
	if (ret < 0)
		pr_err("[%s:%d] Failed to set Y\n", __func__, __LINE__);
}

static int nokia5110_send_byte(bool is_data, u8 data)
{
	int ret;
	struct spi_transfer t;
	struct spi_message  m;

	if (!module_nokia5110 || !module_nokia5110->dc_gpio) {
		pr_err("[%s:%d] GPIO not initialized\n", __func__, __LINE__);
		return -ENODEV;
	}

	/* DC pin - use gpiod_set_value for proper polarity handling */
	gpiod_set_value(module_nokia5110->dc_gpio, is_data ? 1 : 0);

	memset(&t, 0, sizeof(t));
	spi_message_init(&m);

	t.tx_buf   = &data;
	t.len      = 1;
	t.speed_hz = 100000; /* Match DTS max frequency */

	spi_message_add_tail(&t, &m);

	ret = spi_sync(module_nokia5110->spi_dev, &m);
	if (ret < 0)
		pr_err("[%s:%d] SPI transfer failed: %d\n", __func__, __LINE__, ret);

	udelay(1);
	return ret;
}

static void nokia5110_clear_screen(void)
{
    int idx, ret;

    nokia5110_set_position(0, 0);
    for (idx = 0; idx < NOKIA5110_WIDTH * (NOKIA5110_HEIGHT/8); idx++) {
        ret = nokia5110_send_byte(NOKIA5110_MODE_DATA, 0x00);
        if (ret < 0) return;
    }

    module_nokia5110->x_pos = 0;
    module_nokia5110->y_pos = 0;
    nokia5110_set_position(0, 0);
}


static void nokia5110_print_char(char c)
{
    int i, ret;

    // ret = nokia5110_send_byte(NOKIA5110_MODE_DATA, 0x00);

    if (c < 0x20 || c > 0x7F) c = '?';

    for (i = 0; i < 5; i++) {
        ret = nokia5110_send_byte(NOKIA5110_MODE_DATA, ASCII[c - 0x20][i]);
        if (ret < 0) return;
    }

    ret = nokia5110_send_byte(NOKIA5110_MODE_DATA, 0x00);
    if (ret < 0) return;

    // Cursor advance: 5 + 1 = 6
    module_nokia5110->x_pos += 6;
    if (module_nokia5110->x_pos >= NOKIA5110_WIDTH) {
        module_nokia5110->x_pos = 0;
        module_nokia5110->y_pos++;
        if (module_nokia5110->y_pos >= NOKIA5110_NUM_BANK)
            module_nokia5110->y_pos = 0;
        nokia5110_set_position(module_nokia5110->x_pos, module_nokia5110->y_pos);
    }
}


static void nokia5110_print_string(const char *data)
{

	nokia5110_set_position(module_nokia5110->x_pos, module_nokia5110->y_pos);
	while (*data) {
		if (*data == '\n') {
			module_nokia5110->x_pos = 0;
			module_nokia5110->y_pos++;
			if (module_nokia5110->y_pos >= NOKIA5110_NUM_BANK)
				module_nokia5110->y_pos = 0;
			nokia5110_set_position(module_nokia5110->x_pos, module_nokia5110->y_pos);
		} else if (*data == '\r') {
			/* ignore */
		} else {
			nokia5110_print_char(*data);
		}
		data++;
	}
}

/* ----------------- LCD init & cleanup ----------------- */

static void nokia5110_init(void)
{
	int ret;

	pr_info("[%s:%d] Nokia5110 display initialization\n", __func__, __LINE__);

	if (!module_nokia5110->rst_gpio || !module_nokia5110->dc_gpio) {
		pr_err("[%s:%d] GPIO descriptors not available\n", __func__, __LINE__);
		return;
	}

	pr_info("[%s:%d] Starting LCD reset sequence\n", __func__, __LINE__);
	gpiod_set_value(module_nokia5110->rst_gpio, 1);  /* Assert reset (active low) */
	mdelay(10);
	gpiod_set_value(module_nokia5110->rst_gpio, 0);  /* Release reset */
	mdelay(10);
	pr_info("[%s:%d] LCD reset completed\n", __func__, __LINE__);

	/* Initialization sequence với error checking */
	pr_info("[%s:%d] Starting LCD command sequence\n", __func__, __LINE__);
	
	ret = nokia5110_send_byte(NOKIA5110_MODE_CMD, LCD_CMD_EXTENDED);
	if (ret < 0) pr_err("[%s:%d] Extended cmd failed: %d\n", __func__, __LINE__, ret);
	
	ret = nokia5110_send_byte(NOKIA5110_MODE_CMD, LCD_CMD_CONTRAST);
	if (ret < 0) pr_err("[%s:%d] Contrast cmd failed: %d\n", __func__, __LINE__, ret);
	
	ret = nokia5110_send_byte(NOKIA5110_MODE_CMD, LCD_CMD_TEMP_COEF);
	if (ret < 0) pr_err("[%s:%d] Temp coef cmd failed: %d\n", __func__, __LINE__, ret);
	
	ret = nokia5110_send_byte(NOKIA5110_MODE_CMD, LCD_CMD_BIAS);
	if (ret < 0) pr_err("[%s:%d] Bias cmd failed: %d\n", __func__, __LINE__, ret);
	
	ret = nokia5110_send_byte(NOKIA5110_MODE_CMD, LCD_CMD_BASIC);
	if (ret < 0) pr_err("[%s:%d] Basic cmd failed: %d\n", __func__, __LINE__, ret);
	
	ret = nokia5110_send_byte(NOKIA5110_MODE_CMD, LCD_CMD_NORMAL);
	if (ret < 0) pr_err("[%s:%d] Normal cmd failed: %d\n", __func__, __LINE__, ret);
	
	ret = nokia5110_send_byte(NOKIA5110_MODE_CMD, LCD_CMD_DISPLAY_ON);
	if (ret < 0) pr_err("[%s:%d] Display on cmd failed: %d\n", __func__, __LINE__, ret);

	pr_info("[%s:%d] LCD command sequence completed\n", __func__, __LINE__);

	/* Clear screen and reset positions */
	nokia5110_clear_screen();
	pr_info("[%s:%d] Screen cleared\n", __func__, __LINE__);

	module_nokia5110->x_pos = 0;
	module_nokia5110->y_pos = 0;
	
	pr_info("[%s:%d] Nokia5110 initialization completed successfully\n", __func__, __LINE__);
}

static void nokia5110_cleanup(void)
{
	pr_info("[%s:%d] Cleaning up Nokia5110 resources\n", __func__, __LINE__);

	if (module_nokia5110 && module_nokia5110->spi_dev)
		nokia5110_clear_screen();

}

/* ----------------- Char device helpers ----------------- */

static int nokia5110_create_device_file(nokia5110_t *module)
{
	int ret = 0;

	pr_info("[%s:%d] Creating device file\n", __func__, __LINE__);

	/* Allocate dev numbers */
	ret = alloc_chrdev_region(&module->devt, 0, 1, DEVNUM_NAME);
	if (ret < 0) {
		pr_err("[%s:%d] Cannot register major number: %d\n", __func__, __LINE__, ret);
		goto alloc_dev_failed;
	}
	pr_info("[%s:%d] Registered major=%d minor=%d\n",
		__func__, __LINE__, MAJOR(module->devt), MINOR(module->devt));

	/* Create class */
	module->class = class_create(CDEV_NAME_CLASS);
	if (IS_ERR(module->class)) {
		ret = PTR_ERR(module->class);
		pr_err("[%s:%d] Cannot create device class: %d\n", __func__, __LINE__, ret);
		goto create_class_failed;
	}

	/* Create /dev node */
	module->device = device_create(module->class, NULL, module->devt, NULL, CDEV_NAME_DEVICE);
	if (IS_ERR(module->device)) {
		ret = PTR_ERR(module->device);
		pr_err("[%s:%d] Cannot create device file: %d\n", __func__, __LINE__, ret);
		goto create_device_failed;
	}

	/* Init & add cdev */
	cdev_init(&module->cdev, &fops);
	module->cdev.owner = THIS_MODULE;
	module->cdev.dev   = module->devt;

	ret = cdev_add(&module->cdev, module->devt, 1);
	if (ret) {
		pr_err("[%s:%d] Cannot add cdev: %d\n", __func__, __LINE__, ret);
		goto cdev_add_failed;
	}

	pr_info("[%s:%d] Device file created successfully\n", __func__, __LINE__);
	return 0;

cdev_add_failed:
	device_destroy(module->class, module->devt);
create_device_failed:
	class_destroy(module->class);
create_class_failed:
	unregister_chrdev_region(module->devt, 1);
alloc_dev_failed:
	return ret;
}

static void nokia5110_destroy_device_file(nokia5110_t *module)
{
	if (!module) return;

	/* Remove cdev and nodes in reverse order */
	cdev_del(&module->cdev);
	device_destroy(module->class, module->devt);
	class_destroy(module->class);
	unregister_chrdev_region(module->devt, 1);
}

/* ----------------- File operations ----------------- */

static int nokia5110_open(struct inode *inodep, struct file *filep)
{
	pr_info("[%s:%d] Device opened\n", __func__, __LINE__);
	filep->private_data = module_nokia5110;
	return 0;
}

static int nokia5110_release(struct inode *inodep, struct file *filep)
{
	pr_info("[%s:%d] Device closed\n", __func__, __LINE__);
	filep->private_data = NULL;
	return 0;
}

static ssize_t nokia5110_write(struct file *filep, const char __user *buf,
                               size_t len, loff_t *offset)
{
    int ret;
    nokia5110_t *module = filep->private_data;

    if (len > MAX_LENGTH - 1) len = MAX_LENGTH - 1;

    memset(module->message, 0, MAX_LENGTH);
    ret = copy_from_user(module->message, buf, len);
    if (ret) return -EFAULT;

    if (*offset == 0) {
        nokia5110_clear_screen();
        module->x_pos = 0;
        module->y_pos = 0;
        nokia5110_set_position(0, 0);
    }

    nokia5110_print_string(module->message);

    *offset += len;          
    return len;
}

static ssize_t nokia5110_read(struct file *filep, char __user *buf,
                              size_t len, loff_t *off)
{
	ssize_t bytes_to_read = min(len, (size_t)(MAX_LENGTH - *off));
	nokia5110_t *module = filep->private_data;

	pr_info("[%s:%d] Reading from device\n", __func__, __LINE__);

	if (bytes_to_read <= 0) {
		pr_info("[%s:%d] End of file\n", __func__, __LINE__);
		return 0;
	}

	if (copy_to_user(buf, module->message + *off, bytes_to_read)) {
		pr_err("[%s:%d] Copy to user failed\n", __func__, __LINE__);
		return -EFAULT;
	}

	*off += bytes_to_read;
	pr_info("[%s:%d] Read %zd bytes\n", __func__, __LINE__, bytes_to_read);
	return bytes_to_read;
}

/* ----------------- SPI probe/remove ----------------- */

static int nokia5110_spi_probe(struct spi_device *spi)
{
	nokia5110_t *module;
	int ret;

	pr_info("[%s:%d] Probing Nokia5110 SPI device\n", __func__, __LINE__);

	module = kzalloc(sizeof(*module), GFP_KERNEL);
	if (!module) {
		pr_err("[%s:%d] Failed to allocate memory\n", __func__, __LINE__);
		return -ENOMEM;
	}

	/* Get GPIO descriptors from device tree - handles polarity automatically */
	module->rst_gpio = devm_gpiod_get(&spi->dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(module->rst_gpio)) {
		ret = PTR_ERR(module->rst_gpio);
		pr_err("[%s:%d] Failed to get reset GPIO: %d\n", __func__, __LINE__, ret);
		goto err_free_module;
	}

	module->dc_gpio = devm_gpiod_get(&spi->dev, "dc", GPIOD_OUT_LOW);
	if (IS_ERR(module->dc_gpio)) {
		ret = PTR_ERR(module->dc_gpio);
		pr_err("[%s:%d] Failed to get DC GPIO: %d\n", __func__, __LINE__, ret);
		goto err_free_module;
	}

	pr_info("[%s:%d] GPIO descriptors obtained successfully\n", __func__, __LINE__);

	/* SPI config (keep controller defaults except enforce sane ones) */
	spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;
	if (!spi->max_speed_hz)
		spi->max_speed_hz = 100000;  /* Match DTS frequency */

	ret = spi_setup(spi);
	if (ret < 0) {
		pr_err("[%s:%d] Failed to setup SPI: %d\n", __func__, __LINE__, ret);
		goto err_free_module;
	}

	/* Create /dev file */
	ret = nokia5110_create_device_file(module);
	if (ret) {
		pr_err("[%s:%d] Failed to create device file: %d\n", __func__, __LINE__, ret);
		goto err_free_module;
	}

	/* Bind state */
	module->spi_dev = spi;
	module_nokia5110 = module;

	/* Initialize LCD and say hello */
	nokia5110_init();
	nokia5110_clear_screen();
	nokia5110_print_string("Nguyen van tuan\nHello World\nReady!");

	pr_info("[%s:%d] Nokia5110 device created successfully\n", __func__, __LINE__);

	spi_set_drvdata(spi, module);
	return 0;

err_free_module:
	kfree(module);
	return ret;
}

static void nokia5110_spi_remove(struct spi_device *spi)
{
	nokia5110_t *module = spi_get_drvdata(spi);
	nokia5110_clear_screen();
	nokia5110_print_string("Tuan\nGoodbye World\nShutDown!");
	msleep(1500);
	pr_info("[%s:%d] Removing Nokia5110 SPI device\n", __func__, __LINE__);

	nokia5110_cleanup();
	nokia5110_destroy_device_file(module);
	kfree(module);
	module_nokia5110 = NULL;
}

/* ----------------- DT match & module boilerplate ----------------- */

static const struct of_device_id nokia5110_of_match_id[] = {
	{ .compatible = "philips,pcd8544" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, nokia5110_of_match_id);

static const struct spi_device_id nokia5110_spi_id[] = {
	{ "pcd8544", 0 },
	{ "nokia5110", 0 },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(spi, nokia5110_spi_id);

static struct spi_driver nokia5110_spi_driver = {
	.driver = {
		.name           = "nokia5110",
		.owner          = THIS_MODULE,
		.of_match_table = nokia5110_of_match_id,
	},
	.id_table = nokia5110_spi_id, 
	.probe  = nokia5110_spi_probe,
	.remove = nokia5110_spi_remove,
};
module_spi_driver(nokia5110_spi_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("NGUYEN VAN TUAN");
MODULE_DESCRIPTION("SPI Device Driver for Nokia 5110 (PCD8544)");
