#include "stl.h"
#include "transform.h"

#ifndef __PACK_H
#define __PACK_H

#define MINf(x, y) ((x) < (y) ? (x) : (y))
#define MAXf(x, y) ((x) > (y) ? (x) : (y))

int object_bounds(stl_object *obj, float3 *min, float3 *max);
int object_center(stl_object *obj, float3 *center);

int chain_pack(klist_t(transformer) *objects, float buffer);
int collect(stl_object *out, klist_t(transformer) *in);

#endif
