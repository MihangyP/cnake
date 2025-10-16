#include "cnake.h"
#define MA_IMPLEMENTATION
#include "miniaudio.h"

bool	check_collision_rec_circle(t_vector2 rec, t_vector2 center)
{
	return (rec.x == center.x && rec.y == center.y);
}

bool	check_collision_recs(t_list *player)
{
	bool	collision = false;
	t_cube *head = (t_cube *)player->content;
	int	x_head = head->position.x;
	int	y_head = head->position.y;

	if (head->direction == RIGHT) x_head += SQUARE_SIZE;
	if (head->direction == DOWN) y_head += SQUARE_SIZE;

	t_list *curr = player->next;
	while (curr) {
		t_cube *cube = (t_cube *)curr->content;
		if (cube->position.x == x_head && cube->position.y == y_head)
			collision = true;
		curr = curr->next;
	}
	return (collision);
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
	data->dead = false;
	data->score = 0;
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

void	draw_grid(t_data *data, t_color color1, t_color color2)
{
	size_t	x;
	size_t 	y;
	int tmp_y = 0;
	int tmp_x;
	y = 0;
	t_color grid_color;
	while (y < W_HEIGHT) {
		tmp_x = 0;
		x = 0;
		while (x < W_WIDTH) {
			if ((tmp_x + tmp_y) % 2 == 0)
				grid_color = color1;
			else
				grid_color = color2;
			draw_rectangle(data, (t_vector2){x, y}, (t_vector2){SQUARE_SIZE, SQUARE_SIZE}, grid_color);
			x += SQUARE_SIZE;
			++tmp_x;
		}
		y += SQUARE_SIZE;
		++tmp_y;
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

void	draw_text(t_data *data, const char *text, size_t size, int x, int y, t_color color)
{
	FT_Set_Char_Size(data->font.face, 24 * 64, 0, size, 0);
	for (size_t i = 0; text[i]; ++i) {
		FT_Load_Char(data->font.face, text[i], FT_LOAD_RENDER);
		FT_Bitmap *bitmap = &data->font.face->glyph->bitmap;
		for (size_t row = 0; row < bitmap->rows; ++row) {
			for (size_t col = 0; col < bitmap->width; ++col) {
				if (bitmap->buffer[row * bitmap->width + col] > 128) {
					color_pixel(*data->img, x + col, y + row, rgba_to_int(color));
				}
			}
		}
		x += data->font.face->glyph->advance.x >> 6;
	}
}

void	render(t_data *data)
{
	draw_grid(data, (t_color){38, 38, 38, 255}, (t_color){24, 24, 24, 255});
	t_list *curr = data->player;
	t_cube *cube  = NULL;
	while (curr) {
		cube = (t_cube *)curr->content;
		if (curr == data->player)
			draw_rectangle(data, cube->position, (t_vector2){SQUARE_SIZE, SQUARE_SIZE}, (t_color){62, 151, 215, 255});
		else
			draw_rectangle(data, cube->position, (t_vector2){SQUARE_SIZE, SQUARE_SIZE}, SKYBLUE);
		curr = curr->next;
	}
	t_vector2	circle_pos = {data->collectible_position.x + SQUARE_SIZE/2, data->collectible_position.y + SQUARE_SIZE/2};
	draw_circle(data, circle_pos, SQUARE_SIZE / 4, GOLD);
	if (!data->dead && data->paused) {
		draw_pause_icon(data, (t_vector2){W_WIDTH/2 - 20, W_HEIGHT/2 - 20}, (t_vector2){W_WIDTH/2 - 20, W_HEIGHT/2 + 20},
				(t_vector2){W_WIDTH/2 + 20, W_HEIGHT/2}, DONTOWHITE);
		draw_text(data, "Pause", 69, W_WIDTH/2 - 40, W_HEIGHT/2 - 50, DONTOWHITE);
	}
	if (data->dead) {
		draw_rectangle(data, (t_vector2){(W_WIDTH+SQUARE_SIZE)/2, (W_HEIGHT+SQUARE_SIZE)/2}, (t_vector2){SQUARE_SIZE, SQUARE_SIZE}, DONTOWHITE);
	}

	// Score
	char score_text[69];
	bzero(score_text, 69);
	score_text[0] = 's';
	score_text[1] = 'c';
	score_text[2] = 'o';
	score_text[3] = 'r';
	score_text[4] = 'e';
	score_text[5] = ':';
	score_text[6] = ' ';
	strcat(score_text, itoa(data->score));
	draw_text(data, score_text, 69, 469, 20, DONTOWHITE);
}

void	add_cube(t_data *data)
{
	t_list *last = list_last(data->player);
	t_cube *tmp = malloc(sizeof(t_cube));
	if (!tmp) {
		trace_log(ERROR, "Malloc error");
		close_window(data);
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

void	init_player(t_data *data)
{
	int	tab[NUMBER_OF_SQUARE];
	for (size_t i = 0, j = 0; i < NUMBER_OF_SQUARE; ++i, j += SQUARE_SIZE)
		tab[i] = j;

	t_cube	*head = malloc(sizeof(t_cube));
	if (!head) {
		trace_log(ERROR, "Malloc error");
		close_window(data);
	}
	srand(time(NULL));
	head->position.x = tab[get_random_number(0, NUMBER_OF_SQUARE)];
	head->position.y = tab[get_random_number(0, NUMBER_OF_SQUARE)];
	head->direction = get_random_number(UP, RIGHT);
	if (head->direction == LEFT && head->position.x > W_WIDTH - (SQUARE_SIZE * INITIAL_SNAQUE_SIZE))
		head->position.x = W_WIDTH - (SQUARE_SIZE * INITIAL_SNAQUE_SIZE);
	else if (head->direction == RIGHT && head->position.x < SQUARE_SIZE * (INITIAL_SNAQUE_SIZE - 1))
		head->position.x = SQUARE_SIZE * (INITIAL_SNAQUE_SIZE - 1);
	else if (head->direction == UP && head->position.y > W_HEIGHT - (SQUARE_SIZE * 3))
		head->position.y = W_HEIGHT - (SQUARE_SIZE * 3);
	else if (head->direction == DOWN && head->position.y < SQUARE_SIZE * (INITIAL_SNAQUE_SIZE - 1))
		head->position.y = SQUARE_SIZE * (INITIAL_SNAQUE_SIZE - 1);
	t_list	*new = list_new(head);
	list_add_back(&data->player, new);

	add_cube(data);
	add_cube(data);
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

bool	in_player_position(t_list *player, t_vector2 v)
{
	while (player) {
		t_cube *cube = (t_cube *)player->content;
		if (v.x == cube->position.x && v.y == cube->position.y)
			return (true);
		player = player->next;
	}
	return (false);
}

t_vector2	generate_random_collectible_position(t_list *player, int tab[])
{
	t_vector2 result;

	result.x = tab[get_random_number(0, NUMBER_OF_SQUARE - 1)];
	result.y = tab[get_random_number(0, NUMBER_OF_SQUARE - 1)];
	while (in_player_position(player, result)) {
		result.x = tab[get_random_number(0, NUMBER_OF_SQUARE - 1)];
		result.y = tab[get_random_number(0, NUMBER_OF_SQUARE - 1)];
	}
	return (result);
}

// TODO: remove conditional jumps on rendering
// TODO: fix bugs: infinite loop, target position
// TODO: fix bug: sometimes, all the to_turns are not removed
// TODO: optimze the codeA
// TODO: fix bug: death checking is not exactly right
int	main(void)
{
	t_data	data;
	bool	window_should_close = false;
	// For sounds
	ma_engine	engine;
	if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
		trace_log(ERROR, "Cannot init ma_engine");
		return (1);
	}
	//
	// For font
	if (FT_Init_FreeType(&data.font.library)) {
		trace_log(ERROR, "Cannot init FreeType");
		return (1);
	}
	if (FT_New_Face(data.font.library, "./resources/pixantiqua.ttf", 0, &data.font.face)) {
		trace_log(ERROR, "Cannot create new face");
		return (1);
	}
	//
	init_window(&data, W_WIDTH, W_HEIGHT, "Cnake", 0x00181818);
	data.player = NULL;
	init_player(&data);

	t_list *to_turns = NULL;

	// target
	int	tab[NUMBER_OF_SQUARE];
	for (size_t i = 0, j = 0; i < NUMBER_OF_SQUARE; ++i, j += SQUARE_SIZE)
		tab[i] = j;
	data.collectible_position = generate_random_collectible_position(data.player, tab);
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
				else if (keysym == XK_space && !data.dead)
					data.paused = !data.paused;
				else if (!data.dead && !data.paused) {
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
		if (!data.dead && !data.paused) {
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
								if (turn->nb_collision >= (int)list_size(data.player)) {
									list_del_front(&to_turns);
									break ;
								}
							}
							curr2 = curr2->next;
						}
					}
					curr = curr->next;
				}

				curr = data.player;
				for (; curr; curr = curr->next) {
					t_cube	*cube = (t_cube *)curr->content;
					if (cube->direction == UP) cube->position.y -= SQUARE_SIZE;
					if (cube->direction == DOWN) cube->position.y += SQUARE_SIZE;
					if (cube->direction == LEFT) cube->position.x -= SQUARE_SIZE;
					if (cube->direction == RIGHT) cube->position.x += SQUARE_SIZE;
				}
				start = (clock() / (float)CLOCKS_PER_SEC) * 1000;
			}
			t_cube *tmp = (t_cube *)data.player->content;
			if (check_collision_rec_circle(tmp->position, data.collectible_position)) {
				ma_engine_play_sound(&engine, "./resources/coin.wav", NULL);
				data.collectible_position = generate_random_collectible_position(data.player, tab);
				++data.score;
				add_cube(&data);
			}

			if (check_collision_recs(data.player))
				data.dead = true;

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
		{
			/** .  t_list *curr = to_turns;  . **/
			/** .  while (curr) {  . **/
			/** .      t_turn *turn = (t_turn *)curr->content;  . **/
			/** .      draw_rectangle(&data, turn->position, (t_vector2){SQUARE_SIZE, SQUARE_SIZE}, RED);  . **/
			/** .      curr = curr->next;  . **/
			/** .  }  . **/
		}
		put_buffer_to_window(&data);
	}
	close_window(&data);
	ma_engine_uninit(&engine);
	FT_Done_Face(data.font.face);
	FT_Done_FreeType(data.font.library);
	return (0);
}
