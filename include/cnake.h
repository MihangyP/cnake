#ifndef CNAKE_H
#define CNAKE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#define W_WIDTH 800
#define W_HEIGHT 600

// TODO: implement my own hash table

typedef enum
{
	INFO,
	DEBUG,
	WARNING,
	ERROR
}	t_log_level;

typedef enum
{
	UP,
	DOWN,
	LEFT,
	RIGHT
}	t_direction;

typedef struct
{
	unsigned char	r;
	unsigned char	g;
	unsigned char	b;
	unsigned char	a;
}	t_color;

typedef struct s_list
{
	void			*content;
	struct s_list	*next;
}	t_list;

typedef struct
{
	float x;
	float y;
}	t_vector2;

typedef struct
{
	t_vector2	position;
	t_direction	direction;
	int			color;
}	t_cube;

typedef struct
{
	XImage	*image;
	Pixmap	pix;
	GC		gc;
	int		size_line;
	int		bpp;
	int		width;
	int		height;
	int		type;
	int		format;
	char	*data;
}	t_img;

typedef struct
{
	Display	*display;
	Window	window;
	GC		gc;
	Window	root;
	Visual	*visual;
	int		depth;
	Atom	wm_delete_window;
}	t_data;

char	*itoa(int integer);
void	trace_log(t_log_level log_level, const char *format, ...);

// list
t_list	*list_new(void *content);
void	list_add_front(t_list **list, t_list *new_element);
void	list_add_back(t_list **list, t_list *new_element);
t_list	*list_last(t_list *list);
void	list_free(t_list *list);
void	list_print(t_list *list);

// shapes
void	draw_line(t_data *data, t_vector2 p1, t_vector2 p2);
void	draw_rectangle(t_data *data, t_vector2 start, t_vector2 size);

#endif
