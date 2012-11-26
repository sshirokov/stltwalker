#include <math.h>

#include "dbg.h"

#include "stl.h"
#include "transform.h"

#include "pack.h"

int object_bounds(stl_object *obj, float3 *min, float3 *max) {
		check(obj, "No object.");
		check(obj->facet_count > 0, "No facets in object.");

		for(int i = 0; i < obj->facet_count; i++) {
				stl_facet *facet = &(obj->facets[i]);
				for(int v = 0; v < 3; v++) {
						float3 *vert = facet->vertices + v;
						f3X(*min) = MINf(f3X(*min), f3X(*vert));
						f3X(*max) = MAXf(f3X(*min), f3X(*vert));

						f3Y(*min) = MINf(f3Y(*min), f3Y(*vert));
						f3Y(*max) = MAXf(f3Y(*min), f3Y(*vert));

						f3Z(*min) = MINf(f3Z(*min), f3Z(*vert));
						f3Z(*max) = MAXf(f3Z(*min), f3Z(*vert));
				}
		}

		return 0;
error:
		return -1;
}

void transform_chain_zero_z(stl_transformer *t) {
		float3 bounds[2] = {{INF, INF, INF}, {-INF, -INF, -INF}};
		float3 v = FLOAT3_INIT;
		float4x4 tr;
		object_bounds(t->object, &bounds[0], &bounds[1]);
		f3Z(v) = -f3Z(bounds[0]);
		transform_chain(t, *init_transform_translate_f(&tr, v));
}
