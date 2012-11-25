#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "klist.h"
#include "dbg.h"

#include "stl.h"
#include "transform.h"

const char *Version[] = {"0", "0", "0"};

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

struct Options {
		enum {Collect} op;
};
struct Options options = {
		.op = Collect
};


/* struct Options options = { */
/* 		.op = Collect */
/* }; */

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

int main(int argc, char *argv[]) {
		if(argc < 2) usage(argc, argv, "Not enough arguments.");
		int rc = -1;

		klist_t(transformer) *in_objects = kl_init(transformer);
		stl_transformer *out = transformer_alloc(NULL);
		char *out_file = NULL;
		stl_transformer *latest = NULL;

		// Transforms before input specifications
		// will chain on the resulting object
		check_mem(latest = out);

		// Read options, load files, chain transforms
		char opt;
		int i = 1;
		for(char *arg = argv[i]; i < argc; arg=argv[++i]) {
				// Transforms
				if(strncmp(arg, "--", 2) == 0) {
						char t_name[256] = {0};
						char t_args[256] = {0};
						rc = sscanf(arg, "--%[^=]=%s", t_name, t_args);
						check(rc >= 1, "Invalid transform spec '%s', format '--name=args'", arg);

						float4x4 transform_mat;
						transform_t transform = transform_find(t_name);
						check(transform != NULL, "Unknown transform %s", t_name);
						check(transform(&transform_mat, t_args) != NULL, "Failed to build transform %s(%s)", t_name, t_args);
						transform_chain(latest, transform_mat);
				}

				// Options
				else if(arg[0] == '-' && strlen(arg) == 2) {
						switch((opt = arg[1])) {
						case 'o':
								out_file = argv[++i];
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

		// Transform each object and count the total
		// facets for the output
		kliter_t(transformer) *tl_iter = NULL;
		uint32_t total_facets = 0;
		for(tl_iter = kl_begin(in_objects); tl_iter != kl_end(in_objects); tl_iter = kl_next(tl_iter)) {
				stl_transformer *transformer = kl_val(tl_iter);
				transform_apply(transformer);
				total_facets += transformer->object->facet_count;
		}
		check(total_facets > 0, "%d facets in resulting model is insufficient.", total_facets);

		// Compile and transform the output object
		check_mem(out->object = stl_alloc(NULL, total_facets));
		switch(options.op) {
		case Collect: {
				int collected = 0;
				for(tl_iter = kl_begin(in_objects); tl_iter != kl_end(in_objects); tl_iter = kl_next(tl_iter)) {
						stl_transformer *transformer = kl_val(tl_iter);
						stl_object *object = transformer->object;
						memcpy(out->object->facets + collected,
							   object->facets,
							   sizeof(stl_facet) * object->facet_count);
						collected += object->facet_count;
				}
				break;
		}
		default:
				sentinel("Unknown operation %d", options.op);
		}
		log_info("Output contains %d facets", total_facets);
		// TODO: Apply transformations out `out'

		// Perform the "result" operation
		if(out_file != NULL) {
				log_info("Writing result object to: '%s'", out_file);
				rc = stl_write_file(out->object, out_file);
				check(rc == 0, "Failed to write output to %s" , out_file);
		}

		kl_destroy(transformer, in_objects);
		return 0;
error:
		kl_destroy(transformer, in_objects);
		return -1;
}
