#include <unistd.h>
#include <stdint.h>

#ifndef __STL_H
#define __STL_H

// Wrappers
typedef float float3[3];


// File format structs
typedef struct s_stl_facet {
		float3 normal;
		float3 vertices[3];
		uint16_t attr;
} stl_facet;

typedef struct s_stl_object {
		char header[80];
		uint32_t facet_count;
		stl_facet *facets;
} stl_object;


// Alloc/Free
void stl_free(stl_object *obj);
stl_object *stl_alloc(char *header, uint32_t n_facets);

// Binary file readers
stl_facet *stl_read_facet(int fd);
stl_object *stl_read_object(int fd);
stl_object *stl_read_file(char *path);

// Binary file writers
int stl_write_file(stl_object *obj, char *path);
int stl_write_object(stl_object *obj, int fd);
int stl_write_facet(stl_facet *facet, int fd);

#endif
