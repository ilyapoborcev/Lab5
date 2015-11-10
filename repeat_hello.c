#include <linux/module.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/string.h>
#include <linux/timer.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ILYA POBORCEV");

static unsigned long delay = 0;
static unsigned long n = 0;
static int is_active = 0;

struct timer_list timer;

static void create_timer(void);
static void timer_func(unsigned long);

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

static ssize_t show(struct kobject *kobj,
			struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%lu\n", delay);
}

static ssize_t store(struct kobject *kobj,
			struct kobj_attribute *attr, const char *buf, size_t count)
{
  unsigned long tmp;

  if (kstrtoul(buf, 10, &tmp) == -EINVAL) {
    return -EINVAL;
  }
  delay = tmp;
  n = 0;
  create_timer();
  return count;
}

static struct kobj_attribute sc_attrb =
	__ATTR(delay, 0665, show, store);

static struct kobject *kobj;

static int __init repeat_hello_init(void)
{
	int ret;
	init_timer(&timer);
	kobj = kobject_create_and_add("repeat_hello", NULL);
	if (!kobj)
		return - ENOMEM;

 	ret = sysfs_create_file(kobj, &sc_attrb.attr);
	if (ret)
		goto attr_file_failed;
	return 0;

attr_file_failed:
	kobject_put(kobj);
	return ret;
}

static void __exit repeat_hello_exit(void)
{
  if (is_active) {
    del_timer(&timer);
  }
	sysfs_remove_file(kobj, &sc_attrb.attr);
	kobject_put(kobj);
}

module_init(repeat_hello_init);
module_exit(repeat_hello_exit);


