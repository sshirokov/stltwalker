#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "dbg.h"

#include "stl.h"

void stl_free(stl_object *obj) {
		log_err("TODO: stl_free() is a stub.");
}

stl_object *stl_alloc(char *header, uint32_t n_facets) {
		stl_object *obj = (stl_object*)malloc(sizeof(stl_object));
		check_mem(obj);
		bzero(obj, sizeof(stl_object));

		obj->facet_count = n_facets;
		if(n_facets > 0) {
				obj->facets = (stl_facet*)calloc(n_facets, sizeof(stl_facet));
				check_mem(obj->facets);
		}

		return obj;
error:
		exit(-1);
}

stl_facet *stl_read_facet(int fd) {
		int rc = -1;
		stl_facet *facet = (stl_facet*)calloc(1, sizeof(stl_facet));
		check_mem(facet);

		rc = read(fd, &facet->normal, sizeof(facet->normal));
		check(rc == sizeof(facet->normal), "Failed to read normal. Read %d expected %zu", rc, sizeof(facet->normal));
		rc = read(fd, &facet->vertices, sizeof(facet->vertices));
		check(rc == sizeof(facet->vertices), "Failed to read triangle vertecies. Read %d expected %zu", rc, sizeof(facet->vertices));
		rc = read(fd, &facet->attr, sizeof(facet->attr));
		check(rc == sizeof(facet->attr), "Failed to read attr bytes. Read %d expect %zu", rc, sizeof(facet->attr));

		return facet;
error:
		exit(-1);
}

stl_object *stl_read_object(int fd) {
		int rc = -1;
		char header[80] = {0};
		uint32_t n_tris = 0;
		stl_object *obj = NULL;

		// Read the header
		rc = read(fd, header, sizeof(header));
		check(rc == sizeof(header), "Unable to read STL header. Got %d bytes.", rc);

		// Triangle count
		rc = read(fd, &n_tris, sizeof(n_tris));
		check(rc == sizeof(n_tris), "Failed to read facet count.");
		check(n_tris > 0, "Facet count cannot be zero.");

		obj = stl_alloc(header, n_tris);

		for(uint32_t i = 0; i < obj->facet_count; i++) {
				//log_info("Reading triangle %d/%d", i + 1, obj->facet_count);
				stl_facet *facet = stl_read_facet(fd);
				/* log_info(" N <%f, %f, %f>", facet->normal[0], facet->normal[1], facet->normal[2]); */
				/* log_info(" V1: (%f, %f, %f)", facet->vertices[0][0], facet->vertices[0][1], facet->vertices[0][2]); */
				/* log_info(" V2: (%f, %f, %f)", facet->vertices[1][0], facet->vertices[1][1], facet->vertices[1][2]); */
				/* log_info(" V3: (%f, %f, %f)", facet->vertices[2][0], facet->vertices[2][1], facet->vertices[2][2]); */
				memcpy(&obj->facets[i], facet, sizeof(stl_facet));
				free(facet);
		}

		return obj;
error:
		if(obj) stl_free(obj);
		return NULL;
}

stl_object *stl_read_file(char *path) {
		stl_object *obj = NULL;
		int fd = open(path, O_RDONLY);
		check(fd != -1, "Unable to open '%s'", path);

		obj = stl_read_object(fd);

		char buffer[10];
		int rc = read(fd, buffer, sizeof(buffer));
		check(rc == 0, "File did not end when expected, assuming failure.");
		close(fd);

		return obj;
error:
		if(fd != -1) close(fd);
		if(obj) stl_free(obj);
		return NULL;
}
