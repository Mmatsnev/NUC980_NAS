#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

/*时间和定时器相关的文件，如果不使用sleep和mdelay等，可以去掉delay函数，如果不*使用定时器，可以去掉定时器相关函数
*/
#include <linux/delay.h>
#include<linux/timer.h>
#include<linux/jiffies.h>
/*驱动注册的头文件，包含驱动的结构体和注册和卸载的函数*/
#include <linux/platform_device.h>
/*注册杂项设备头文件*/
#include <linux/miscdevice.h>
#include <linux/device.h>
/*注册设备节点的文件结构体*/
#include <linux/fs.h>
/*Linux中申请GPIO的头文件*/
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
由宏定义组成的一组32位数据
bit31~bit30 2位为 “区别读写” 区，作用是区分是读取命令还是写入命令。
bit29~bit15 14位为 "数据大小" 区，表示 ioctl() 中的 arg 变量传送的内存大小。
bit20~bit08  8位为 “魔数"(也称为"幻数")区，这个值用以与其它设备驱动程序的 ioctl 命令进行区别。
bit07~bit00   8位为 "区别序号" 区，是区分命令的命令顺序序号。

_IO (魔数， 基数);
_IOR (魔数， 基数， 变量型)
_IOW  (魔数， 基数， 变量型)
一般情况下用于作为CMD指令的参量，第一个魔数，可以设定一个字母，第二个为指令的值，第三个直接写int数据类型即可，只要在主程序将上述代码添加，然后使用宏定义，即可操作字符设备的函数
*/
#define TIMER_SET (200*HZ)/1000 /*设定定时器的定时值，亮灭都为0.2HZ*/

#define CAP_VSYNC_GPIO  NUC980_PB6
#define STR_CAP_VSYNC_GPIO  "NUC980_PB6"
#define LED_GPIO  NUC980_PB8
#define STR_LED_GPIO  "NUC980_PB8"

static int major;	/*定义一个用于保存major的值，可以指定值，也可以自由分配*/
static struct class *ts_gpio_class;	/*定义一个class，用于注册设备*/
static struct device *ts_gpio_device; /*将device自动加入到设备列表中，方便操作*/
static struct timer_list test_timer;	/*定时器，用于定时*/
unsigned char ledflag = 1;		/*是否闪烁的flag，可以使用其他名称*/
MODULE_LICENSE("Dual BSD/GPL");	/*常规描述*/
MODULE_AUTHOR("Ryan");/*常规描述*/

/*
*定时器处理函数,用于处理定时器相关中断，需要定义到定时器之前
*/
static void sec_timer_handler(unsigned long arg)
{
	int num;
	mod_timer(&test_timer,jiffies+TIMER_SET);	/*重新初始化定时器*/
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

//	atomic_inc(&led_devp->sec_counter);		//原子变量的值加一，不可被打断
//	num = atomic_read(&led_devp->sec_counter);	//原子变量的值读取，不可被打断
//	printk(KERN_INFO "sec_count:%d\n",num);
}
/**
*初始化相关的定时器数值，保证定时器运行，需依赖相应的头文件
*/
static int led_shun(void)
{
//	struct timer_list *timer;
//	timer = &test_timer;

	printk(KERN_INFO "timer init \n");
	test_timer.function = sec_timer_handler;
	test_timer.expires = jiffies + TIMER_SET;		  //计时频率为HZ HZ一般为100，所以此处可以加速闪烁
	printk(KERN_INFO "timer HZ \n");

	init_timer(&test_timer);  /*初始化一定要放到赋值函数的后面，否则会出现错误，此处可以增加错误处理函数*/
	add_timer(&test_timer); /*将设定好的timer加入到列表中，可增加错误处理函数*/
	return 0;
}

/*操作函数*/
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
/*释放函数*/
static int TS_GPIO_release(struct inode *inode, struct file *file){
    printk(KERN_EMERG "TS_GPIO release\n");
    return 0;
}
/*打开函数*/
static int TS_GPIO_open(struct inode *inode, struct file *file){
    printk(KERN_EMERG "TS_GPIO open\n");
    return 0;
}
/*ops结构体，存储相关的操作函数*/
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

