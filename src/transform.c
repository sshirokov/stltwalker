#include "dbg.h"

#include "transform.h"

stl_transformer *transformer_alloc(stl_object *obj) {
		stl_transformer *t = (stl_transformer*)calloc(1, sizeof(stl_transformer));
		check_mem(t);
		return transformer_init(t, obj);
error:
		exit(-1);
}

void transformer_free(stl_transformer *t) {
		if(t == NULL) return;
		if(t->object) stl_free(t->object);
		free(t);
}

stl_transformer *transformer_init(stl_transformer *t, stl_object *obj) {
		t->object = obj;
		memcpy(t->transform, Identity4x4, sizeof(float4x4));
		return t;
}

void float3tofloat4x1(const float3 *v, float4x1 *m) {
		(*m)[0][0] = (*v)[0];
		(*m)[1][0] = (*v)[1];
		(*m)[2][0] = (*v)[2];
		(*m)[3][0] = 1.0f;
}
