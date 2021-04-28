#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include "tools.h"

#define pf printf

char cmd[20] = {};

// 开始运行
void *start_run(void *arg);
// 客户端上传
void c_up(int *clifd);
// 客户端下载
void c_down(int *clifd);
// 返回文件列表
void c_list(int *clifd);

typedef struct LS
{
	char mode[15];	// 文件的模式
	int dir_num;	// 是否目录或目录中包含目录的数量
	char user[20];	// 文件的用户名
	char group[20]; // 文件的组名
	long size;		// 文件的字节数
	char time[30];	// 文件的最后修改时间
	int st_mode;	// 文件类型和权限
	char name[20];	// 文件名
} LS;

// 主函数
int main()
{
	pf("服务器创建socket...\n");
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > sockfd)
	{
		perror("socket");
		return -1;
	}

	pf("准备地址...\n");
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	// 端口 IP 自行修改
	addr.sin_port = htons(60000);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	socklen_t len = sizeof(addr);

	pf("绑定socket与地址...\n");
	if (bind(sockfd, (struct sockaddr *)&addr, len))
	{
		perror("bind");
		return -1;
	}

	pf("设置监听...\n");
	if (listen(sockfd, 5))
	{
		perror("listen");
		return -1;
	}

	pf("等待客户端连接...\n");
	for (;;)
	{
		struct sockaddr_in addrcli = {};
		// int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
		int *clifd = (int*)malloc(sizeof(int));
		*clifd = accept(sockfd, (struct sockaddr *)&addrcli, &len);
		if (0 > *clifd)
		{
			perror("accept");
			continue;
		}

		pthread_t pid;
		// 创建线程函数 int pthread_create(pthread_t *restrict tidp,const pthread_attr_t *restrict_attr,void*（*start_rtn)(void*),void *restrict arg);
		//第一个参数为指向线程标识符的指针。
		//第二个参数用来设置线程属性。
		//第三个参数是线程运行函数的地址。
		//最后一个参数是运行函数的参数。
		pthread_create(&pid, NULL, start_run, (void *)clifd);
	}

	return 0;
}

// 开始运行
void *start_run(void *arg)
{
	int *clifd = (int *)arg;
	char up[20] = "我想上你";
	char down[20] = "我想下你";
	char see[20] = "我想看你";
	char quit[20] = "我要走了";
	int c_size = 0;
	for (;;)
	{
		c_size = read(*clifd, cmd, sizeof(cmd));
		if(-1 == c_size)
		{
			pf("read函数出错！\n");
		}
		
		if (strcmp(up, cmd) == 0)
		{
			pf("收到客户端的上传指令\n");
			c_up(clifd);
			memset(cmd, 0, 20);
		}
		else if (strcmp(down, cmd) == 0)
		{
			pf("收到客户端的下载指令\n");
			c_down(clifd);
			memset(cmd, 0, 20);
		}
		else if (strcmp(see, cmd) == 0)
		{
			pf("收到客户端的目录指令\n");
			c_list(clifd);
			memset(cmd, 0, 20);
		}
		else if (strcmp(quit, cmd) == 0)
		{
			pf("收到服务端的退出指令\n");
			pthread_exit(0);
			return (void *)NULL;
		}
		
	}
	//char *str = "我死了";
	//pthread_exit(str);
}

// 上传
void c_up(int *clifd)
{
	int flag = 0;
	int r_size = 0;
	int w_size = 0;
	char buf[1024] = {};

	w_size = write(*clifd, "success", 8);
	read(*clifd, buf, 10);
	if(strncmp(buf, "error", 10) == 0)
	{
		printf("收到客户端返回error,接收终止\n");
		return;
	}
	else if(strncmp(buf, "success", 10) == 0)
	{
		printf("收到客户端返回success,继续接收\n");
	}
	else
	{
		printf("收到客户端异常数据:%s,接收终止\n", buf);
		return;
	}
	

	char filename[50] = {};
	memset(filename, 0, sizeof(filename));
	int f_size = read(*clifd, filename, sizeof(filename));
	if(-1 == f_size)
	{
		pf("read函数出错！\n");
	}
	pf("收到文件名:%s\n", filename);
	usleep(100000);
	w_size = write(*clifd, "success", 8);
	pf("发送success给客户端\n");

	int fd = open(filename, O_CREAT | O_RDWR, 0777);

	do
	{
		memset(buf, 0, sizeof(buf));
		r_size = read(*clifd, buf, sizeof(buf));
		pf("[收到字节数:%d ", r_size);

		w_size = write(fd, buf, r_size);
		pf("写入文件字节数:%d ", w_size);

		usleep(10000);
		
		w_size = write(*clifd, "success", 8);
		pf("发送success给客户端 ]\n");

		flag++;
	} while (r_size == 1024);

	sleep(1);

	if (flag > 0)
	{
		char result[20] = "success";
		pf("    文件传输完毕 返回客户端success\n\n");
		write(*clifd, result, strlen(result) + 1);
	}
	else
	{
		char result[20] = "error";
		pf("    文件传输失败 返回客户端error\n\n");
		write(*clifd, result, strlen(result) + 1);
	}
	close(fd);
	return;
}

