/*
	hny.c
	Copyright (c) 2018, Valentin Debon

	This file is part of the Honey package manager
	subject the BSD 3-Clause License, see LICENSE.txt
*/
#include "internal.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/param.h> /* MAXPATHLEN */

#include <stdio.h>

hny_t
hny_create(const char *prefix) {
	hny_t hny = malloc(sizeof(*hny));

	if(hny == NULL) {
		goto err_hny_create_1;
	}

	hny->dirp = opendir(prefix);
	if(hny->dirp == NULL) {
		goto err_hny_create_2;
	}

	hny->path = malloc(MAXPATHLEN);
	if(hny->path == NULL
		|| fcntl(dirfd(hny->dirp),
			F_GETPATH, hny->path) == -1) {
		goto err_hny_create_3;
	}

	if(pthread_mutex_init(&hny->mutex, NULL) != 0) {
		goto err_hny_create_4;
	}

	return hny;
err_hny_create_4:
	pthread_mutex_destroy(&hny->mutex);
err_hny_create_3:
	free(hny->path);
err_hny_create_2:
	closedir(hny->dirp);
err_hny_create_1:
	free(hny);
	return NULL;
}

void
hny_destroy(hny_t hny) {

	hny_lock(hny);

	free(hny->path);

	hny_unlock(hny);

	closedir(hny->dirp);

	while(pthread_mutex_destroy(&hny->mutex) == EBUSY) {
		pthread_mutex_lock(&hny->mutex);
		pthread_mutex_unlock(&hny->mutex);
	}

	free(hny);
}
