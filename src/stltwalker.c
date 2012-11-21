#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

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

typedef struct s_stl_object {
		char header[80];
		uint32_t facet_count;
} stl_object;

void stl_free(stl_object *obj) {
		log_err("TODO: stl_free() is a stub.");
}

stl_object *stl_alloc(void) {
		stl_object *obj = (stl_object*)malloc(sizeof(stl_object));
		check_mem(obj);
		bzero(obj, sizeof(stl_object));
		return obj;
error:
		exit(-1);
}

stl_object *stl_parse(int fd) {
		int rc = -1;
		stl_object *obj = stl_alloc();

		// Read the header
		check((rc = read(fd, obj->header, sizeof(obj->header))) != -1,
			  "Unable to read STL header.");

		char h1[81] = {0};
		memcpy(h1, obj->header, 80);
		log_info("Header: [%s]", h1);

		return obj;
error:
		if(obj) stl_free(obj);
		return NULL;
}

stl_object *stl_parse_file(char *path) {
		stl_object *obj = NULL;
		int fd = open(path, O_RDONLY);
		check(fd != -1, "Unable to open '%s'", path);

		obj = stl_parse(fd);
		close(fd);

		return obj;
error:
		if(fd != -1) close(fd);
		if(obj) stl_free(obj);
		return NULL;
}

int main(int argc, char *argv[]) {
		stl_object *obj = NULL;
		char *infile = argv[1];

		if(argc < 2) usage(argc, argv, "At least a filename is required.");

		obj = stl_parse_file(infile);
		check(obj != NULL, "Failed to parse '%s'", infile);

		log_info("==> TODO: Something.");

		return 0;
error:
		return -1;
}
