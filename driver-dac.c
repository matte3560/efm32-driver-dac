/*
 * This is a dac Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/sched.h>

#include <asm/io.h>
#include <asm/siginfo.h>
#include <asm/errno.h>

//#include "offsets.h"

// Device name
#define DEVICE_NAME "dac"

// Device number used for dac
static dev_t dac_dev;

// cdev struct
static struct cdev dac_cdev;

// Class struct
static struct class *dac_cl;

// Platfom information
static struct resource *dac_res;


// User program opens the driver
static int dac_open(struct inode *inode, struct file *filp) {
	printk("Opened by PID %i\n", current->pid);
	return 0;
}

// User program closes the driver
static int dac_release(struct inode *inode, struct file *filp) {
	printk("Closed by PID %i\n", current->pid);
	return 0;
}

// User program reads from the driver
static ssize_t dac_read(struct file *filp, char __user *buff, size_t count, loff_t *offp) {
	printk("Read\n");

	return 0;
}

// User program writes to the driver
static ssize_t dac_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp) {
	printk("Write\n");

	return count;
}

// File operations struct for cdev
static struct file_operations dac_fops = {
	.owner = THIS_MODULE,
	.read = dac_read,
	.write = dac_write,
	.open = dac_open,
	.release = dac_release
};


// Interrupt handler
static irqreturn_t dac_irq_handler(int irq, void *dev_id) {
	// TODO: Implement interrupt handling

	return IRQ_HANDLED;
}


static int dac_probe(struct platform_device *p_dev) {
	int result;

	printk("Device found for dac driver\n");

	// Get platform info
	dac_res = platform_get_resource(p_dev, IORESOURCE_MEM, 0);
	printk("Name: %s\n", dac_res->name);
	printk("Base addr: %x\n", dac_res->start);

	// TODO: Configure dac

	// Allocate device number
	result = alloc_chrdev_region(&dac_dev, 1, 1, DEVICE_NAME);
	if (result < 0) return -1; // Failed to allocate device number
	printk("Device number allocated: major %i, minor %i\n", MAJOR(dac_dev), MINOR(dac_dev));

	// Initialize cdev
	cdev_init(&dac_cdev, &dac_fops);
	result = cdev_add(&dac_cdev, dac_dev, 1);
	if (result < 0) return -1; // Failed to add cdev

	// Make visible in userspace
	dac_cl = class_create(THIS_MODULE, DEVICE_NAME);
	device_create(dac_cl, NULL, dac_dev, NULL, DEVICE_NAME);
	
	return 0;
}

static int dac_remove(struct platform_device *p_dev) {
	// TODO: Disable DAC

	// Delete class
	device_destroy(dac_cl, dac_dev);
	class_destroy(dac_cl);

	// Delete cdev
	cdev_del(&dac_cdev);

	// Free device number
	unregister_chrdev_region(dac_dev, 1);

	return 0;
}

static const struct of_device_id dac_of_match[] = {
	{ .compatible = "tdt4258", },
	{ },
};
MODULE_DEVICE_TABLE(of, dac_of_match);

static struct platform_driver dac_driver = {
	.probe = dac_probe,
	.remove = dac_remove,
	.driver = {
		.name = DEVICE_NAME,
		.owner = THIS_MODULE,
		.of_match_table = dac_of_match,
	},
};

/*
 * dac_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 *
 * Returns 0 if successfull, otherwise -1
 */

static int __init dac_init(void)
{
	printk("Hello World, here is your module speaking\n");

	// Register platform driver
	platform_driver_register(&dac_driver);

	return 0;
}

/*
 * dac_cleanup - function to cleanup this module from kernel space
 *
 * This is the second of two exported functions to handle cleanup this
 * code from a running kernel
 */

static void __exit dac_cleanup(void)
{
	 printk("Short life for a small module...\n");

	 // Unregister platform driver
	 platform_driver_unregister(&dac_driver);
}

module_init(dac_init);
module_exit(dac_cleanup);

MODULE_DESCRIPTION("Module for accessing dac buttons.");
MODULE_LICENSE("GPL");

