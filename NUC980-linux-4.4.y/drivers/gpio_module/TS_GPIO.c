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
#include <mach/regs-cap.h>
#include <mach/regs-gcr.h>
#include <mach/regs-clock.h>
//#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>

#define LED_MAGIC 'k'
#define IOCTL_LED_ON  _IOW (LED_MAGIC, 1, int)
#define IOCTL_LED_OFF _IOW (LED_MAGIC, 2, int)
#define IOCTL_LED_RUN _IOW (LED_MAGIC, 3, int)
#define IOCTL_CAP_VH  _IOW (LED_MAGIC, 4, int)
#define IOCTL_CAP_VL  _IOW (LED_MAGIC, 5, int)
#define IOCTL_READ_REG _IOW (LED_MAGIC, 6, int)
#define IOCTL_READ_DATA _IOW (LED_MAGIC, 7, int)
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

#define CAP_VSYNC_GPIO  NUC980_PB6
#define STR_CAP_VSYNC_GPIO  "NUC980_PB6"
#define LED_GPIO  NUC980_PB8
#define STR_LED_GPIO  "NUC980_PB8"

static int major;	/*����һ�����ڱ���major��ֵ������ָ��ֵ��Ҳ�������ɷ���*/
static struct class *ts_gpio_class;	/*����һ��class������ע���豸*/
static struct device *ts_gpio_device; /*��device�Զ����뵽�豸�б��У��������*/
static struct timer_list test_timer;	/*��ʱ�������ڶ�ʱ*/
unsigned char ledflag = 1;		/*�Ƿ���˸��flag������ʹ����������*/
MODULE_LICENSE("Dual BSD/GPL");	/*��������*/
MODULE_AUTHOR("Ryan");/*��������*/

