#include "tools.h"
#include <string.h>
#include <stdbool.h>

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

char get_cmd(char start,char end)
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

char* get_pw(char* passwd,bool is_show,size_t size)
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