// 下载
void c_down(int *clifd)
{
	DIR *dir;
	dir = opendir(".");
	char list[1024] = {};
	struct dirent *dirent;
	int r_size = 0;
	int w_size = 0;
	char buf[1024] = {};
	char buf2[20] = {};
	char filename[50] = {};
	char filename2[51] = {};

	usleep(10000);
	w_size = write(*clifd, "success", 8);

	usleep(10000);

	// 获取目录下所有文件名
	while ((dirent = readdir(dir)) != NULL)
	{
		strcat(list, dirent->d_name);
		strcat(list, " ");
	}
	pf("当前目录列表:%s\n", list);
	pf("strlen(list):%d\n", (int)strlen(list));
	int l_size = write(*clifd, list, strlen(list)+1);
	if(-1 == l_size)
	{
		pf("read函数出错！\n");
	}
	pf("发送当前下载目录列表给客户端\n");

	pf("等待接收文件名...\n");
	int f_size = read(*clifd, filename, sizeof(filename));
	if(-1 == f_size)
	{
		pf("read函数出错！\n");
	}
	//pf("filename:%s\n", filename);
	strncpy(filename2, filename, 50);
	strcat(filename2, " ");
	if (strstr(list, filename2) == NULL || strncmp(filename2, " ", 1) == 0 || strncmp(filename2, "  ", 2) == 0)
	{
		char result[6] = "error";
		pf("文件:%s 不存在,下载终止\n", filename);

		write(*clifd, result, strlen(result));
		return;
	}
	else
	{
		char result[8] = "success";
		pf("文件:%s 存在,开始传输文件内容\n", filename);
		write(*clifd, result, strlen(result));

		int fd = open(filename, O_RDONLY);

		//设置文件读写位置为文件尾部
		lseek(fd, 0, SEEK_END);
		// 获取文件字节数（尾部位置）
		off_t end_pos = lseek(fd, 0, SEEK_CUR);
		//pf("end_pos:%d\n", end_pos);
		//设置文件读写位置为文件头部
		lseek(fd, 0, SEEK_SET);

		usleep(1000000);

		do
		{
			memset(buf, 0, sizeof(buf));
			r_size = read(fd, buf, sizeof(buf));
			pf("[读取文件字节数:%d ", r_size);
			w_size = write(*clifd, buf, r_size);
			pf("发送字节数:%d ", w_size);
			read(*clifd, result, sizeof(result));
			if(strncmp(result, "success", 10) == 0)
			{
				pf("成功收到客户端端返回的success]\n");
			}
			usleep(10000);

			off_t cur_pos = lseek(fd, 0, SEEK_CUR);
			//pf("cur_pos:%d\n", cur_pos);
			if(cur_pos == end_pos && w_size == 1024)
			{
				char end[1] = "\0";
				pf("[读取文件字节数:1 ");
				w_size = write(*clifd, end, sizeof(end));
				pf("发送字节数:%d ", w_size);
				read(*clifd, buf2, sizeof(buf2));
				
				if(strncmp(buf2, "success", 10) == 0)
				{
					pf("成功收到客户端端返回的success]\n");
				}
				else
				{
					pf("收到客户端返回的异常数据:%s]\n", buf2);
				}
				break;
			}
		} while (r_size == 1024);
		usleep(1000000);
		pf("    文件:%s 发送完毕\n", filename);
		close(fd);
	}

	return;
}

// 文件列表
void c_list(int *clifd)
{
	DIR *dir;
	dir = opendir(".");
	char list[1024] = {};
	struct dirent *dirent;
	while ((dirent = readdir(dir)) != NULL)
	{
		strcat(list, dirent->d_name);
		strcat(list, " ");
	}

	int l_size = write(*clifd, list, strlen(list) + 1);
	if(-1 == l_size)
	{
		pf("write函数出错！\n");
	}

	memset(list, 0, 1024);

	char dirname[20] = {};
	int d_size = read(*clifd, dirname, sizeof(dirname));
	if(-1 == d_size)
	{
		pf("read函数出错！\n");
	}
	pf("收到客户端的数据:%s\n", dirname);

	if(strncmp(dirname, "...", 20) == 0)
	{
		closedir(dir);
		return;
	}

	if (strncmp(dirname, ".", 20) == 0)
	{
		dir = opendir(".");
		while ((dirent = readdir(dir)) != NULL)
		{
			strcat(list, dirent->d_name);
			strcat(list, " ");
		}
		l_size = write(*clifd, list, strlen(list) + 1);
	}
	else if (strncmp(dirname, "..", 20) == 0)
	{
		chdir("..");
		dir = opendir(".");
		while ((dirent = readdir(dir)) != NULL)
		{
			strcat(list, dirent->d_name);
			strcat(list, " ");
		}
		l_size = write(*clifd, list, strlen(list) + 1);
	}
	else
	{
		int re = chdir(dirname);
		if (re == -1)
		{
			char result[20] = "目录名错误";
			int err = write(*clifd, result, strlen(result) + 1);
			if(-1 == err)
			{
				pf("write函数出错！\n");
			}
		}
		else
		{
			dir = opendir(".");
			while ((dirent = readdir(dir)) != NULL)
			{
				strcat(list, dirent->d_name);
				strcat(list, " ");
			}
			l_size = write(*clifd, list, strlen(list) + 1);
		}
	}
	closedir(dir);
	return;
}
