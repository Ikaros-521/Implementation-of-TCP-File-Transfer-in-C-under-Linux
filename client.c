#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include "tools.h"

#define pf printf

int sockfd = 0;

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

// 菜单
void menu(void);
// 上传
void upload(void);
// 下载
void download(void);
// 显示服务器目录和文件
void s_list(void);
// 显示客户端目录和文件
void c_list(void);
// 退出程序
void quit(void);

// 主函数
int main()
{
	pf("服务器创建socket...\n");
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (0 > sockfd)
	{
		perror("socket");
		return -1;
	}

	pf("准备地址...\n");
	struct sockaddr_in addr = {};
	addr.sin_family = AF_INET;
	// 端口自己修改
	addr.sin_port = htons(60000);
	// IP自行修改
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	socklen_t len = sizeof(addr);

	pf("绑定连接服务器...\n");
	if (connect(sockfd, (struct sockaddr *)&addr, len))
	{
		perror("connect");
		return -1;
	}

	menu(); // 加载菜单

	close(sockfd);

	return 0;
}

// 加载菜单
void menu(void)
{
	for (;;)
	{
		system("clear");
		pf("*** 局域网文件传输客户端 ***\n");
		pf("     1、上传\n");
		pf("     2、下载\n");
		pf("     3、查看/修改服务端目录\n");
		pf("     4、查看客户端目录\n");
		pf("     0、退出\n");
		pf("--------------------------\n");
		// 我的自定义函数 在tools.c里面 get_cmd
		switch (get_cmd('0', '4'))
		{
			case '1':
				upload();
				break;
			case '2':
				download();
				break;
			case '3':
				s_list();
				break;
			case '4':
				c_list();
				break;
			case '0':
				quit();
				return;
		}
	}
}

// 上传
void upload(void)
{
	char up[20] = "我想上你";
	write(sockfd, up, strlen(up) + 1);

	// 打印当前目录文件
	c_list();

	int r_size = 0;
	int w_size = 0;
	char buf[1024] = {};
	char buf2[20] = {};
	r_size = read(sockfd, buf, sizeof(buf));
	if(strncmp(buf, "success", 10) != 0)
	{
		pf("收到服务端异常数据\n");
		getch();
		return;
	}

	pf("请输入文件名:");
	char pathname[100] = {};
	char *filename = malloc(50);
	memset(filename, 0, 50);
	get_str(pathname, 100);
	int fd = open(pathname, O_RDONLY);

	struct stat stat = {};
	int fs = fstat(fd, &stat);
	long file_size = 0;
	file_size = stat.st_size;

	if (fd == -1)
	{
		pf("文件不存在\n");
		write(sockfd, "error", 6);
		getch();
	}
	else
	{
		write(sockfd, "success", 8);
		usleep(100000);

		if (strrchr(pathname, '/') == NULL)
		{
			strncpy(filename, pathname, 50);
		}
		else
		{
			filename = strrchr(pathname, '/');
			filename += 1;
		}

		pf("发送文件名:%s 至服务端\n", filename);
		write(sockfd, filename, strlen(filename) + 1);

		usleep(100000);

		memset(buf, 0, sizeof(buf));
		r_size = read(sockfd, buf, sizeof(buf));
		if(strncmp(buf, "success", 10) != 0)
		{
			pf("收到服务端异常数据\n");
			getch();
			return;
		}
		else
		{
			pf("收到服务端返回success\n");
		}
		
		sleep(1);

		//设置文件读写位置为文件尾部
		lseek(fd, 0, SEEK_END);
		// 获取文件字节数（尾部位置）
		off_t end_pos = lseek(fd, 0, SEEK_CUR);
		//pf("end_pos:%d\n", end_pos);
		//设置文件读写位置为文件头部
		lseek(fd, 0, SEEK_SET);

		do
		{
			//pf("  进入while循环...\n");
			r_size = read(fd, buf, 1024);

			pf("[读取文件字节数:%d ", r_size);

			w_size = write(sockfd, buf, r_size);
			pf("发送字节数:%d ", w_size);

			read(sockfd, buf2, sizeof(buf2));
			if(strncmp(buf2, "success", 10) == 0)
			{
				pf("成功收到服务端返回的success]\n");
			}
			usleep(10000);

			off_t cur_pos = lseek(fd, 0, SEEK_CUR);
			//pf("cur_pos:%d\n", cur_pos);
			if(cur_pos == end_pos && w_size == 1024)
			{
				char end[1] = "\0";
				pf("[读取文件字节数:1 ");
				w_size = write(sockfd, end, sizeof(end));
				pf("发送字节数:%d ", w_size);
				read(sockfd, buf2, sizeof(buf2));
				
				if(strncmp(buf2, "success", 10) == 0)
				{
					pf("成功收到服务端返回的success]\n");
				}
				else
				{
					pf("收到服务端返回的异常数据:%s]\n", buf2);
				}
				break;
			}
		} while (r_size == 1024);

		close(fd);

		char result[20] = {};
		read(sockfd, result, sizeof(result));
		if(strncmp(buf2, "success", 10) == 0)
		{
			pf("成功收到服务端返回值:%s,服务器接收文件成功\n", result);
		}
		else if(strncmp(buf2, "error", 10) == 0)
		{
			pf("成功收到服务端返回值:%s,服务器接收文件异常\n", result);
		}
		else
		{
			pf("收到服务端返回值:%s,数据异常\n", result);
		}
			
		getch();
	}

	return;
}

