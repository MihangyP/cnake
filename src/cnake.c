#include "cnake.h"

void	make_window_not_resizable(t_data *data, size_t width, size_t height)
{
	XSizeHints	hints;

	XGetWMNormalHints(data->display, data->window, &hints, NULL);
	hints.width = width;
	hints.height = height;
	hints.min_width = width;
	hints.min_height = height;
	hints.max_width = width;
	hints.max_height = height;
	hints.flags = PPosition | PSize | PMinSize | PMaxSize;
	XSetWMNormalHints(data->display, data->window, &hints);
}

void	init_window(t_data *data, size_t width, size_t height, const char *title,
					int background_color)
{
	data->display = XOpenDisplay(NULL);
	if (!data->display) {
		trace_log(ERROR, "Cannot to connect to X11 Server");
		exit(EXIT_FAILURE);
	}
	data->root = DefaultRootWindow(data->display);
	int screen = DefaultScreen(data->display);
	data->visual = DefaultVisual(data->display, screen);
	data->depth = DefaultDepth(data->display, screen); 
	data->window = XCreateSimpleWindow(data->display, data->root,
					0, 0,
					width, height,
					0,
					0, background_color);
	data->gc = XCreateGC(data->display, data->window, 0, NULL);

	data->wm_delete_window = XInternAtom(data->display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(data->display, data->window, &data->wm_delete_window, 1);
	XStoreName(data->display, data->window, title);
	make_window_not_resizable(data, W_WIDTH, W_HEIGHT);
	XSelectInput(data->display, data->window, KeyPressMask);
	XMapWindow(data->display, data->window);
	data->img = new_image(data, W_WIDTH, W_HEIGHT);
	XFlush(data->display);
}

void	close_window(t_data *data)
{
	// destroy image
	XDestroyImage(data->img->image);
	free(data->img);
	//

	XFreeGC(data->display, data->gc);
	XDestroyWindow(data->display, data->window);
	XCloseDisplay(data->display);
}

void	draw_grid(t_data *data, t_color color)
{
	size_t	grid_size = 20;
	size_t	x_size = W_WIDTH / grid_size;
	size_t	y_size = W_HEIGHT / grid_size;
	size_t	x = x_size;
	size_t 	y = y_size;

	while (x <= W_WIDTH) {
		draw_line(data, (t_vector2){x, 0}, (t_vector2){x, W_HEIGHT}, color);
		x += x_size;
	}
	while (y <= W_HEIGHT) {
		draw_line(data, (t_vector2){0, y}, (t_vector2){W_WIDTH, y}, color);
		y += y_size;
	}
	put_buffer_to_window(data);
}

int	get_random_number(int min, int max)
{
	int	result = 0;

	if (min > max) {
		int	tmp = min;
		min = max;
		max = tmp;
	}
	result = (rand() % (abs(max - min) + 1) + min);
	return (result);
}

void	*new_image(t_data *data, int width, int height)
{
	t_img	*img;

	img = malloc(sizeof(t_img));
	if (!img) return (NULL);
	// TODO: try to understand why (+ 32)
	int size = sizeof(int) * (width + 32) * height;
	img->data = malloc(size);
	if (!img->data) {
		free(img);
		return (NULL);
	}
	bzero(img->data, size);
	img->image = XCreateImage(data->display, data->visual, data->depth, ZPixmap,
					0, img->data, width, height, 32, 0);
	img->gc = 0;
	img->size_line = img->image->bytes_per_line;
	img->bpp = img->image->bits_per_pixel;
	img->width = width;
	img->height = height;
	img->pix = XCreatePixmap(data->display, data->root, width, height, data->depth);
	return (img);
}

void	color_pixel(t_img img, int x, int y, int color)
{
	int	offset;

	offset = (y * img.size_line) + (x * (img.bpp / 8));
	*(unsigned int *)(img.data + offset) = color;
}

void	put_buffer_to_window(t_data *data)
{
	XPutImage(data->display, data->img->pix, data->gc, data->img->image, 0, 0, 0, 0,
			data->img->width, data->img->height);
	XCopyArea(data->display, data->img->pix, data->window, data->gc,
			0, 0, data->img->width, data->img->height, 0, 0);
}

void	clear_background(t_data *data, t_color color)
{
	for (size_t y = 0; y < W_HEIGHT; ++y) {
		size_t x = 0;
		for (; x < W_WIDTH; ++x)
			color_pixel(*data->img, x, y, rgba_to_int(color));
	}
	put_buffer_to_window(data);
}

// TODO: Fix blinking bug
int	main(void)
{
	t_data	data;
	bool	window_should_close = false;

	init_window(&data, W_WIDTH, W_HEIGHT, "Cnake", 0x00181818);

	XEvent event;
	while (!window_should_close) {
		while (XPending(data.display)) {
			XNextEvent(data.display, &event);
			if (event.type == KeyPress) {
				KeySym keysym = XLookupKeysym(&event.xkey, 0);
				if (keysym == XK_Escape) {
					window_should_close = true;
				}
			} else if (event.type == ClientMessage) {
				if ((Atom)event.xclient.data.l[0] == data.wm_delete_window) {
					window_should_close = true;
				}
			}
		}
		clear_background(&data, (t_color){24, 24, 24, 255});
		draw_grid(&data, DONTOWHITE);
	}
	close_window(&data);
	return (0);
}
