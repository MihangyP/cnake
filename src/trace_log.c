#include "cnake.h"

void	trace_log(t_log_level log_level, const char *format, ...)
{
	va_list	ap;
	char	*string;
	int		fd = 1;;
	
	if (log_level == WARNING || log_level == ERROR) fd = 2;
	va_start(ap, format);
	while (*format) {
		if (*format != '%')	{
			write(fd, format, 1);
			++format;
		} else if (++format) {
			switch (*format++) {
				case 'c': {
					int c = va_arg(ap, int);
					write(fd, &c, 1);
				} break;
				case 's': {
					string = va_arg(ap, char *);
					write(fd, string, strlen(string));
				} break;
				case 'd': {
					string = itoa(va_arg(ap, int));	
					write(fd, string, strlen(string));
				} break;
			}
		}
	}
	write(fd, "\n", 1);
	va_end(ap);
}