/*
*��ʱ��������,���ڴ���ʱ������жϣ���Ҫ���嵽��ʱ��֮ǰ
*/
static void sec_timer_handler(unsigned long arg)
{
	int num;
	mod_timer(&test_timer,jiffies+TIMER_SET);	/*���³�ʼ����ʱ��*/
	if(ledflag > 0)
	{
		gpio_set_value(LED_GPIO, 0);
		ledflag  = 0;
	}
	else
	{
		gpio_set_value(LED_GPIO, 1);
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
static long TS_GPIO_IOCTL( struct file *files, unsigned int cmd, unsigned long arg){

	printk("cmd is %d,arg is %d \n",cmd,(unsigned int)(arg));

	switch(cmd){
		case IOCTL_LED_ON:
		{
			del_timer(&test_timer);
			gpio_set_value(LED_GPIO, 0);
			break;
		}
		case IOCTL_LED_OFF:
		{
			del_timer(&test_timer);
			gpio_set_value(LED_GPIO, 1);
			break;
		}
		case IOCTL_LED_RUN:
		{
				led_shun();
				break;
		}
		case IOCTL_CAP_VH:
		{
            gpio_set_value(CAP_VSYNC_GPIO, 1);
            break;
		}
		case IOCTL_CAP_VL:
		{
            gpio_set_value(CAP_VSYNC_GPIO, 0);
            break;
		}
		case IOCTL_READ_REG:
		{
            printk("GCR_BA+0x080=0x%08x\n",__raw_readl(GCR_BA+0x080));
            printk("GCR_BA+0x084=0x%08x\n",__raw_readl(GCR_BA+0x084));
            printk("CLK_HCLKEN=0x%08x\n",__raw_readl(CLK_BA+0x010));
            printk("REG_CLK_DIV3=0x%08x\n",__raw_readl(CLK_BA+0x02C));
            printk("VDIN0 ISR status=0x%08x\n",__raw_readl(REG_CAP0_INT));
            printk("REG_CAP0_CTL=0x%08x\n",__raw_readl(REG_CAP0_CTL));
            printk("REG_CAP0_PAR=0x%08x\n",__raw_readl(REG_CAP0_PAR));
            printk("REG_CAP0_CWSP=0x%08x\n",__raw_readl(REG_CAP0_CWSP));
            printk("REG_CAP0_CWS=0x%08x\n",__raw_readl(REG_CAP0_CWS));
            printk("REG_CAP0_PKTSM=0x%08x\n",__raw_readl(REG_CAP0_PKTSM));
            printk("REG_CAP0_PKTSL=0x%08x\n",__raw_readl(REG_CAP0_PKTSL));
            printk("REG_CAP0_MD=0x%08x\n",__raw_readl(REG_CAP0_MD));
            printk("REG_CAP0_FIFOTH=0x%08x\n",__raw_readl(REG_CAP0_FIFOTH));
            printk("REG_CAP0_FRCTL=0x%08x\n",__raw_readl(REG_CAP0_FRCTL));
            printk("REG_CAP0_CURADDRP=0x%08x\n",__raw_readl(REG_CAP0_CURADDRP));
//            printk("REG_CAP0_CURADDRY=0x%08x\n",__raw_readl(REG_CAP0_CURADDRY));
//            printk("REG_CAP0_CURADDRU=0x%08x\n",__raw_readl(REG_CAP0_CURADDRU));
//            printk("REG_CAP0_CURADDRV=0x%08x\n",__raw_readl(REG_CAP0_CURADDRV));
//            printk("REG_CAP0_YBA=0x%08x\n",__raw_readl(REG_CAP0_YBA));
//            printk("REG_CAP0_UBA=0x%08x\n",__raw_readl(REG_CAP0_UBA));
//            printk("REG_CAP0_VBA=0x%08x\n",__raw_readl(REG_CAP0_VBA));
            printk("REG_CAP0_PKTBA0=0x%08x\n",__raw_readl(REG_CAP0_PKTBA0));
            break;
		}
//		case IOCTL_READ_DATA:
//		{
//            printk("REG_CAP0_PKTBA0_DATA=0x%08x\n",__raw_readl((void *)(__raw_readl(REG_CAP0_PKTBA0))));
//            break;
//		}
		default:
			break;
	}
    return 0;
}
/*�ͷź���*/
static int TS_GPIO_release(struct inode *inode, struct file *file){
    printk(KERN_EMERG "TS_GPIO release\n");
    return 0;
}
/*�򿪺���*/
static int TS_GPIO_open(struct inode *inode, struct file *file){
    printk(KERN_EMERG "TS_GPIO open\n");
    return 0;
}
/*ops�ṹ�壬�洢��صĲ�������*/
static struct file_operations TS_GPIO_ops = {
    .owner					= THIS_MODULE,
    .open					= TS_GPIO_open,
    .release				= TS_GPIO_release,
    .unlocked_ioctl = TS_GPIO_IOCTL,
};

static int TS_GPIO_init(void)
{
    int DriverState;
    int ret;

    printk(KERN_EMERG "module TS GPIO init...!\n");
    ret = gpio_request(LED_GPIO,STR_LED_GPIO);
    if(ret < 0){
        printk(KERN_EMERG "gpio_request LED_GPIO failed!\n");
        return ret;
    }
    gpio_direction_output(LED_GPIO,1);
    gpio_set_value(LED_GPIO, 0);

    ret = gpio_request(CAP_VSYNC_GPIO,STR_CAP_VSYNC_GPIO);
    if(ret < 0){
        printk(KERN_EMERG "gpio_request CAP_VSYNC_GPIO failed!\n");
        return ret;
    }
    gpio_direction_output(CAP_VSYNC_GPIO,1);
    gpio_set_value(CAP_VSYNC_GPIO, 0);

    major = register_chrdev(0, "TS_GPIO", &TS_GPIO_ops);

	ts_gpio_class = class_create(THIS_MODULE, "ts_gpio_class");
    if (!ts_gpio_class) {
        printk(KERN_INFO "ts_gpio_class create fail\n");
        return -1;
    }
    ts_gpio_device = device_create(ts_gpio_class, NULL, MKDEV(major, 0), NULL, "TS_GPIO");
    if (!ts_gpio_device) {
        printk(KERN_INFO "TS_gpio device_create fail\n");
        return -1;
    }

    printk(KERN_EMERG "DriverState is %d\n",DriverState);
    return 0;
}

static void TS_GPIO_exit(void)
{
    printk(KERN_EMERG "module TS_GPIO exit!\n");

    unregister_chrdev(major,"TS_GPIO");
    gpio_free(LED_GPIO);
    gpio_free(CAP_VSYNC_GPIO);
    device_unregister(ts_gpio_device);
    class_destroy(ts_gpio_class);
	del_timer(&test_timer);
}

module_init(TS_GPIO_init);
module_exit(TS_GPIO_exit);

