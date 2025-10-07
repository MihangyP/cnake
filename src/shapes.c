#include "cnake.h"

void	draw_line(t_data *data, t_vector2 p1, t_vector2 p2, t_color color)
{
	float		dx;
	float		dy;
	int			step;
	float		inc_x;
	float		inc_y;

	dx = p2.x - p1.x;
	dy = p2.y - p1.y;
	step = dx;
	if (dy > dx) step = dy;
	inc_x = dx / step;
	inc_y = dy / step;

	int i = 0;
	while (i < step) {
		if (p1.x >= 0 && p1.x < W_WIDTH && p1.y >= 0 && p1.y < W_HEIGHT)
			color_pixel(*data->img, p1.x, p1.y, rgba_to_int(color));
		p1.x += inc_x;
		p1.y += inc_y;
		++i;
	}
}

void	draw_rectangle(t_data *data, t_vector2 start, t_vector2 size, t_color color)
{
	t_vector2	end;

	end.x = start.x + size.x;
	end.y = start.y + size.y;
	float tmp = start.x;
	
	for (; start.y <= end.y; ++start.y) {
		if (start.y >= 0 || start.y < W_WIDTH) {
			start.x = tmp;
			for (; start.x <= end.x; ++start.x) {
				if (start.x >= 0 || start.x < W_HEIGHT)
					color_pixel(*data->img, start.x, start.y, rgba_to_int(color));
			}
		}
	}
}
