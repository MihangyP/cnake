#include "cnake.h"

bool	check_collision_rec_circle(t_vector2 rec, t_vector2 center)
{
	return (rec.x == center.x && rec.y == center.y);
}

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
	data->paused = false;
	trace_log(INFO, "Window initialised succesfully");
	trace_log(INFO, "   > Window size: %d x %d", W_WIDTH, W_HEIGHT);
}

void	close_window(t_data *data)
{
	t_list *curr = data->player;
	while (curr) {
		t_list *tmp = curr;
		curr = curr->next;
		free(tmp->content);
		free(tmp);
	}
	XDestroyImage(data->img->image);
	free(data->img);
	XFreeGC(data->display, data->gc);
	XDestroyWindow(data->display, data->window);
	XCloseDisplay(data->display);
	trace_log(INFO, "Window closed succesfully");
}

void	draw_grid(t_data *data, t_color color)
{
	size_t	x = SQUARE_SIZE;
	size_t 	y = SQUARE_SIZE;

	while (x <= W_WIDTH) {
		draw_line(data, (t_vector2){x, 0}, (t_vector2){x, W_HEIGHT}, color);
		x += SQUARE_SIZE;
	}
	while (y <= W_HEIGHT) {
		draw_line(data, (t_vector2){0, y}, (t_vector2){W_WIDTH, y}, color);
		y += SQUARE_SIZE;
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
	/** .  XFlush(data->display);  . **/
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
	/** .  XFlush(data->display);  . **/
}

void	clear_background(t_data *data, t_color color)
{
	for (size_t y = 0; y < W_HEIGHT; ++y) {
		size_t x = 0;
		for (; x < W_WIDTH; ++x)
			color_pixel(*data->img, x, y, rgba_to_int(color));
	}
}

void	render(t_data *data)
{
	/** .  draw_grid(data, DONTOWHITE);  . **/
	t_list *curr = data->player;
	t_cube *cube  = NULL;
	while (curr) {
		cube = (t_cube *)curr->content;
		draw_rectangle(data, cube->position, (t_vector2){SQUARE_SIZE, SQUARE_SIZE}, SKYBLUE);
		curr = curr->next;
	}
	t_vector2	circle_pos = {data->collectible_position.x + SQUARE_SIZE/2, data->collectible_position.y + SQUARE_SIZE/2};
	draw_circle(data, circle_pos, SQUARE_SIZE / 3, GOLD);
	if (data->paused) {
		draw_pause_icon(data, (t_vector2){W_WIDTH/2 - 20, W_HEIGHT/2 - 20}, (t_vector2){W_WIDTH/2 - 20, W_HEIGHT/2 + 20},
				(t_vector2){W_WIDTH/2 + 20, W_HEIGHT/2}, DONTOWHITE);
	}
	put_buffer_to_window(data);
}

void	init_player(t_data *data)
{
	int	tab[NUMBER_OF_SQUARE];
	for (size_t i = 0, j = 0; i < NUMBER_OF_SQUARE; ++i, j += SQUARE_SIZE)
		tab[i] = j;

	t_cube	*head = malloc(sizeof(t_cube));
	if (!head) {
		trace_log(ERROR, "Malloc error");
		// Return an error
	}
	srand(time(NULL));
	head->position.x = tab[get_random_number(0, NUMBER_OF_SQUARE)];
	head->position.y = tab[get_random_number(0, NUMBER_OF_SQUARE)];
	head->direction = get_random_number(UP, RIGHT);
	t_list	*new = list_new(head);
	list_add_back(&data->player, new);
}

void	add_cube(t_data *data)
{
	t_list *last = list_last(data->player);
	t_cube *tmp = malloc(sizeof(t_cube));
	if (!tmp) {
		trace_log(ERROR, "Malloc error");
		// Return error
	}

	t_cube *last_content = (t_cube*)last->content;
	if (last_content->direction == UP) {
		tmp->position.x = last_content->position.x;
		tmp->position.y = last_content->position.y + SQUARE_SIZE;
		tmp->direction = UP;
	} else if (last_content->direction == DOWN) {
		tmp->position.x = last_content->position.x;
		tmp->position.y = last_content->position.y - SQUARE_SIZE;
		tmp->direction = DOWN;
	} else if (last_content->direction == LEFT) {
		tmp->position.x = last_content->position.x + SQUARE_SIZE ;
		tmp->position.y = last_content->position.y;
		tmp->direction = LEFT;
	} else if (last_content->direction == RIGHT) {
		tmp->position.x = last_content->position.x - SQUARE_SIZE ;
		tmp->position.y = last_content->position.y;
		tmp->direction = RIGHT;
	}
	t_list *new = list_new(tmp);
	list_add_back(&data->player, new);
}

t_turn	*create_new_turn(t_cube cube)
{
	t_turn *turn = malloc(sizeof(t_turn));
	if (!turn) return (NULL);
	turn->position.x = cube.position.x;
	turn->position.y = cube.position.y;
	turn->direction = cube.direction;
	turn->nb_collision = 0;
	return (turn);
}

// TODO: remove conditional jumps on rendering
// TODO: fix bugs: infinite loop, target position
// TODO: fix bug: sometimes, the position of the target is not correct
int	main(void)
{
	t_data	data;
	bool	window_should_close = false;

	init_window(&data, W_WIDTH, W_HEIGHT, "Cnake", 0x00181818);

	data.player = NULL;
	init_player(&data);
	add_cube(&data);
	add_cube(&data);
	add_cube(&data);

	t_list *to_turns = NULL;

	// collecible
	int	tab[NUMBER_OF_SQUARE];
	for (size_t i = 0, j = 0; i < NUMBER_OF_SQUARE; ++i, j += SQUARE_SIZE)
		tab[i] = j;
	data.collectible_position.x = tab[get_random_number(0, NUMBER_OF_SQUARE)];
	data.collectible_position.y = tab[get_random_number(0, NUMBER_OF_SQUARE)];
	//

	XEvent event;
	double start = (clock() / (float)CLOCKS_PER_SEC) * 1000;
	while (!window_should_close) {
		while (XPending(data.display)) {
			XNextEvent(data.display, &event);
			if (event.type == KeyPress) {
				KeySym keysym = XLookupKeysym(&event.xkey, 0);
				if (keysym == XK_Escape)
					window_should_close = true;
				else if (keysym == XK_space)
					data.paused = !data.paused;
				else if (!data.paused) {
					t_cube *cube = (t_cube*)data.player->content;
					if (keysym == XK_Left && cube->direction != LEFT && cube->direction != RIGHT) {
						cube->direction = LEFT;
						t_turn *turn = create_new_turn(*cube);
						t_list *new = list_new(turn);
						list_add_back(&to_turns, new);
					} else if (keysym == XK_Right && cube->direction != RIGHT && cube->direction != LEFT) {
						cube->direction = RIGHT;
						t_turn *turn = create_new_turn(*cube);
						t_list *new = list_new(turn);
						list_add_back(&to_turns, new);
					} else if (keysym == XK_Up && cube->direction != UP && cube->direction != DOWN) {
						cube->direction = UP;
						t_turn *turn = create_new_turn(*cube);
						t_list *new = list_new(turn);
						list_add_back(&to_turns, new);
					} else if (keysym == XK_Down && cube->direction != DOWN && cube->direction != UP ) {
						cube->direction = DOWN;
						t_turn *turn = create_new_turn(*cube);
						t_list *new = list_new(turn);
						list_add_back(&to_turns, new);
					}
				}
			} else if (event.type == ClientMessage) {
				if ((Atom)event.xclient.data.l[0] == data.wm_delete_window)
					window_should_close = true;
			}
		}
	
		clear_background(&data, (t_color){24, 24, 24, 255});
		// Update data
		if (!data.paused) {
			double end = (clock() / (float)CLOCKS_PER_SEC) * 1000;
			if (end - start >= 160) {
				t_list *curr = data.player;
				while (curr) {
					t_cube *cube = (t_cube *)curr->content;
					if (to_turns) {
						t_list *curr2 = to_turns;
						while (curr2) {
							t_turn *turn = (t_turn *)curr2->content;
							if (cube->position.x == turn->position.x && cube->position.y == turn->position.y) {
								++turn->nb_collision;
								cube->direction = turn->direction;
								if (turn->nb_collision == (int)list_size(data.player))
									list_del_front(&to_turns);
								break ;
							}
							curr2 = curr2->next;
						}
					}
					curr = curr->next;
				}

				curr = data.player;
				for (; curr; curr = curr->next) {
					t_cube	*cube = (t_cube *)curr->content;
					switch (cube->direction) {
						case UP: {
							cube->position.y -= SQUARE_SIZE;
						} break ;
						case DOWN: {
							cube->position.y += SQUARE_SIZE;
						} break ;
						case LEFT: {
							cube->position.x -= SQUARE_SIZE;
						} break ;
						case RIGHT: {
							cube->position.x += SQUARE_SIZE;
						} break ;
					}
				}
				start = (clock() / (float)CLOCKS_PER_SEC) * 1000;
			}
			t_cube *tmp = (t_cube *)data.player->content;
			if (check_collision_rec_circle(tmp->position, data.collectible_position)) {
				data.collectible_position.x = tab[get_random_number(0, NUMBER_OF_SQUARE)];
				data.collectible_position.y = tab[get_random_number(0, NUMBER_OF_SQUARE)];
				add_cube(&data);
			}

			// To make snake pass through the walls
			t_list *curr = data.player;
			while (curr) {
				t_cube *tmp = (t_cube *)curr->content;
				if (tmp->position.x > W_WIDTH) {
					tmp->position.x = 0;
				} else if (tmp->position.x < 0) {
					tmp->position.x = W_WIDTH;
				} else if (tmp->position.y > W_HEIGHT) {
					tmp->position.y = 0;
				} else if (tmp->position.y < 0) {
					tmp->position.y = W_HEIGHT;
				}
				curr = curr->next;
			}
		}
		render(&data);
	}
	close_window(&data);
	return (0);
}
