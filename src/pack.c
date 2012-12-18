#include <math.h>

#include "klist.h"
#include "dbg.h"

#include "stl.h"
#include "transform.h"

#include "pack.h"

int object_bounds(stl_object *obj, float3 *min, float3 *max) {
		check(obj, "No object.");
		check(obj->facet_count > 0, "No facets in object.");
		const float3 f3Min = FLOAT3_INIT_MIN;
		const float3 f3Max = FLOAT3_INIT_MAX;

		memcpy(min, &f3Max, sizeof(float3));
		memcpy(max, &f3Min, sizeof(float3));

		for(int i = 0; i < obj->facet_count; i++) {
				stl_facet *facet = &(obj->facets[i]);
				for(int v = 0; v < 3; v++) {
						float3 *vert = facet->vertices + v;
						f3X(*min) = MINf(f3X(*min), f3X(*vert));
						f3X(*max) = MAXf(f3X(*max), f3X(*vert));

						f3Y(*min) = MINf(f3Y(*min), f3Y(*vert));
						f3Y(*max) = MAXf(f3Y(*max), f3Y(*vert));

						f3Z(*min) = MINf(f3Z(*min), f3Z(*vert));
						f3Z(*max) = MAXf(f3Z(*max), f3Z(*vert));
				}
		}

		return 0;
error:
		return -1;
}

int object_center(stl_object *obj, float3 *center) {
		float3 bounds[2] = {FLOAT3_INIT, FLOAT3_INIT};
		check(object_bounds(obj, &bounds[0], &bounds[1]) == 0, "Failed to compute bounds for center.");

		f3X(*center) = (f3X(bounds[0]) + f3X(bounds[1])) / 2.0;
		f3Y(*center) = (f3Y(bounds[0]) + f3Y(bounds[1])) / 2.0;
		f3Z(*center) = (f3Z(bounds[0]) + f3Z(bounds[1])) / 2.0;

		return 0;
error:
		return -1;
}

int collect(stl_object *out, klist_t(transformer) *in) {
		kliter_t(transformer) *tl_iter = NULL;
		int collected = 0;
		for(tl_iter = kl_begin(in); tl_iter != kl_end(in); tl_iter = kl_next(tl_iter)) {
				stl_transformer *transformer = kl_val(tl_iter);
				stl_object *object = transformer->object;
				memcpy(out->facets + collected,
					   object->facets,
					   sizeof(stl_facet) * object->facet_count);
				collected += object->facet_count;
		}
		return collected;
}

int chain_pack(klist_t(transformer) *objects) {

		return objects->size;
}
