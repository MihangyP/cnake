#include "cnake.h"

t_color	int_to_rgba(int color)
{
	t_color	result;
	
	result.a = (color >> 24) & 0xFF;
	result.r = (color >> 16) & 0xFF;
	result.g = (color >> 8) & 0xFF;
	result.b = color & 0xFF;
	return (result);
}

int	rgba_to_int(t_color color)
{
	int	result;
	result = (color.a << 24) | (color.r << 16) | (color.g << 8) | color.b;
	return (result);
}
