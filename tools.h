#ifndef TOOL_H
#define TOOL_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>

//　修改终端的控制方式，1取消回显、确认　２获取数据　3还原
int getch(void);

void clear_stdin(void);

char* get_str(char* str,size_t len);

char get_sex(void);

char get_cmd(char start,char end);

char* get_pw(char* passwd,bool is_show,size_t size);

#endif//TOOL_h
