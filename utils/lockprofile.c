// SPDX-License-Identifier: LGPL-2.1-or-later
// Copyright © 2009-2018 ANSSI. All Rights Reserved.
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void usage (const char* progname, const char* msg) {
	if (msg) fprintf (stderr, "%s\n", msg);
	fprintf (stderr, "Usage: %s <profiledir> <cmd>\n", (progname) ? progname : "lockparent");
}

int main (int argc, char** argv) {
	int fd;
	char* filename;

	if (argc < 3) {
		usage (argv[0], "Wrong number of arguments");
		exit (1);
	}

	if (asprintf (&filename, "%s/%s", argv[1], ".parentlock") < 0) {
		fprintf (stderr, "Not enough memory");
		exit (1);
	}

	fd = open (filename, O_WRONLY);
	if (fd < 0) {
		usage (argv[0], "Impossible to open the parent lock");
		exit (1);
	}
	free (filename);

	if (lockf (fd, F_TLOCK, 0) != 0) {
		close (fd);
		fprintf (stderr, "Parent lock already locked\n");
		exit (2);
	}

	execvp (argv[2], &(argv[2]));
	perror ("execvp");
	exit (3);
}
