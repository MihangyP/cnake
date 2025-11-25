#include "cnake.h"

void	draw_line(t_game *game, t_vector2 p1, t_vector2 p2, t_color color)
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
			color_pixel(*game->img, p1.x, p1.y, rgba_to_int(color));
		p1.x += inc_x;
		p1.y += inc_y;
		++i;
	}
}

void	draw_rectangle_outline(t_game *game, t_vector2 start, t_vector2 size, t_color color)
{
	t_vector2	top_left = start;
	t_vector2	top_right = (t_vector2){start.x + size.x, start.y};
	t_vector2	bottom_left = (t_vector2){start.x, start.y + size.y};
	t_vector2	bottom_right = (t_vector2){start.x + size.x, start.y + size.y};

	draw_line(game, top_left, top_right, color);
	draw_line(game, bottom_left, bottom_right, color);
	draw_line(game, top_left, bottom_left, color);
	draw_line(game, top_right, bottom_right, color);
}

void	draw_rectangle(t_game *game, t_vector2 start, t_vector2 size, t_color color)
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
					color_pixel(*game->img, start.x, start.y, rgba_to_int(color));
			}
		}
	}
}

void	draw_circle(t_game *game, t_vector2 center, int radius, t_color color)
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
						color_pixel(*game->img, i, j, rgba_to_int(color));
				}
			}
		}
	}
}

// TODO: update this to draw_triangle
void	draw_pause_icon(t_game *game, t_vector2 p1, t_vector2 p2, t_vector2 p3, t_color color)
{
	draw_line(game, p1, p2, color);	
	draw_line(game, p1, p3, color);	
	draw_line(game, p2, p3, color);	
	
	for (int y = p1.y; y < p2.y; ++y)
		draw_line(game, (t_vector2){p1.x, y}, p3, color);	
}
