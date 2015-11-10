#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/timer.h>
#include <linux/string.h>
#include <linux/kobject.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ILYA POBORCEV");

#define SUCCESS 0
#define BUF_LEN 128
#define DEVICE_NAME "hello"

static long n = 0;
static unsigned long delay = 0;
static int is_active = 0;

static int major = -1;
static char buf[BUF_LEN];
static int input_error_code = 0;
static char *input_error_message;

struct timer_list timer;

static void create_timer(void);
static void timer_func(unsigned long);

static int hello_init(void);
static void hello_exit(void);
static ssize_t hello_write(struct file *, const char *, size_t, loff_t *);
static ssize_t hello_read(struct file *, char *, size_t, loff_t *);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = hello_read,
	.write = hello_write
};

static void timer_func(unsigned long data)
{
  printk("Hello %lu\n", data);
  create_timer();
}

static void create_timer(){
	if (is_active) {
    del_timer(&timer);
    is_active = 0;
  }

  if (delay != 0) {
	  timer.expires = jiffies + delay * HZ;
	  timer.data = n++;
	  timer.function = timer_func;
	  is_active = 1;
	  add_timer(&timer);
	  }
}

static int __init hello_init() 
{
	init_timer(&timer);
	major = register_chrdev(280, DEVICE_NAME, &fops);
	if (major < 0) {
		printk(KERN_ALERT "repeat_hello register_chrdev() error: %d\n", major);
		return major;
	}
	printk(KERN_NOTICE "major number = %d\n", major);
	return SUCCESS;
}

static void __exit hello_exit() 
{
	if (is_active) {
		del_timer(&timer);
	}
	if (major > 0)
		unregister_chrdev(280, DEVICE_NAME);
}

static ssize_t hello_write(struct file *filp, const char *buffer, size_t length, loff_t *offset) 
{
  unsigned long tmp;
  if (copy_from_user(buf, buffer, length)) {
		input_error_code = -EINVAL;
		input_error_message = "copy_from_user error";
		return input_error_code;
  }
  if (kstrtoul(buf, 10, &tmp) == -EINVAL) {
    return -EINVAL;
  }
  delay = tmp;
  n = 0;
  create_timer();
  return length;	
}

static ssize_t hello_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
	ssize_t size;
	long error_length;

	if (*offset != 0) {
		return 0;
	}

	if (input_error_code < 0) {
		error_length = strlen(input_error_message);
		copy_to_user(buffer, input_error_message, error_length);
		*offset = error_length;
		return error_length;
	}

	snprintf(buf, BUF_LEN, "%lu", delay);
	size = strlen(buf);
	if (size >= length) {
		return -EINVAL;
	}
	if (copy_to_user(buffer, buf, size)) {
		return -EINVAL;
	}
	*offset = size;
	return size;
}

module_init(hello_init);
module_exit(hello_exit);
