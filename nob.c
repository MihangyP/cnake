#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX
#define NOB_EXPERIMENTAL_DELETE_OLD
#include "nob.h"

#define SRC_FOLDER "src/"
#define BUILD_FOLDER "build/"

Cmd cmd = {0};

void	add_cflags(void)
{
	cmd_append(&cmd, "-Wall", "-Wextra");
	cmd_append(&cmd, "-I./include");
	cmd_append(&cmd, "-I./freetype/include/freetype2");
	cmd_append(&cmd, "-g");
}

void	add_libs(void)
{
	cmd_append(&cmd, "-lX11");
	cmd_append(&cmd, "-lm");
	cmd_append(&cmd, "-Lfreetype/lib");
	cmd_append(&cmd, "-l:libfreetype.a");
	cmd_append(&cmd, "-lz");
}

int	main(int ac, char **av)
{
	NOB_GO_REBUILD_URSELF(ac, av);

	const char *src_files[] = {
		SRC_FOLDER"cnake.c",
		SRC_FOLDER"shapes.c",
		SRC_FOLDER"trace_log.c",
		SRC_FOLDER"list.c",
		SRC_FOLDER"itoa.c",
		SRC_FOLDER"rgba.c",
		SRC_FOLDER"graphics.c"
	};
	size_t	src_files_size = sizeof(src_files) / sizeof(src_files[0]);

	if (!mkdir_if_not_exists(BUILD_FOLDER))
		return 1;
	cmd_append(&cmd, "cc");
	add_cflags();
	for (size_t i = 0; i < src_files_size; ++i)
		cmd_append(&cmd, src_files[i]);
	cmd_append(&cmd, "-o", BUILD_FOLDER"cnake");
	add_libs();
	if (!cmd_run(&cmd))
		return 1;
	return (0);
}
