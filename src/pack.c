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
