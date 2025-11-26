#define MA_IMPLEMENTATION
#include "cnake.h"
#define NOB_IMPLEMENTATION
#include "../nob.h"

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

void	draw_grid(t_game *game, t_color color1, t_color color2)
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
			draw_rectangle(game, (t_vector2){x, y}, (t_vector2){SQUARE_SIZE, SQUARE_SIZE}, grid_color);
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

void	*new_image(t_game *game, int width, int height)
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
	img->image = XCreateImage(game->graphic.display, game->graphic.visual, game->graphic.depth, ZPixmap,
					0, img->data, width, height, 32, 0);
	img->gc = 0;
	img->size_line = img->image->bytes_per_line;
	img->bpp = img->image->bits_per_pixel;
	img->width = width;
	img->height = height;
	img->pix = XCreatePixmap(game->graphic.display, game->graphic.root, width, height, game->graphic.depth);
	return (img);
}

void	color_pixel(t_img img, int x, int y, int color)
{
	int	offset;

	offset = (y * img.size_line) + (x * (img.bpp / 8));
	*(unsigned int *)(img.data + offset) = color;
}

void	put_buffer_to_window(t_game *game)
{
	XPutImage(game->graphic.display, game->img->pix, game->graphic.gc, game->img->image, 0, 0, 0, 0,
			game->img->width, game->img->height);
	XCopyArea(game->graphic.display, game->img->pix, game->graphic.window, game->graphic.gc,
			0, 0, game->img->width, game->img->height, 0, 0);
}

void	clear_background(t_game *game, t_color color)
{
	for (size_t y = 0; y < W_HEIGHT; ++y) {
		size_t x = 0;
		for (; x < W_WIDTH; ++x)
			color_pixel(*game->img, x, y, rgba_to_int(color));
	}
}

void	draw_text(t_game *game, const char *text, size_t size, int x, int y, t_color color)
{
	FT_Set_Char_Size(game->font.face, 24 * 64, 0, size, 0);
	for (size_t i = 0; text[i]; ++i) {
		FT_Load_Char(game->font.face, text[i], FT_LOAD_RENDER);
		FT_Bitmap *bitmap = &game->font.face->glyph->bitmap;
		for (size_t row = 0; row < bitmap->rows; ++row) {
			for (size_t col = 0; col < bitmap->width; ++col) {
				if (bitmap->buffer[row * bitmap->width + col] > 128) {
					color_pixel(*game->img, x + col, y + row, rgba_to_int(color));
				}
			}
		}
		x += game->font.face->glyph->advance.x >> 6;
	}
}

void	render(t_game *game)
{
	if (!game->state.started) {
		draw_text(game, "CNAKE", 150, W_WIDTH/2 - 90, 200, GOLD);
		draw_text(game, "PRESS ENTER TO START", 69, W_WIDTH/2 - 140, 300, DONTOWHITE);
	} else {
		if (!game->state.dead) {
			draw_grid(game, (t_color){38, 38, 38, 255}, (t_color){24, 24, 24, 255});
			t_list *curr = game->player;
			t_cube *cube  = NULL;
			while (curr) {
				cube = (t_cube *)curr->content;
				if (curr == game->player)
					draw_rectangle(game, cube->position, (t_vector2){SQUARE_SIZE, SQUARE_SIZE}, (t_color){62, 151, 215, 255});
				else
					draw_rectangle(game, cube->position, (t_vector2){SQUARE_SIZE, SQUARE_SIZE}, SKYBLUE);
				curr = curr->next;
			}

			t_vector2	circle_pos = {game->collectible_position.x + SQUARE_SIZE/2, game->collectible_position.y + SQUARE_SIZE/2};
			draw_circle(game, circle_pos, SQUARE_SIZE / 4, GOLD);
			
			/* render score text */
			Nob_String_Builder	score = {0};
            nob_sb_append_cstr(&score, "score: ");
			nob_sb_append_cstr(&score, itoa(game->state.score));
			nob_sb_append_null(&score);
			draw_text(game, score.items, 69, 469, 20, DONTOWHITE);
			/* --------------- */

			if (game->state.paused) {
				draw_pause_icon(game, (t_vector2){W_WIDTH/2 - 20, W_HEIGHT/2 - 20}, (t_vector2){W_WIDTH/2 - 20, W_HEIGHT/2 + 20},
						(t_vector2){W_WIDTH/2 + 20, W_HEIGHT/2}, DONTOWHITE);
			}
		} else {
			// TODO: implement a cool dead screen instead of just text
			draw_text(game, "DEAD", 69, W_WIDTH/2 - 40, W_HEIGHT/2 - 20, DONTOWHITE);
		}
	}
	put_buffer_to_window(game);
}

