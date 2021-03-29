#ifndef TOOL_H
#define TOOL_H

#include <stdio.h>

#include "tools.h"
#include <string.h>
#include "getch.h"
#include <stdbool.h>

void clear_stdin(void);

char* get_str(char* str,size_t len);

char get_sex(void);

char get_cmd(char start,char end);

char* get_pw(char* passwd,bool is_show,size_t size);

#endif//TOOL_h
