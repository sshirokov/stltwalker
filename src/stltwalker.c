#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "klist.h"
#include "dbg.h"

#include "stl.h"
#include "transform.h"

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

		// Any objects we load from the command line get
		// pushed here as transformers
		klist_t(transformer) *in_objects = kl_init(transformer);

		// The objects will be accumilated in this transformer
		// and if specified, the output file will be stored in
		// `out_file`
		stl_transformer *out = transformer_alloc(stl_alloc(NULL, 0));
		char *out_file = NULL;

		// Transformations are applied on the latest
		// mentioned transformer wrapped object.
		// At the beginning, this is the output object
		stl_transformer *latest = out;

		// Did we fail already!?
		check_mem(out);
		check_mem(out->object);

		// Read options, load files, chain transforms
		char opt;
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
						latest = transformer_alloc(stl_read_file(arg));
						check(latest != NULL, "Failed to create transformer");
						check(latest->object != NULL, "Failed to load object from '%s'", arg);
						*kl_pushp(transformer, in_objects) = latest;
						log_info("Loaded: %s", arg);
				}
		}

		// TODO: Apply transofmrs on each object in `in_objects'
		// TODO: Accumilate all the objects in `out'
		// TODO: Apply transformations out `out'
		// TODO: Serialize `out' to `out_file' if `out_file' != NULL

		kl_destroy(transformer, in_objects);
		return 0;
error:
		kl_destroy(transformer, in_objects);
		return -1;
}
