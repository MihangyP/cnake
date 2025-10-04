#ifndef CNAKE_H
#define CNAKE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

typedef enum
{
	INFO,
	DEBUG,
	WARNING,
	ERROR
}	t_log_level;

typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}	t_list;

char	*itoa(int integer);
void	trace_log(t_log_level log_level, const char *format, ...);

#endif
