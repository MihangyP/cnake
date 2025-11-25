#include "cnake.h"

void	make_window_not_resizable(t_game *game, size_t width, size_t height)
{
	XSizeHints	hints;

	XGetWMNormalHints(game->graphic.display, game->graphic.window, &hints, NULL);
	hints.width = width;
	hints.height = height;
	hints.min_width = width;
	hints.min_height = height;
	hints.max_width = width;
	hints.max_height = height;
	hints.flags = PPosition | PSize | PMinSize | PMaxSize;
	XSetWMNormalHints(game->graphic.display, game->graphic.window, &hints);
}

void	init_window(t_game *game, size_t width, size_t height, const char *title, int background_color)
{
	game->graphic.display = XOpenDisplay(NULL);
	if (!game->graphic.display) {
		trace_log(ERROR, "Cannot to connect to X11 Server");
		exit(EXIT_FAILURE);
	}
	game->graphic.root = DefaultRootWindow(game->graphic.display);
	int screen = DefaultScreen(game->graphic.display);
	game->graphic.visual = DefaultVisual(game->graphic.display, screen);
	game->graphic.depth = DefaultDepth(game->graphic.display, screen); 
	game->graphic.window = XCreateSimpleWindow(game->graphic.display, game->graphic.root,
					0, 0,
					width, height,
					0,
					0, background_color);
	game->graphic.gc = XCreateGC(game->graphic.display, game->graphic.window, 0, NULL);

	game->graphic.wm_delete_window = XInternAtom(game->graphic.display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(game->graphic.display, game->graphic.window, &game->graphic.wm_delete_window, 1);
	XStoreName(game->graphic.display, game->graphic.window, title);
	make_window_not_resizable(game, W_WIDTH, W_HEIGHT);
	XSelectInput(game->graphic.display, game->graphic.window, KeyPressMask);
	XMapWindow(game->graphic.display, game->graphic.window);
	game->img = new_image(game, W_WIDTH, W_HEIGHT);

	game->to_turns = NULL;

	// States
	game->state.paused = false;
	game->state.dead = false;
	game->state.score = 0;
	game->state.started = false;
	game->state.window_should_close = false;
	//

	trace_log(INFO, "Window initialised succesfully");
	trace_log(INFO, "   > Window size: %d x %d", W_WIDTH, W_HEIGHT);
}

void	close_window(t_game *game)
{
	t_list *curr = game->player;
	while (curr) {
		t_list *tmp = curr;
		curr = curr->next;
		free(tmp->content);
		free(tmp);
	}
	XDestroyImage(game->img->image);
	free(game->img);
	XFreeGC(game->graphic.display, game->graphic.gc);
	XDestroyWindow(game->graphic.display, game->graphic.window);
	XCloseDisplay(game->graphic.display);
	printf("DISPLAY: %p\n", game->graphic.display);
	trace_log(INFO, "Window closed succesfully");
}
