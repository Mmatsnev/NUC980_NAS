#include <stdio.h>	/*标准的输入输出函数库*/
#include <stdlib.h>	/*标准的输入输出函数库*/
#include <unistd.h> /*对于read和write等函数的支持*/
#include <sys/ioctl.h> /*IO指令流函数，如cmd等，除了打开函数之外，其他的函数定义*/

#define LED_MAGIC 'k'
#define IOCTL_LED_ON _IOW (LED_MAGIC, 1, int)
#define IOCTL_LED_OFF _IOW (LED_MAGIC, 2, int)
#define IOCTL_LED_RUN _IOW (LED_MAGIC, 3, int)
#define IOCTL_LED_SHINE _IOW (LED_MAGIC, 4, int)
#define IOCTL_LED_ALLON _IOW (LED_MAGIC, 5, int)
#define IOCTL_LED_ALLOFF _IOW (LED_MAGIC, 6, int)

void usage(char *exename)
{
    printf("Usage:\n");
    printf("    %s <led_no> <on/off>\n", exename);
    printf("    led_no = 1, 2, 3 or 4\n");
} 
   
int main(int argc, char **argv)
{
    unsigned int led_no;
    int fd = -1;
    unsigned int count=10;
    
    if (argc > 3 || argc == 1)
        goto err;
        
    fd = open("/dev/gpio_PB8_gpio", 0);  // 打开设备
    if (fd < 0) {
        printf("Can't open /dev/gpio_PB8_gpio\n");
        return -1;    
    }    
    
	printf("argc = %d\n",argc);
    if (argc == 2) {
        if (!strcmp(argv[1], "on")) {
			printf("argc[1] = on\n");
            ioctl(fd, IOCTL_LED_ON, count);    // 点亮它
        } 
		else if (!strcmp(argv[1], "off")) {
			printf("argc[1] = off\n");
            ioctl(fd, IOCTL_LED_OFF, count);   // 熄灭它
        } 
		else if (!strcmp(argv[1], "run")) {
            ioctl(fd, IOCTL_LED_RUN, count);   //运行跑马灯
                } 
        } 
	else {
            goto err;
        }        
    close(fd);
	return 0;        
err:
    if (fd > 0) 
        close(fd);
    usage(argv[0]);
    return -1;        
}

