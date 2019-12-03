#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

/*ʱ��Ͷ�ʱ����ص��ļ��������ʹ��sleep��mdelay�ȣ�����ȥ��delay�����������*ʹ�ö�ʱ��������ȥ����ʱ����غ���
*/
#include <linux/delay.h>
#include<linux/timer.h>
#include<linux/jiffies.h>
/*����ע���ͷ�ļ������������Ľṹ���ע���ж�صĺ���*/
#include <linux/platform_device.h>
/*ע�������豸ͷ�ļ�*/
#include <linux/miscdevice.h>
#include <linux/device.h>
/*ע���豸�ڵ���ļ��ṹ��*/
#include <linux/fs.h>
/*Linux������GPIO��ͷ�ļ�*/
#include <linux/gpio.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <mach/gpio.h>

#define LED_MAGIC 'k'
#define IOCTL_LED_ON _IOW (LED_MAGIC, 1, int)
#define IOCTL_LED_OFF _IOW (LED_MAGIC, 2, int)
#define IOCTL_LED_RUN _IOW (LED_MAGIC, 3, int)
#define IOCTL_LED_SHINE _IOW (LED_MAGIC, 4, int)
#define IOCTL_LED_ALLON _IOW (LED_MAGIC, 5, int)
#define IOCTL_LED_ALLOFF _IOW (LED_MAGIC, 6, int)
/*
�ɺ궨����ɵ�һ��32λ����
bit31~bit30 2λΪ �������д�� ���������������Ƕ�ȡ�����д�����
bit29~bit15 14λΪ "���ݴ�С" ������ʾ ioctl() �е� arg �������͵��ڴ��С��
bit20~bit08  8λΪ ��ħ��"(Ҳ��Ϊ"����")�������ֵ�����������豸��������� ioctl �����������
bit07~bit00   8λΪ "�������" �������������������˳����š�

_IO (ħ���� ����);
_IOR (ħ���� ������ ������)
_IOW  (ħ���� ������ ������)
һ�������������ΪCMDָ��Ĳ�������һ��ħ���������趨һ����ĸ���ڶ���Ϊָ���ֵ��������ֱ��дint�������ͼ��ɣ�ֻҪ������������������ӣ�Ȼ��ʹ�ú궨�壬���ɲ����ַ��豸�ĺ���
*/
#define TIMER_SET (200*HZ)/1000 /*�趨��ʱ���Ķ�ʱֵ������Ϊ0.2HZ*/

static int major;	/*����һ�����ڱ���major��ֵ������ָ��ֵ��Ҳ�������ɷ���*/
static struct class *jt_gpio_class;	/*����һ��class������ע���豸*/
static struct device *jt_gpio_device; /*��device�Զ����뵽�豸�б��У��������*/
static struct timer_list test_timer;	/*��ʱ�������ڶ�ʱ*/
unsigned char ledflag = 1;		/*�Ƿ���˸��flag������ʹ����������*/
MODULE_LICENSE("Dual BSD/GPL");	/*��������*/
MODULE_AUTHOR("TOPEET");/*��������*/

/*
*��ʱ��������,���ڴ���ʱ������жϣ���Ҫ���嵽��ʱ��֮ǰ
*/
static void sec_timer_handler(unsigned long arg)
{
	int num;
	mod_timer(&test_timer,jiffies+TIMER_SET);	/*���³�ʼ����ʱ��*/
	if(ledflag > 0)
	{
		gpio_set_value(NUC980_PB8, 0);
		ledflag  = 0;
	}
	else
	{
		gpio_set_value(NUC980_PB8, 1);
		ledflag  = 1;
	}

//	atomic_inc(&led_devp->sec_counter);		//ԭ�ӱ�����ֵ��һ�����ɱ����
//	num = atomic_read(&led_devp->sec_counter);	//ԭ�ӱ�����ֵ��ȡ�����ɱ����
//	printk(KERN_INFO "sec_count:%d\n",num);
}
/**
*��ʼ����صĶ�ʱ����ֵ����֤��ʱ�����У���������Ӧ��ͷ�ļ�
*/
static int led_shun(void)
{
//	struct timer_list *timer;
//	timer = &test_timer;

	printk(KERN_INFO "timer init \n");
	test_timer.function = sec_timer_handler;
	test_timer.expires = jiffies + TIMER_SET;		  //��ʱƵ��ΪHZ HZһ��Ϊ100�����Դ˴����Լ�����˸
	printk(KERN_INFO "timer HZ \n");

	init_timer(&test_timer);  /*��ʼ��һ��Ҫ�ŵ���ֵ�����ĺ��棬�������ִ��󣬴˴��������Ӵ�������*/
	add_timer(&test_timer); /*���趨�õ�timer���뵽�б��У������Ӵ�������*/
	return 0;
}

