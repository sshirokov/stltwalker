#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "klist.h"
#include "dbg.h"

#include "stl.h"
#include "transform.h"
#include "pack.h"

const char *Version[] = {"0", "0", "3"};

struct Options {
		enum {Collect, Pack, Copy} op;

		// .op == Pack options
		float padding;

		float3 max_model_lwh;

		// Disables centering the model before chaining other transforms
		int raw_load;
		int raw_out;

		stl_transformer out;
		int describe_level;
		char *out_file;

		// Verbosity level
		int volume;
};
struct Options options = {
		.op = Collect,

		.padding = 5.0,

		.max_model_lwh = FLOAT3_INIT_MAX,

		.raw_load = 0,
		.raw_out  = 0,
		.describe_level = 0,

		.out_file = NULL,


		.volume = 0
};


void usage(int argc, char **argv, char *err, ...) {
		va_list va;
		FILE *stream = stderr;
		fprintf(stream, "%s [-options] {file0 [--operations]..}..{fileN [--operations]..}\n", argv[0]);
		fprintf(stream, "\tv%s.%s.%s\n", Version[0], Version[1], Version[2]);
		fprintf(stream, "\n");

		fprintf(stream, "Options:\n");
		fprintf(stream, "\t-h\tShow help\n");
		fprintf(stream, "\t-I\tCopy maximum 1 input object to output memory directly.\n");
		fprintf(stream, "\t-L <float>\tMaximum result object length\n");
		fprintf(stream, "\t-W <float>\tMaximum result object width\n");
		fprintf(stream, "\t-H <float>\tMaximum result object height\n");
		fprintf(stream, "\t-p\tPack input objects automatically\n");
		fprintf(stream, "\t-b <float>\tSet packing margin\n");
		fprintf(stream, "\t-o filename\tOutput the resulting composite object to `filename'\n");
		fprintf(stream, "\t-r\tDo not center and raise object above the Z plane on load\n");
		fprintf(stream, "\t-R\tDo not center and raise the result object above the Z plane\n");
		fprintf(stream, "\t-Z\tApply all pending transforms and place the current model on the Z plane\n");
		fprintf(stream, "\t-D\tIncrease the detail with which the result is described\n");
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
						case 'L': {
								char *dim = argv[++i];
								check(i < argc, "-%c requires a numeric parameter", opt);
								check(sscanf(dim, "%f", &options.max_model_lwh[0]) == 1,
									  "Invalid maximum length '%s'", dim);
								break;
						}
						case 'W': {
								char *dim = argv[++i];
								check(i < argc, "-%c requires a numeric parameter", opt);
								check(sscanf(dim, "%f", &options.max_model_lwh[1]) == 1,
									  "Invalid maximum width '%s'", dim);
								break;
						}
						case 'H': {
								char *dim = argv[++i];
								check(i < argc, "-%c requires a numeric parameter", opt);
								check(sscanf(dim, "%f", &options.max_model_lwh[2]) == 1,
									  "Invalid maximum height '%s'", dim);
								break;
						}
						case 'Z': {
								log_info("Applying pending transforms for %c", opt);
								transform_apply(latest);
								log_info("Chaining and applying Z-plane transform");
								object_transform_chain_zero_z(latest);
								transform_apply(latest);
						}
						case 'o':
								latest = &options.out;
								check(i + 1 < argc, "-%c requires a filename.", opt);
								options.out_file = argv[++i];
								break;
						case 'b':
								check(i + 1 < argc, "-%c requires a size.", opt);
								rc = sscanf(argv[++i], "%f", &options.padding);
								check(rc == 1, "-%c requires a size.", opt);
								log_info("Padding is now %f", options.padding);
								break;
						case 'p':
								log_info("Setting accumilation mode to Pack");
								options.op = Pack;
								break;
						case 'I':
								log_info("Setting memcopy collection mode.");
								options.op = Copy;
								break;
						case 'r':
								options.raw_load = !options.raw_load;
								log_info("Raw load is now %s", options.raw_load ? "ON" : "OFF");
								break;
						case 'R':
								options.raw_out = !options.raw_out;
								options.out.options = Noop;
								log_info("Raw out is now %s", options.raw_out ? "ON" : "OFF");
								break;
						case 'D':
								log_info("Result description level is now %d", ++options.describe_level);
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

						if(!options.raw_load) {
								log_info("Centering '%s'", arg);
								// Center and +Z the object
								object_transform_chain_zero_z(latest);
								object_transform_chain_center_x(latest);
								object_transform_chain_center_y(latest);
								// Apply any potential chained object transforms
								transform_apply(latest);
						}
						else {
								latest->options = Noop;
								log_info("NOT Centering '%s'", arg);
						}

						float3 b[2] = {FLOAT3_INIT, FLOAT3_INIT};
						float3 c = FLOAT3_INIT_MAX;
						object_bounds(latest->object, &b[0], &b[1]);
						object_center(latest->object, &c);
						log_info("%s center: (%f, %f, %f). Size: %fx%fx%f", arg, FLOAT3_FORMAT(c), f3X(b[1]) - f3X(b[0]), f3Y(b[1]) - f3Y(b[0]), f3Z(b[1]) - f3Z(b[0]));

						*kl_pushp(transformer, in_objects) = latest;
						log_info("Loaded: %s", arg);
				}
		}

		// Transform each object and count the total
		// facets for the output
		kliter_t(transformer) *tl_iter = NULL;
		uint32_t total_facets = 0;
		rc = transform_apply_list(in_objects);
		for(tl_iter = kl_begin(in_objects); tl_iter != kl_end(in_objects); tl_iter = kl_next(tl_iter)) {
				total_facets += kl_val(tl_iter)->object->facet_count;
		}
		check(total_facets > 0, "%d facets in resulting model is insufficient.", total_facets);

		// Compile and transform the output object
		check_mem(options.out.object = stl_alloc(NULL, total_facets));
		switch(options.op) {
		case Collect: {
				int collected = collect(options.out.object, in_objects);
				check(collected == total_facets, "Facets lost during collection.");
				break;
		}

		case Copy: {
				check(in_objects->size == 1, "Identiy-copy accumilation only allows for one input.");
				stl_object* input = kl_val(kl_begin(in_objects))->object;

				memcpy(options.out.object->header, input->header, sizeof(input->header));
				memcpy(options.out.object->facets, input->facets, sizeof(stl_facet) * input->facet_count);
				break;
		}

		case Pack: {
				check(chain_pack(in_objects, options.padding) == in_objects->size,
					  "Failed to chain pack transforms for all objects.");
				check(transform_apply_list(in_objects) == in_objects->size,
					  "Failed to apply pack transforms for all objects.");

				check(collect(options.out.object, in_objects) == total_facets,
					  "Facets lost during collection.");
				break;
		}

		default:
				sentinel("Unknown operation %d", options.op);
		}

		// Apply transformations to the result
		log_info("Output contains %d facets, performing accumilated transforms", total_facets);
		transform_apply(&options.out);

		// Make sure the result object is centered if needed
		log_info("%sCentering output object.", options.raw_out ? "NOT " : "");
		if(!options.raw_out) {
				object_transform_chain_zero_z(&options.out);
				object_transform_chain_center_x(&options.out);
				object_transform_chain_center_y(&options.out);
				transform_apply(&options.out);
		}

		// Perform bounds checking
		float3 bounds[2] = {FLOAT3_INIT, FLOAT3_INIT};
		object_bounds(options.out.object, &bounds[0], &bounds[1]);
		float3 dims = {f3X(bounds[1]) - f3X(bounds[0]), f3Y(bounds[1]) - f3Y(bounds[0]), f3Z(bounds[1]) - f3Z(bounds[0])};

		check((dims[0] <= options.max_model_lwh[0]) &&
			  (dims[1] <= options.max_model_lwh[1]) &&
			  (dims[2] <= options.max_model_lwh[2]),
			  "Bounds check FAILED: Result dimensions: %f x %f x %f Maximum dimensions: %f x %f x %f",
			  FLOAT3_FORMAT(dims), FLOAT3_FORMAT(options.max_model_lwh));

 		// Perform the "result" operation
		if(options.out_file != NULL) {
				log_info("Writing result object to: '%s'", options.out_file);
				rc = stl_write_file(options.out.object, options.out_file);
				check(rc == 0, "Failed to write output to %s" , options.out_file);
		}

		// Describe the result in as much detail as requested.
		if(options.describe_level > 0) {
				float3 center = FLOAT3_INIT;
				check(object_center(options.out.object, &center) == 0,
					  "Failed to get center of output object: %p", options.out.object);

				log_info("=> Output object description:");
				log_info("\tDimensions: %f x %f x %f Max: %f x %f x %f",
						 FLOAT3_FORMAT(dims), FLOAT3_FORMAT(options.max_model_lwh));
				log_info("\tCenter: (%f, %f, %f)", FLOAT3_FORMAT(center));
				log_info("\t%d faces", options.out.object->facet_count);
		}

		if(options.describe_level > 1) {
				log_info("=> Header.");
				for(int i = 0; i < sizeof(options.out.object->header); i += 16) {
						log_info("[0x%02X] %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", i,
								 options.out.object->header[i+0],
								 options.out.object->header[i+1],
								 options.out.object->header[i+2],
								 options.out.object->header[i+3],
								 options.out.object->header[i+4],
								 options.out.object->header[i+5],
								 options.out.object->header[i+6],
								 options.out.object->header[i+7],
								 options.out.object->header[i+8],
								 options.out.object->header[i+9],
								 options.out.object->header[i+10],
								 options.out.object->header[i+11],
								 options.out.object->header[i+12],
								 options.out.object->header[i+13],
								 options.out.object->header[i+14],
								 options.out.object->header[i+15]
								);
				}
		}

		if(options.describe_level > 2) {
				log_info("=> Faces:");
				for(uint32_t i = 0; i < options.out.object->facet_count; i++) {
						stl_facet* facet = &options.out.object->facets[i];
						log_info("\t%d Attr: 0x%X: Normal: <%f, %f, %f>",
								 i, facet->attr, FLOAT3_FORMAT(facet->normal));
						for(int v = 0; v < 3; v++) {
								log_info("\t\t%d: (%f, %f, %f)",
										 v, FLOAT3_FORMAT(facet->vertices[v]));
						}
				}
		}

		kl_destroy(transformer, in_objects);
		return 0;
error:
		kl_destroy(transformer, in_objects);
		return -1;
}