void	add_cube(t_game *game)
{
	t_list *last = list_last(game->player);
	t_cube *tmp = malloc(sizeof(t_cube));
	if (!tmp) {
		trace_log(ERROR, "Malloc error");
		close_window(game);
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
	list_add_back(&game->player, new);
}

void	init_player(t_game *game)
{
	int	tab[NUMBER_OF_SQUARE];
	for (size_t i = 0, j = 0; i < NUMBER_OF_SQUARE; ++i, j += SQUARE_SIZE)
		tab[i] = j;

	t_cube	*head = malloc(sizeof(t_cube));
	if (!head) {
		trace_log(ERROR, "Malloc error");
		close_window(game);
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
	list_add_back(&game->player, new);

	add_cube(game);
	add_cube(game);
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

bool	init_sound(t_sound *sound)
{
	if (ma_engine_init(NULL, &sound->engine) != MA_SUCCESS) {
		trace_log(ERROR, "Cannot init ma_engine");
		return (false);
	}
	return (true);
}

bool	init_font(t_font *font)
{
	const char *faces[] = {
		"./resources/pixantiqua.ttf"
	};

	if (FT_Init_FreeType(&font->library)) {
		trace_log(ERROR, "Cannot init FreeType");
		return (false);
	}
	for (size_t i = 0; i < ARRAY_LEN(faces); ++i) {
		if (FT_New_Face(font->library, faces[i], 0, &font->face)) {
			trace_log(ERROR, "Cannot create new face");
			return (false);
		}
	}
	return (true);
}

void	clean_sound(t_sound *sound)
{
	ma_engine_uninit(&sound->engine);
}

void	clean_font(t_font *font)
{
	FT_Done_Face(font->face);
	FT_Done_FreeType(font->library);
}

void	update_game(t_game *game)
{
	if (game->state.started && !game->state.dead && !game->state.paused) {
		game->end = (clock() / (float)CLOCKS_PER_SEC) * 1000;
		if (game->end - game->start >= 160)
		{
			t_list *curr = game->player;
			while (curr) {
				t_cube *cube = (t_cube *)curr->content;
				if (game->to_turns) {
					t_list *curr2 = game->to_turns;
					while (curr2) {
						t_turn *turn = (t_turn *)curr2->content;
						if (cube->position.x == turn->position.x && cube->position.y == turn->position.y) {
							++turn->nb_collision;
							cube->direction = turn->direction;
							if (turn->nb_collision >= (int)list_size(game->player)) {
								list_del_front(&game->to_turns);
								break ;
							}
						}
						curr2 = curr2->next;
					}
				}
				curr = curr->next;
			}

			curr = game->player;
			for (; curr; curr = curr->next) {
				t_cube	*cube = (t_cube *)curr->content;
				if (cube->direction == UP) cube->position.y -= SQUARE_SIZE;
				if (cube->direction == DOWN) cube->position.y += SQUARE_SIZE;
				if (cube->direction == LEFT) cube->position.x -= SQUARE_SIZE;
				if (cube->direction == RIGHT) cube->position.x += SQUARE_SIZE;
			}
			game->start = (clock() / (float)CLOCKS_PER_SEC) * 1000;
		}
		t_cube *tmp = (t_cube *)game->player->content;
		if (check_collision_rec_circle(tmp->position, game->collectible_position)) {
			ma_engine_play_sound(&game->sound.engine, "./resources/coin.wav", NULL);
			game->collectible_position = generate_random_collectible_position(game->player, game->tab);
			++game->state.score;
			add_cube(game);
		}

		if (check_collision_recs(game->player))
			game->state.dead = true;

		// To make snake pass through the walls
		t_list *curr = game->player;
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
}

// TODO: remove conditional jumps on rendering
// TODO: fix bugs: infinite loop, target position
// TODO: fix bug: sometimes, all the to_turns are not removed
// TODO: optimze the code
// TODO: fix bug: death checking is not exactly right
int	main(void)
{
	t_game	game = {0};

	// Initialisation
	init_window(&game, W_WIDTH, W_HEIGHT, "Cnake", 0x00181818);
	game.player = NULL;
	init_player(&game);
	if (!init_sound(&game.sound)) {
		close_window(&game);
		return (1);
	}
	if (!init_font(&game.font)) {
		clean_sound(&game.sound);
		return (1);
	}
	for (size_t i = 0, j = 0; i < NUMBER_OF_SQUARE; ++i, j += SQUARE_SIZE)
		game.tab[i] = j;
	game.collectible_position = generate_random_collectible_position(game.player, game.tab);

	// event loop
	game.start = (clock() / (float)CLOCKS_PER_SEC) * 1000;
	while (!game.state.window_should_close) {
		while (XPending(game.graphic.display)) {
			handle_events(&game);
		}
		update_game(&game);
		clear_background(&game, (t_color){24, 24, 24, 255});
		render(&game);
	}

	// Cleaning
	clean_font(&game.font);
	clean_sound(&game.sound);
	close_window(&game);
	return (0);
}
