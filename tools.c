#include "tools.h"
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

//　修改终端的控制方式，1取消回显、确认　２获取数据　3还原
int getch(void)
{
    // 记录终端的配置信息
    struct termios old;
    // 获取终端的配置信息
    tcgetattr(STDIN_FILENO,&old);
    // 设置新的终端配置   
    struct termios new1 = old;
    // 取消确认、回显
    new1.c_lflag &= ~(ICANON|ECHO);
    // 设置终端配置信息
    tcsetattr(STDIN_FILENO,TCSANOW,&new1);

    // 在新模式下获取数据   
    int key_val = 0; 
    do{
    	key_val += getchar();
    }while(stdin->_IO_read_end - stdin->_IO_read_ptr);

    // 还原配置信息
    tcsetattr(STDIN_FILENO,TCSANOW,&old); 
    return key_val; 
}

void clear_stdin(void)
{
	stdin->_IO_read_ptr = stdin->_IO_read_end;//清理输入缓冲区
}

char* get_str(char* str, size_t len)
{
	if(NULL == str)
	{
		puts("空指针异常！");
		return NULL;
	}

	char *in = fgets(str, len, stdin);
	if(NULL == in)
		return str;
	
	size_t cnt = strlen(str);
	if('\n' == str[cnt-1])
	{
		str[cnt-1] = '\0';
	}
	
	clear_stdin();

	return str;
}

char get_sex(void)
{
	printf("（m男，w女）：");
	while(true)
	{
		char sex = getch();
		if('w' == sex || 'm' == sex)
		{
			printf("%s\n",'w'==sex?"女":"男");
			return sex;
		}
	}
}

char get_cmd(char start, char end)
{
	clear_stdin();

	printf("请输入指令:");
	while(true)
	{
		char val = getch();
		if(val >= start && val <= end)
		{
			printf("%c\n",val);
			return val;
		}
	}
}

char* get_pw(char* passwd, bool is_show, size_t size)
{
	if(NULL == passwd) return NULL;

	int count = 0;
	do{
		char val = getch();
		if(127 == val)
		{
			if(count > 0)
			{
				if(is_show)printf("\b \b");
				count--;
			}
			continue;
		}
		else if(10 == val)
		{
			break;
		}
		passwd[count++] = val;
		if(is_show) printf("*");
	}while(count < size -1);

	passwd[count] = '\0';
	return passwd;
}

// 获取时间，传入type来获取各种时间
char* get_time(int type)
{
	time_t timep;
	static struct tm *tm_p = NULL;
	time(&timep);
	tm_p = localtime(&timep);
	
	static char now_time[20] = {};

	if(0 == type)
	{
		snprintf(now_time, 1, " ");
	}
	else if(1 == type)
	{
		snprintf(now_time, 20, "%04d-%02d-%02d", (1900 + tm_p->tm_year), (1 + tm_p->tm_mon), tm_p->tm_mday);
	}
	else if(2 == type)
	{
		snprintf(now_time, 20, "%04d-%02d-%02d %02d:%02d:%02d",(1900 + tm_p->tm_year), (1 + tm_p->tm_mon), tm_p->tm_mday, 
			tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec);
	}
	else if(3 == type)
	{
		snprintf(now_time, 30, "%04d-%02d-%02d_%02d:%02d:%02d", (1900 + tm_p->tm_year), (1 + tm_p->tm_mon), 
			tm_p->tm_mday, tm_p->tm_hour, tm_p->tm_min, tm_p->tm_sec);
	}
	else
	{
	}
	
	return now_time;
}