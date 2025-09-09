#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/gpio.h>
#include <linux/ktime.h>

#define DEVICE_NAME "servo_pwm"
#define BUFFER_SIZE 32

#define GPIO_PIN 471
#define SIGNAL_PERIOD 25000000
#define FULL_LEFT 500000
#define FULL_RIGHT 2500000

static int major;
static struct cdev char_cdev;
static dev_t dev_num;

static struct hrtimer pwm_timer;
static struct hrtimer servo_timer;

static int on_time = 1500000; //servo 90 degrees.
static int off_time;
static bool is_on_interval = true;

static spinlock_t lock;

enum direction {
	LEFT,
	RIGHT,
};

static enum direction current_direction = RIGHT;

static enum hrtimer_restart pwm_task(struct hrtimer *timer) {
	
	ktime_t interval;
	unsigned long flags;

	spin_lock_irqsave(&lock, flags);

	off_time = SIGNAL_PERIOD - on_time;
	
	if (is_on_interval) {
		interval = ktime_set(0, off_time);
		gpio_set_value(GPIO_PIN, 0);
		is_on_interval = false;
	} else {
		interval = ktime_set(0, on_time);
		gpio_set_value(GPIO_PIN, 1);
		is_on_interval = true;
	}

	spin_unlock_irqrestore(&lock, flags);
	hrtimer_forward_now(timer, interval);
	return HRTIMER_RESTART;
}

static enum hrtimer_restart servo_sweep(struct hrtimer *timer) {
	ktime_t interval = 100000; //5ms
	unsigned long flags;
	spin_lock_irqsave(&lock, flags);
	if (current_direction == RIGHT) {
		on_time += 500;
	} else if (current_direction == LEFT) {
		on_time -= 500;
	}

	if (on_time >= FULL_RIGHT) {
		current_direction = LEFT;
	} else if (on_time <= FULL_LEFT) {
		current_direction = RIGHT;
	}
	spin_unlock_irqrestore(&lock, flags);
	//pr_info("%d\n", on_time);
	hrtimer_forward_now(timer, interval);
	return HRTIMER_RESTART;
}

static int _open(struct inode *inode, struct file *file){
	return 0;
}

static int _release(struct inode *inode, struct file *file){
	return 0;
}

static ssize_t _read(struct file *file, char __user *buffer, size_t len, loff_t  *offset){
	char temp_buf[32];
	int bytes_to_copy;

	if (*offset > 0)
		return 0;

	bytes_to_copy = snprintf(temp_buf, sizeof(temp_buf), "%d %d\n", on_time, off_time);

	if (len < bytes_to_copy)
		bytes_to_copy = len;

	if (copy_to_user(buffer, temp_buf, bytes_to_copy))
		return -EFAULT;

	*offset += bytes_to_copy;
	return bytes_to_copy;
}

static ssize_t _write(struct file *file, const char __user *buffer, size_t len, loff_t *offset){
	char device_buffer[2];
	
	if (len > 1) 
		return -EINVAL;

	if (copy_from_user(device_buffer, buffer, 1)){
		return -EFAULT;
	}
	if (device_buffer[0] == 'W')
		on_time += 50000;
	else if (device_buffer[0] == 'S')
		on_time -= 50000;

	if (on_time >= 2500000)
		on_time = 2500000;
	else if (on_time <= 500000)
		on_time = 500000;

	return len;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = _open,
	.release = _release,
	.read = _read,
	.write = _write,
};

static int __init shit_init(void){
	int ret;
	ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
	if (ret < 0){
		printk(KERN_ERR "Failed to create a char dev\n");
		return ret;
	}
	major = MAJOR(dev_num);	
	cdev_init(&char_cdev, &fops);
	char_cdev.owner = THIS_MODULE;

	if (gpio_request(GPIO_PIN, "pwm_pin") < 0) {
		printk(KERN_ERR "GPIO %d request faled\n", GPIO_PIN);
		unregister_chrdev_region(dev_num, 1);
		return -1;
	}

	gpio_direction_output(GPIO_PIN, 0);
	
	ret = cdev_add(&char_cdev, dev_num, 1);
	if (ret < 0) {
		unregister_chrdev_region(dev_num, 1);
		gpio_free(GPIO_PIN);
		printk(KERN_ALERT "Failed to add cdev\n");
		return ret;
	}
	
	printk(KERN_INFO "Device registered with major: %d\n", major);

	ktime_t ktime;
	ktime_t testing;
	
	spin_lock_init(&lock);

	hrtimer_init(&pwm_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	pwm_timer.function = &pwm_task;
	ktime = ktime_set(0, on_time);
	hrtimer_start(&pwm_timer, ktime, HRTIMER_MODE_REL);

	hrtimer_init(&servo_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	servo_timer.function = &servo_sweep;
	testing = ktime_set(0, 250 * 1000000);
	hrtimer_start(&servo_timer, testing, HRTIMER_MODE_REL);

	return 0;
}

static void __exit shit_exit(void){
	hrtimer_cancel(&pwm_timer);
	hrtimer_cancel(&servo_timer);
	gpio_set_value(GPIO_PIN, 0);
	gpio_free(GPIO_PIN);
	cdev_del(&char_cdev);
	unregister_chrdev_region(dev_num, 1);
	printk(KERN_INFO "Device unregistered\n");
}

module_init(shit_init);
module_exit(shit_exit);

MODULE_LICENSE("GPL");

