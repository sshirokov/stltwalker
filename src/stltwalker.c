#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "klist.h"
#include "dbg.h"

#include "stl.h"

const char *Version[] = {"0", "0", "0"};

void usage(int argc, char **argv, char *err, ...) {
		va_list va;
		FILE *stream = stderr;
		fprintf(stream, "%s [-options] {file0 [--operations]..}..{fileN [--operations]..}\n", argv[0]);
		fprintf(stream, "\tv%s.%s.%s\n", Version[0], Version[1], Version[2]);

		fprintf(stream, "\n");
		if(err) {
				char *e_fmt = calloc(strlen("[PADDING]error: ") + strlen(err), sizeof(char));
				check_mem(e_fmt);
				sprintf(e_fmt, "ERROR: %s\n", err);

				va_start(va, err);
				vfprintf(stream, e_fmt, va);
				free(e_fmt);
				va_end(va);
		}
		exit(err ? 1 : 0);
error:
		exit(-1);
}

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

int main(int argc, char *argv[]) {
		if(argc < 2) usage(argc, argv, "Not enough arguments.");

		klist_t(stl_object) *in_objects = kl_init(stl_object);
		stl_object *latest = NULL;
		char *out_file = NULL;

		char opt;
		int rc = -1;
		int i = 1;
		for(char *arg = argv[i]; i < argc; arg=argv[++i]) {
				// Transforms
				if(strncmp(arg, "--", 2) == 0) {
						log_info("Transform: %s", arg);
				}

				// Options
				else if(arg[0] == '-' && strlen(arg) == 2) {
						switch((opt = arg[1])) {
						case 'o':
								out_file = argv[++i];
								log_info("Output to: '%s'", out_file);
								break;
						default:
								usage(argc, argv, "Unknown option %c", opt);
						}
				}

				// Input loading
				else {
						latest = stl_read_file(arg);
						check(latest != NULL, "Failed to load input from '%s'", arg);
						*kl_pushp(stl_object, in_objects) = latest;
						log_info("Loaded: %s", arg);
				}
		}

		kl_destroy(stl_object, in_objects);
		return 0;
error:
		kl_destroy(stl_object, in_objects);
		return -1;
}
