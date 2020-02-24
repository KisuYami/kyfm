#include <sys/stat.h>
#include <ncurses.h>
#include <dirent.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>

#include "dir.h"

static int
compare(const void *p1, const void *p2)
{
	return strcmp(*(char *const *)p1, *(char *const *)p2);
}

int
is_file(char *path)
{
	struct stat path_to_file;
	stat(path, &path_to_file);
	return S_ISDIR(path_to_file.st_mode);
}

void
list_files(struct dir_display *dir_display, char *path)
{
	memset(dir_display->files.marked, 0, sizeof(int) * 100); // XXX
	dir_display->files.size = 0;

	DIR *d = NULL;
	char *tmp_list[100];
	size_t i = 0;


	if(path == NULL)
	{
		getcwd(config.path, PATH_MAX);
		d = opendir(config.path);
	}

	else
	{
		if(!is_file(path)) return;
		d = opendir(path);
	}

	for(struct dirent *dir = readdir(d); dir != NULL &&
        i < (sizeof(tmp_list) / sizeof(char *)); dir = readdir(d))
	{
		// Don't Show hidden files
		if(config.hidden || *dir->d_name != '.')
			tmp_list[i++] = dir->d_name;
	}

	qsort(&tmp_list[0], i, sizeof(char *), compare);

	for(dir_display->files.size = 0;
	    dir_display->files.size < i;
	    ++dir_display->files.size)
	{
		strcpy(dir_display->files.list[dir_display->files.size],
		       tmp_list[dir_display->files.size]);
	}

	closedir(d);
}

void
preview_list_files(struct dir_display *parent_dir,
		   struct dir_display *child_dir, int cursor)
{
	char tmp[1024];
	strcpy(tmp, config.path);
	strcat(tmp, "/");
	strcat(tmp, parent_dir->files.list[cursor]);
	list_files(child_dir, tmp);
}

static const char *
file_extension(const char *filename)
{
	const char *dot = strrchr(filename, '.');

	if(!dot || dot == filename)
		return "";

	return dot + 1;
}

void
file_open(struct dir_display *dir, int cursor)
{
	// Get the extension
	const char *extension = file_extension(dir->files.list[cursor]);

	int file_type = -1;

	if((strncmp(extension, "jpg", 3) == 0) ||
	   (strncmp(extension, "png", 3) == 0) ||
	   (strncmp(extension, "jpeg", 4) == 0))
		file_type = 0;

	else if((strncmp(extension, "mkv", 3) == 0) ||
		(strncmp(extension, "mp4", 3) == 0))
		file_type = 1;

	else if(strncmp(extension, "pdf", 3) == 0)
		file_type = 2;

	else if(file_type == -1)
		return; // Non listed file type found

	if(!config.envp[file_type])
	{
		fprintf(stderr, "YAFM: enviroment variable missing");
		return;
	}

	pid_t child = fork();

	if(child == 0)
	{
		int fd = open("/dev/null", O_WRONLY);
		// No output or input
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDIN_FILENO);
		dup2(fd, STDERR_FILENO);
		close(fd);

		execlp(config.envp[file_type],
		       config.envp[file_type],
		       dir->files.list[cursor], (char *)NULL);
		exit(0);
	}
}
