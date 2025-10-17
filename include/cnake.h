#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#define W_WIDTH 600
#define W_HEIGHT W_WIDTH
#define NUMBER_OF_SQUARE 20
#define SQUARE_SIZE (W_WIDTH / NUMBER_OF_SQUARE)
#define INITIAL_SNAQUE_SIZE 3

// TODO: implement my own hash table

typedef struct s_color t_color;
// Primary Colors, inspired by raylib's colors
#define LIGHTGRAY	(t_color){200, 200, 200, 255}
#define RED			(t_color){230, 41, 55, 255} 
#define DARKGRAY	(t_color){80, 80, 80, 255} 
#define YELLOW		(t_color){253, 249, 0, 255} 
#define GOLD		(t_color){255, 203, 0, 255} 
#define ORANGE		(t_color){255, 161, 0, 255} 
#define PINK		(t_color){255, 109, 194, 255} 
#define MAROON		(t_color){190, 33, 55, 255} 
#define GREEN		(t_color){0, 228, 48, 255} 
#define LIME		(t_color){0, 158, 47, 255} 
#define DARKGREEN	(t_color){0, 117, 44, 255} 
#define SKYBLUE		(t_color){102, 191, 255, 255} 
#define BLUE		(t_color){0, 121, 241, 255} 
#define DARKBLUE	(t_color){0, 82, 172, 255} 
#define PURPLE		(t_color){200, 122, 255, 255} 
#define VIOLET		(t_color){135, 60, 190, 255} 
#define DARKPURPLE	(t_color){112, 31, 126, 255} 
#define BEIGE		(t_color){211, 176, 131, 255} 
#define BROWN		(t_color){127, 106, 79, 255} 
#define DARKBROWN	(t_color){76, 63, 47, 255} 
#define WHITE		(t_color){255, 255, 255, 255} 
#define BLACK		(t_color){0, 0, 0, 255}
#define MAGENTA		(t_color){255, 0, 255, 255}
#define DONTOWHITE	(t_color){242, 242, 242, 255} 

typedef enum {
	INFO,
	DEBUG,
	WARNING,
	ERROR
}	t_log_level;

typedef enum {
	UP,
	DOWN,
	LEFT,
	RIGHT
}	t_direction;

struct s_color {
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
	unsigned char	a;
};

typedef struct s_list {
	void			*content;
	struct s_list	*next;
}	t_list;

typedef struct {
	float x;
	float y;
	float width;
	float height;
}	t_rectangle;

typedef struct {
	float x;
	float y;
}	t_vector2;

typedef struct {
	t_vector2 	position;
	t_direction direction;
	int			nb_collision;
}	t_turn;

typedef struct {
	t_vector2	position;
	t_direction	direction;
	int			color;
}	t_cube;

typedef struct {
	FT_Library	library;
	FT_Face		face;
}	t_font;

typedef struct {
	XImage	*image;
	Pixmap	pix;
	GC		gc;
	int		size_line;
	int		bpp;
	int		width;
	int		height;
	char	*data;
}	t_img;

typedef struct {
	Display	*display;
	Window	window;
	GC		gc;
	Window	root;
	Visual	*visual;
	int		depth;
	Atom	wm_delete_window;
	t_img	*img;
	t_list	*player;
	t_vector2 collectible_position;
	bool	paused;
	bool	dead;
	int		score;
	t_font	font;
	bool	started;
}	t_data;

char	*itoa(int integer);
void	trace_log(t_log_level log_level, const char *format, ...);

// list
t_list	*list_new(void *content);
void	list_add_front(t_list **list, t_list *new_element);
void	list_add_back(t_list **list, t_list *new_element);
t_list	*list_last(t_list *list);
void	list_free(t_list *list);
void	list_del_front(t_list **list);
size_t	list_size(t_list *list);
void	list_print(t_list *list);

// shapes
void	draw_line(t_data *data, t_vector2 p1, t_vector2 p2, t_color color);
void	draw_rectangle(t_data *data, t_vector2 start, t_vector2 size, t_color color);
void	draw_circle(t_data *data, t_vector2 center, int radius, t_color color);
void	draw_pause_icon(t_data *data, t_vector2 p1, t_vector2 p2, t_vector2 p3, t_color color);

// rgba
int		rgba_to_int(t_color color);
t_color	int_to_rgba(int color);
t_color	color_alpha(t_color color, float alpha);

// color pixel
void	color_pixel(t_img img, int x, int y, int color);
// image
void	*new_image(t_data *data, int width, int height);
void	put_buffer_to_window(t_data *data);
