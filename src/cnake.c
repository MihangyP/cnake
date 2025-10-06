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
	XFlush(data->display);
}

void	close_window(t_data *data)
{
	XFreeGC(data->display, data->gc);
	XDestroyWindow(data->display, data->window);
	XCloseDisplay(data->display);
}

void	draw_grid(t_data *data)
{
	size_t	grid_size = 20;
	size_t	x_size = W_WIDTH / grid_size;
	size_t	y_size = W_HEIGHT / grid_size;
	size_t	x = x_size;
	size_t 	y = y_size;

	XSetForeground(data->display, data->gc, 0xffffffff);
	while (x <= W_WIDTH) {
		draw_line(data, (t_vector2){x, 0}, (t_vector2){x, W_HEIGHT});
		x += x_size;
	}
	while (y <= W_HEIGHT) {
		draw_line(data, (t_vector2){0, y}, (t_vector2){W_WIDTH, y});
		y += y_size;
	}
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
	img->format = ZPixmap;
	img->type = 1; 
	return (img);
}

void	color_pixel(t_img img, int x, int y, int color)
{
	int	offset;

	offset = (y * img.size_line) + (x * (img.bpp / 8));
	*(unsigned int *)(img.data + offset) = color;
}

void	clear_background(t_img *img, int color)
{
	for (size_t y = 0; y < W_HEIGHT; ++y) {
		size_t x = 0;
		for (; x < W_WIDTH; ++x)
			color_pixel(*img, x, y, color);
	}
}

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

int	main(void)
{
	t_data	data;
	bool	window_should_close = false;

	/** .  srand(time(NULL));  . **/

	init_window(&data, W_WIDTH, W_HEIGHT, "Cnake", 0x00181818);

	t_img	*img = new_image(&data, W_WIDTH, W_HEIGHT);

	clear_background(img, rgba_to_int(SKYBLUE));

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
		XPutImage(data.display, img->pix, data.gc, img->image, 0, 0, 0, 0,
				img->width, img->height);
		XCopyArea(data.display, img->pix, data.window, data.gc,
				0, 0, img->width, img->height, 0, 0);
	}
	close_window(&data);
	return (0);
}
