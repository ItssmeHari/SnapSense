//snapsense.c — SnapSense Linux Character Driver

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>

#define DRIVER_NAME     "snapsense"
#define CLASS_NAME      "snapsense_class"

#define PKT_SOP1        0xAAU
#define PKT_SOP2        0x55U
#define PKT_DATA_LEN    2U
#define PKT_TOTAL_LEN   6U
#define PKT_HELLO_LEN   4U

struct snapsense_dev {
    struct cdev     cdev;
    struct device  *device;
    struct mutex    lock;
    int32_t         temp_x10;
    int             has_data;
};

static struct snapsense_dev *snap_dev;
static struct class         *snap_class;
static dev_t                 snap_devno;

static int32_t parse_packet(const uint8_t *buf)
{
    uint8_t checksum;

    if (buf[0] != PKT_SOP1 || buf[1] != PKT_SOP2) {
        pr_warn("snapsense: bad SOP\n");
        return INT32_MIN;
    }

    if (buf[2] != PKT_DATA_LEN) {
        pr_warn("snapsense: bad length\n");
        return INT32_MIN;
    }

    checksum = buf[2] ^ buf[3] ^ buf[4];
    if (checksum != buf[5]) {
        pr_warn("snapsense: checksum error\n");
        return INT32_MIN;
    }

    return (int32_t)(int16_t)(((uint16_t)buf[3] << 8) | (uint16_t)buf[4]);
}

static int snapsense_open(struct inode *inode, struct file *filp)
{
    filp->private_data = snap_dev;
    return 0;
}

static ssize_t snapsense_read(struct file *filp, char __user *ubuf,
                               size_t count, loff_t *ppos)
{
    struct snapsense_dev *dev = filp->private_data;
    char    kbuf[32];
    int     len;
    int32_t t, int_part, dec_part;

    if (*ppos > 0)
        return 0;

    mutex_lock(&dev->lock);
    if (!dev->has_data) {
        len = snprintf(kbuf, sizeof(kbuf), "NO_DATA\n");
    } else {
        t        = dev->temp_x10;
        int_part = t / 10;
        dec_part = t % 10;
        if (dec_part < 0)
            dec_part = -dec_part;
        len = snprintf(kbuf, sizeof(kbuf), "%d.%d C\n",
                       (int)int_part, (int)dec_part);
    }
    mutex_unlock(&dev->lock);

    if (count < (size_t)len)
        return -EINVAL;

    if (copy_to_user(ubuf, kbuf, len))
        return -EFAULT;

    *ppos += len;
    return (ssize_t)len;
}

static ssize_t snapsense_write(struct file *filp, const char __user *ubuf,
                                size_t count, loff_t *ppos)
{
    struct snapsense_dev *dev = filp->private_data;
    uint8_t  kbuf[PKT_TOTAL_LEN];
    int32_t  temp;

    if (count != PKT_TOTAL_LEN && count != PKT_HELLO_LEN) {
        pr_err("snapsense: unexpected write size: %zu\n", count);
        return -EINVAL;
    }

    if (copy_from_user(kbuf, ubuf, count))
        return -EFAULT;

    if (count == PKT_HELLO_LEN &&
        kbuf[0] == 0xAAU &&
        kbuf[1] == 0x55U &&
        kbuf[2] == 0x00U &&
        kbuf[3] == 0xFFU) {
        pr_info("snapsense: firmware hello received\n");
        return (ssize_t)count;
    }

    temp = parse_packet(kbuf);
    if (temp == INT32_MIN)
        return -EBADMSG;

    mutex_lock(&dev->lock);
    dev->temp_x10 = temp;
    dev->has_data  = 1;
    mutex_unlock(&dev->lock);

    pr_info("snapsense: temp = %d.%d C\n",
            (int)(temp / 10),
            (int)(temp % 10 < 0 ? -(temp % 10) : temp % 10));

    return (ssize_t)count;
}

static int snapsense_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static const struct file_operations snapsense_fops = {
    .owner   = THIS_MODULE,
    .open    = snapsense_open,
    .read    = snapsense_read,
    .write   = snapsense_write,
    .release = snapsense_release,
};

static int __init snapsense_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&snap_devno, 0, 1, DRIVER_NAME);
    if (ret < 0) {
        pr_err("snapsense: alloc_chrdev_region failed: %d\n", ret);
        return ret;
    }

    snap_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(snap_class)) {
        ret = PTR_ERR(snap_class);
        goto err_unregister;
    }

    snap_dev = kzalloc(sizeof(*snap_dev), GFP_KERNEL);
    if (!snap_dev) {
        ret = -ENOMEM;
        goto err_class;
    }

    mutex_init(&snap_dev->lock);

    cdev_init(&snap_dev->cdev, &snapsense_fops);
    snap_dev->cdev.owner = THIS_MODULE;
    ret = cdev_add(&snap_dev->cdev, snap_devno, 1);
    if (ret < 0)
        goto err_kfree;

    snap_dev->device = device_create(snap_class, NULL, snap_devno,
                                     NULL, DRIVER_NAME);
    if (IS_ERR(snap_dev->device)) {
        ret = PTR_ERR(snap_dev->device);
        goto err_cdev;
    }

    pr_info("snapsense: ready at /dev/%s (major=%d)\n",
            DRIVER_NAME, MAJOR(snap_devno));
    return 0;

err_cdev:
    cdev_del(&snap_dev->cdev);
err_kfree:
    kfree(snap_dev);
err_class:
    class_destroy(snap_class);
err_unregister:
    unregister_chrdev_region(snap_devno, 1);
    return ret;
}

static void __exit snapsense_exit(void)
{
    device_destroy(snap_class, snap_devno);
    cdev_del(&snap_dev->cdev);
    kfree(snap_dev);
    class_destroy(snap_class);
    unregister_chrdev_region(snap_devno, 1);
    pr_info("snapsense: driver removed\n");
}

module_init(snapsense_init);
module_exit(snapsense_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hari Prasath S");
MODULE_DESCRIPTION("SnapSense: STM32 temperature sensor to Linux character device");
MODULE_VERSION("1.1");