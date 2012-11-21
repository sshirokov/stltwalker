#ifndef __MATRIX_H
#define __MATRIX_H

typedef float float4x4[4][4];
typedef float float4x1[4][1];

extern const float4x4 Identity4x4;

void mult_4x4f(float4x4 *result, float4x4 a, float4x4 b);
void mult_4x1f(float4x1 *result, float4x4 a, float4x1 b);

#endif
