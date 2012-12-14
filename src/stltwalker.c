#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "klist.h"
#include "dbg.h"

#include "stl.h"
#include "transform.h"
#include "pack.h"

const char *Version[] = {"0", "0", "2"};

struct Options {
		enum {Collect} op;
		stl_transformer out;
		char *out_file;
};
struct Options options = {
		.op = Collect,
		.out_file = NULL
};


void usage(int argc, char **argv, char *err, ...) {
		va_list va;
		FILE *stream = stderr;
		fprintf(stream, "%s [-options] {file0 [--operations]..}..{fileN [--operations]..}\n", argv[0]);
		fprintf(stream, "\tv%s.%s.%s\n", Version[0], Version[1], Version[2]);
		fprintf(stream, "\n");

		fprintf(stream, "Options:\n");
		fprintf(stream, "\t-h\tShow help\n");
		fprintf(stream, "\t-o filename\tOutput the resulting composite object to `filename'\n");
		fprintf(stream, "\n");

		fprintf(stream, "Transforms:\n");
		for(transformer *t = (transformer*)transformers; t->name != NULL; t++) {
				fprintf(stream, "\t--%s=<options>\t%s\n", t->name, t->description);
		}
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
		transformer_init(&options.out, NULL);
		stl_transformer *latest = &options.out;

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
								latest = &options.out;
								options.out_file = argv[++i];
								break;
						case 'h':
								usage(argc, argv, NULL);
						default:
								usage(argc, argv, "Unknown option %c", opt);
						}
				}

				// Input loading
				else {
						latest = transformer_alloc(stl_read_file(arg));
						check(latest != NULL, "Failed to create transformer");
						check(latest->object != NULL, "Failed to load object from '%s'", arg);

						// Center and +Z the object
						object_transform_chain_zero_z(latest);
						object_transform_chain_center_x(latest);
						object_transform_chain_center_y(latest);
						// Apply any potential chained object transforms
						transform_apply(latest);

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
		check_mem(options.out.object = stl_alloc(NULL, total_facets));
		switch(options.op) {
		case Collect: {
				int collected = 0;
				for(tl_iter = kl_begin(in_objects); tl_iter != kl_end(in_objects); tl_iter = kl_next(tl_iter)) {
						stl_transformer *transformer = kl_val(tl_iter);
						stl_object *object = transformer->object;
						memcpy(options.out.object->facets + collected,
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


		float3 bounds[2] = {FLOAT3_INIT, FLOAT3_INIT};
		object_bounds(options.out.object, &bounds[0], &bounds[1]);
		log_info("Object bounds [before]: m(%f, %f, %f) M(%f, %f, %f)", FLOAT3_FORMAT(bounds[0]), FLOAT3_FORMAT(bounds[1]));


		// Apply transformations to the result
		transform_apply(&options.out);

		object_bounds(options.out.object, &bounds[0], &bounds[1]);
		log_info("Object bounds [after]: m(%f, %f, %f) M(%f, %f, %f)", FLOAT3_FORMAT(bounds[0]), FLOAT3_FORMAT(bounds[1]));

		// Perform the "result" operation
		if(options.out_file != NULL) {
				log_info("Writing result object to: '%s'", options.out_file);
				rc = stl_write_file(options.out.object, options.out_file);
				check(rc == 0, "Failed to write output to %s" , options.out_file);
		}

		kl_destroy(transformer, in_objects);
		return 0;
error:
		kl_destroy(transformer, in_objects);
		return -1;
}