// 下载
void download(void)
{
	int r_size = 0;
	int w_size = 0;
	char buf[1024] = {};
	char filename[50] = {};
	char list[1024] = {};
	char down[20] = "我想下你";
	write(sockfd, down, strlen(down) + 1);

	r_size = read(sockfd, buf, sizeof(buf));
	if(strncmp(buf, "success", 10) != 0)
	{
		pf("收到服务端异常数据\n");
		getch();
		return;
	}
	else
	{
		pf("服务端成功接收命令\n");
	}

	read(sockfd, list, sizeof(list));
	pf("服务端目录列表:%s\n", list);

	usleep(1000);

	while(1)
	{
		pf("请输入要下载的文件名:");
		get_str(filename, 50);
		if(!strncmp(filename, "..", 3))
		{
			pf("..不是普通文件，无法下载，请重新输入！");
			continue;		
		}
		break;
	}
	write(sockfd, filename, strlen(filename) + 1);

	char result[20] = {};
	read(sockfd, result, sizeof(result));
	if(strncmp(result, "success", 8) == 0)
	{
		pf("收到服务端发送的数据:%s 文件准备下载\n", result);
	}
	else if(strncmp(result, "error", 8) == 0)
	{
		pf("收到服务端发送的数据:%s 文件不存在\n", result);
		getch();
		return;
	}
	else
	{
		pf("收到服务端发送的数据:%s 数据异常,下载终止\n", result);
		getch();
		return;
	}

	int fd = open(filename, O_CREAT | O_RDWR, 0777);

	do
	{
		usleep(500);
		memset(buf, 0, sizeof(buf));
		r_size = read(sockfd, buf, sizeof(buf));
		pf("[收到字节数:%d ", r_size);
		w_size = write(fd, buf, r_size);
		pf("写入文件字节数:%d ", w_size);
		w_size = write(sockfd, "success", 8);
		pf("发送success给服务端]\n");
	} while (r_size == 1024);
	usleep(1000000);
	pf("    文件:%s 下载完毕\n", filename);
	close(fd);
	getch();
	return;
}

// 服务端文件列表
void s_list(void)
{
	char see[20] = "我想看你";
	write(sockfd, see, strlen(see) + 1);

	char list[1024] = {};
	read(sockfd, list, sizeof(list));
	pf("服务端目录列表: %s\n", list);

	pf("输入cd+空格+目录，修改服务器工作目录，否则返回上一级\n");
	char cmd[50] = {};
	get_str(cmd, 50);
	if (strstr(cmd, "cd ") == NULL)
	{
		pf("非cd指令，按任意键返回主界面\n");
		snprintf(cmd, 20, "...");
		write(sockfd, cmd, strlen(cmd));
		getch();
	}
	else
	{
		char *dir = malloc(20);
		dir = strrchr(cmd, ' ');
		dir += 1;
		write(sockfd, dir, strlen(dir) + 1);

		read(sockfd, list, sizeof(list));
		pf("服务端目录列表: %s\n", list);
		getch();
	}
	return;
}

// 客户端文件列表
void c_list(void)
{
	pf("当前目录列表:");
	DIR *dir;
	dir = opendir(".");
	struct dirent *dirent;
	while ((dirent = readdir(dir)) != NULL)
	{
		pf(" %s ", dirent->d_name);
	}
	pf("\n");
	closedir(dir);
	getch();
	return;
}

// 退出程序
void quit(void)
{
	char buf[100] = {};
	pf("告知服务端，我要走了");
	char quit[20] = "我要走了";
	write(sockfd, quit, strlen(quit) + 1);

	usleep(10000);

	pf("程序退出\n");
	return;
}
