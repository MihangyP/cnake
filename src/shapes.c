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
		if (start.y >= 0 && start.y < W_HEIGHT) {
			start.x = tmp;
			for (; start.x <= end.x; ++start.x) {
				if (start.x >= 0 && start.x < W_WIDTH)
					color_pixel(*data->img, start.x, start.y, rgba_to_int(color));
			}
		}
	}
}

void	draw_circle(t_data *data, t_vector2 center, int radius, t_color color)
{
	t_vector2	start = {
		center.x - radius,
		center.y - radius
	};
	int i;
	for (int j = start.y; j < center.y + radius; ++j) {
		i = start.x;
		if (j >= 0 && j < W_HEIGHT) {
			for (; i < center.x + radius; ++i) {
				if (i >= 0 && i < W_WIDTH) {
					int dx = i - center.x;
					int dy = j - center.y;
					if (sqrt(dx * dx + dy * dy) <= (float)radius)
						color_pixel(*data->img, i, j, rgba_to_int(color));
				}
			}
		}
	}
}

static int	min(float a, float b)
{
	return (a <= b ? a : b);
}

static int	max(float a, float b)
{
	return (a >= b ? a : b);
}

int	collision_width_edges(int x, int y, int x_max, t_vector2 p1, t_vector2 p2, t_vector2 p3)
{
	// p1 p2
	float a1 = p2.y - p1.y / p2.x - p1.x;
	float b1 = p1.y - a1 * p1.x;
	// p1 p3
	float a2 = p3.y - p1.y / p3.x - p1.x;
	float b2 = p1.y - a2 * p1.x;
	// p2 p3
	float a3 = p3.y - p2.y / p3.x - p2.x;
	float b3 = p2.y - a3 * p2.x;

	int collision = 0;	
	for (; x < x_max; ++x) {
		if (y == a1 * x + b1 || y == a2 * x + b2 || y == a3 * x + b3) {
			trace_log(DEBUG, "yo");
			++collision;
		}
	}
	return (collision);
}

void	draw_triangle(t_data *data, t_vector2 p1, t_vector2 p2, t_vector2 p3, t_color color)
{
	int	x_min = min(p1.x, min(p2.x, p3.x));
	int	x_max = max(p1.x, max(p2.x, p3.x));
	int	y_min = min(p1.y, min(p2.y, p3.y));
	int	y_max = max(p1.y, max(p2.y, p3.y));

	draw_line(data, p1, p2, color);	
	draw_line(data, p1, p3, color);	
	draw_line(data, p2, p3, color);	

	for (int y = y_min + 1; y < y_max - 1; ++y) {
		int x = x_min + 1;
		for (; x < x_max - 1; ++x) {
			if (collision_width_edges(x, y, x_max, p1, p2, p3))
				color_pixel(*data->img, x, y, rgba_to_int(color));
		}
	}
}