/*��������*/
static long gpio_PB8_ioctl( struct file *files, unsigned int cmd, unsigned long arg){

	printk("cmd is %d,arg is %d \n",cmd,(unsigned int)(arg));

	switch(cmd){
		case IOCTL_LED_ON:{
			del_timer(&test_timer);
			gpio_set_value(NUC980_PB8, 0);
			break;
		}
		case IOCTL_LED_OFF:{
			del_timer(&test_timer);
			gpio_set_value(NUC980_PB8, 1);
			break;
		}
		case IOCTL_LED_RUN:{
			/*for (i=0;i<arg;i++)
				for (j=0;j<4;j++) {
					gpio_set_value(NUC980_PB8, 0);
					msleep(400); //delay 400ms
					gpio_set_value(NUC980_PB8, 1);
					msleep(400); //delay 400ms
				}
				*/
				led_shun();
		}
		default:
			break;
	}
    return 0;
}
/*�ͷź���*/
static int gpio_PB8_release(struct inode *inode, struct file *file){
    printk(KERN_EMERG "gpio_PB8 release\n");
    return 0;
}
/*�򿪺���*/
static int gpio_PB8_open(struct inode *inode, struct file *file){
    printk(KERN_EMERG "gpio_PB8 open\n");
    return 0;
}
/*ops�ṹ�壬�洢��صĲ�������*/
static struct file_operations gpio_PB8_ops = {
    .owner					= THIS_MODULE,
    .open						= gpio_PB8_open,
    .release				= gpio_PB8_release,
    .unlocked_ioctl = gpio_PB8_ioctl,
};

static int gpio_PB8_init(void)
{
    int DriverState;
    int ret;

    printk(KERN_EMERG "module 20190824 !\n");
    ret = gpio_request(NUC980_PB8,"NUC980_PB8");
    if(ret < 0){
        printk(KERN_EMERG "gpio_request NUC980_PB8 failed!\n");
        return ret;
    }
    gpio_direction_output(NUC980_PB8,1);
    gpio_set_value(NUC980_PB8, 0);
		major = register_chrdev(0, "gpio_PB8_gpio", &gpio_PB8_ops);

		jt_gpio_class = class_create(THIS_MODULE, "jt_gpio_class");
    if (!jt_gpio_class) {
        printk(KERN_INFO "jt_gpio class_create fail\n");
        return -1;
    }
    jt_gpio_device = device_create(jt_gpio_class, NULL, MKDEV(major, 0), NULL, "gpio_PB8_gpio");
    if (!jt_gpio_device) {
        printk(KERN_INFO "jt_gpio device_create fail\n");
        return -1;
    }

    printk(KERN_EMERG "DriverState is %d\n",DriverState);
    return 0;
}

static void gpio_PB8_exit(void)
{
    printk(KERN_EMERG "module 20190824 exit!\n");

    unregister_chrdev(major,"gpio_PB8_gpio");
    gpio_free(NUC980_PB8);
    device_unregister(jt_gpio_device);
    class_destroy(jt_gpio_class);
	del_timer(&test_timer);
}

module_init(gpio_PB8_init);
module_exit(gpio_PB8_exit);

