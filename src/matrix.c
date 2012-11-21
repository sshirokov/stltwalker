#include <strings.h>
#include <stdio.h>

#include "matrix.h"

const float4x4 Identity4x4 = {
		{1.0, 0.0, 0.0, 0.0},
		{0.0, 1.0, 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{0.0, 0.0, 0.0, 1.0}
};

/*
 * Matrix multiplication functions in the format
 *  mult_#{Ra}x#{Cb}f(r, a, b);
 *    * Where Ra is the number of rows in matrix a
 *    * And Cb is the number of cols in matrix b
 *    * the last letter is the type, but everything
 *      is a fucking float, so whatever.
 */
void mult_4x4f(float4x4 *result, float4x4 a, float4x4 b) {
		bzero(result, sizeof(float4x4));
		for(int row = 0; row < 4; row++) {
				for(int col = 0; col < 4; col++) {
						for(int i = 0; i < 4; i++)
								(*result)[row][col] += a[row][i] * b[i][col];
				}
		}
}

void mult_4x1f(float4x1 *result, float4x4 a, float4x1 b) {
		static const int col = 0;
		bzero(result, sizeof(float4x1));
		for(int row = 0; row < 4; row++) {
				for(int i = 0; i < 4; i++) {
						(*result)[row][col] += a[row][i] * b[i][col];
				}
		}
}
