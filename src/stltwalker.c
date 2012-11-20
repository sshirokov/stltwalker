#include <stdio.h>
#include <stdlib.h>

#include "dbg.h"

const char *Version[] = {"0", "0", "0"};

void usage(int argc, char **argv, char *err) {
		FILE *stream = stderr;
		fprintf(stream, "%s [-flags] [inputs]\n", argv[0]);
		fprintf(stream, "\tv%s.%s.%s\n", Version[0], Version[1], Version[2]);

		fprintf(stream, "\n");
		if(err) {
				fprintf(stream, "ERROR: %s\n", err);
		}
		exit(err ? 1 : 0);
}

int main(int argc, char *argv[]) {
		if(argc < 2) usage(argc, argv, "At least a filename is required.");

		log_info("==> TODO: Something.");

		return 0;
}
