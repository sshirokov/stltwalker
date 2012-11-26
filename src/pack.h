#include "stl.h"
#include "transform.h"

#ifndef __PACK_H
#define __PACK_H

#define MINf(x, y) ((x) < (y) ? (x) : (y))
#define MAXf(x, y) ((x) > (y) ? (x) : (y))

int object_bounds(stl_object *obj, float3 *min, float3 *max);

void transform_chain_zero_z(stl_transformer *t);

#endif
