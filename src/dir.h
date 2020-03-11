#ifndef DIR_H
#define DIR_H

#include <ncurses.h>
#include <stddef.h>

struct config
{
	short int hidden; // 1 = show hidden files
	char *envp[3];
	char path[1024];

	struct size
	{
		size_t x;
		size_t y;
	} size;
} config;

struct dir_display
{
	WINDOW *screen;

	struct files
	{
		size_t size;

		int  marked[100];
		char list[100][1024]; // XXX
	} files;

	struct position
	{
		int x[2];
		int y[2];
	} position;
};

typedef struct dir_display display_t;

// Shall check if the path points to an accessible file
int
is_file(char *path);

int
list_files(display_t *dir_display, char *path);

int
preview_list_files(display_t *parent_dir, display_t *child_dir, int cursor);

void
file_open(display_t *dir, int cursor);
#endif /* DIR_H */
