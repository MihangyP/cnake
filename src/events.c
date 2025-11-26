#include "cnake.h"

void	handle_events(t_game *game)
{
	XEvent	event;
	XNextEvent(game->graphic.display, &event);

	if (event.type == KeyPress)
	{
		KeySym keysym = XLookupKeysym(&event.xkey, 0);
		if (keysym == XK_Escape)
			game->state.window_should_close = true;
		else if (game->state.started && !game->state.dead && keysym == XK_space)
			game->state.paused = !game->state.paused;
		else if (!game->state.started && keysym == XK_Return) {
			game->state.started = true;
		} else if (game->state.started && !game->state.dead && !game->state.paused) {
			t_cube *cube = (t_cube*)game->player->content;
			t_turn *turn;
			t_list *new;
			if (keysym == XK_Left && cube->direction != LEFT && cube->direction != RIGHT) {
				cube->direction = LEFT;
				turn = create_new_turn(*cube);
				new = list_new(turn);
				list_add_back(&game->to_turns, new);
			} else if (keysym == XK_Right && cube->direction != RIGHT && cube->direction != LEFT) {
				cube->direction = RIGHT;
				turn = create_new_turn(*cube);
				new = list_new(turn);
				list_add_back(&game->to_turns, new);
			} else if (keysym == XK_Up && cube->direction != UP && cube->direction != DOWN) {
				cube->direction = UP;
				turn = create_new_turn(*cube);
				new = list_new(turn);
				list_add_back(&game->to_turns, new);
			} else if (keysym == XK_Down && cube->direction != DOWN && cube->direction != UP ) {
				cube->direction = DOWN;
				turn = create_new_turn(*cube);
				new = list_new(turn);
				list_add_back(&game->to_turns, new);
			}
		}
	}
	else if (event.type == ClientMessage) {
		if ((Atom)event.xclient.data.l[0] == game->graphic.wm_delete_window)
			game->state.window_should_close = true;
	}
}
