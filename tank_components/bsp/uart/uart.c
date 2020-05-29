#include "uart.h"
#include<sys/signal.h>

#define flag 1
#define noflag 0

int wait_flag = noflag;

static int serial_fd = 0;
uart_recv_cb_t uart_recv_cb = NULL;


void signal_handler_IO (int status)
{
    printf ("received SIGIO signale.\n");
    int len = 0;
    char recv_buf[1000] = {0};
    len = read(serial_fd, recv_buf, sizeof(recv_buf)-1);
    if(len > 0){
        uart_recv_cb(recv_buf, (uint16_t*)&len);
        printf("[UART]recv a data, len is %d\n", len);
        ftruncate(serial_fd, 0);
    }else{
        printf("[UART]get error\n");
    };
}

tank_status_t hal_uart_init(const char *device_name, uart_recv_cb_t cb)
{
	serial_fd = open(device_name, O_RDWR | O_NOCTTY | O_NDELAY);
	if (serial_fd < 0) {
		perror("open uart device");
		return TANK_FAIL;
	}

    struct sigaction saio;

    saio.sa_handler = signal_handler_IO;
    sigemptyset (&saio.sa_mask);
    saio.sa_flags = 0;
    saio.sa_restorer = NULL;
    sigaction (SIGIO, &saio, NULL);

    //allow the process to receive SIGIO
    fcntl (serial_fd, F_SETOWN, getpid());
    //make the file descriptor asynchronous
    fcntl (serial_fd, F_SETFL, FNDELAY|FASYNC);

	//串口主要设置结构体termios <termios.h>
	struct termios options;

	/**1. tcgetattr函数用于获取与终端相关的参数。
	*参数fd为终端的文件描述符，返回的结果保存在termios结构体中
	*/
	tcgetattr(serial_fd, &options);
	/**2. 修改所获得的参数*/
	options.c_cflag |= (CLOCAL | CREAD);//设置控制模式状态，本地连接，接收使能
	options.c_cflag &= ~CSIZE;//字符长度，设置数据位之前一定要屏掉这个位
	options.c_cflag &= ~CRTSCTS;//无硬件流控
	options.c_cflag |= CS8;//8位数据长度
	options.c_cflag &= ~CSTOPB;//1位停止位
	options.c_iflag |= IGNPAR;//无奇偶检验位
	options.c_oflag = 0; //输出模式
	options.c_lflag = 0; //不激活终端模式
	cfsetospeed(&options, B115200);//设置波特率

	/**3. 设置新属性，TCSANOW：所有改变立即生效*/
	tcflush(serial_fd, TCIOFLUSH);//溢出数据可以接收，但不读
	tcsetattr(serial_fd, TCSANOW, &options);
    uart_recv_cb = cb;
	return TANK_SUCCESS;
}


int hal_uart_send(char *data, int datalen)
{
	int len = 0;
	len = write(serial_fd, data, datalen);//实际写入的长度
	if(len == datalen) {
        printf("[UART]write data success\n");
		return len;
	} else {
		tcflush(serial_fd, TCOFLUSH);//TCOFLUSH刷新写入的数据但不传送
        printf("[UART]the data write into not macth\n");
		return TANK_FAIL;
	}
	return TANK_SUCCESS;
}

tank_status_t hal_uart_read(void)
{

	return TANK_SUCCESS;
}