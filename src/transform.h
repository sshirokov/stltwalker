#include "stl.h"
#include "matrix.h"

#ifndef __TRANSFORM_H
#define __TRANSFORM_H

typedef struct s_stl_transformer {
		stl_object *object;

		float4x4 transform;
} stl_transformer;

stl_transformer *transformer_alloc(stl_object *obj);
void transformer_free(stl_transformer *t);

stl_transformer *transformer_init(stl_transformer *t, stl_object *obj);

// Composite wrappers
#define mp_transformer_free(x) transformer_free(kl_val(x))
KLIST_INIT(transformer, stl_transformer*, mp_transformer_free)

#endif
