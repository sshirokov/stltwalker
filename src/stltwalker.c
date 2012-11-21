#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbg.h"

#include "stl.h"

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
		int rc = -1;
		stl_object *obj = NULL;
		char *infile = argv[1];

		if(argc < 2) usage(argc, argv, "At least a filename is required.");

		obj = stl_read_file(infile);
		check(obj != NULL, "Failed to read '%s'", infile);

		log_info("Parsed object with %d triangles!", obj->facet_count);

		rc = stl_write_file(obj, "/tmp/out.stl");
		check(rc != -1, "Failed to write file.");

		stl_free(obj);

		return 0;
error:
		return -1;
}
