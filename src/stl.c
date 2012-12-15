#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "dbg.h"

#include "stl.h"

void stl_free(stl_object *obj) {
		if(obj == NULL) return;
		if(obj->facets) free(obj->facets);
		free(obj);
}

stl_object *stl_alloc(char *header, uint32_t n_facets) {
		stl_object *obj = (stl_object*)calloc(1, sizeof(stl_object));
		check_mem(obj);

		if(header != NULL) {
				memcpy(obj->header, header, sizeof(obj->header));
		}

		obj->facet_count = n_facets;
		if(n_facets > 0) {
				obj->facets = (stl_facet*)calloc(n_facets, sizeof(stl_facet));
				check_mem(obj->facets);
		}

		return obj;
error:
		exit(-1);
}

void v3_cross(float3 *result, float3 v1, float3 v2, int normalize) {
		float3 v1_x_v2 = {
				v1[1]*v2[2] - v1[2]*v2[1],
				v1[2]*v2[0] - v1[0]*v2[2],
				v1[0]*v2[1] - v1[1]*v2[0]
		};
		if(normalize) {
				float mag = sqrt(v1_x_v2[0]*v1_x_v2[0] + v1_x_v2[1]*v1_x_v2[1] + v1_x_v2[2]*v1_x_v2[2]);
				v1_x_v2[0] /= mag;
				v1_x_v2[1] /= mag;
				v1_x_v2[2] /= mag;
		}
		memcpy(result, &v1_x_v2, sizeof(float3));
}

void stl_facet_update_normal(stl_facet *facet) {
		float3 *fvs = facet->vertices;
		float3 v1 = {fvs[0][0] - fvs[1][0], fvs[0][1] - fvs[1][1], fvs[0][2] - fvs[1][2]};
		float3 v2 = {fvs[0][0] - fvs[2][0], fvs[0][1] - fvs[2][1], fvs[0][2] - fvs[2][2]};
		v3_cross(&facet->normal, v1, v2, 1);
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

/*
 * Allocate, read, and return a anull terminated string from `fd'.
 * Return NULL in caes of error
 */
char* read_line(int fd, int downcase, int trim) {
		int rc = -1;
		char *buffer = NULL;
		size_t size = 1024;
		int cur = 0;
		check_mem((buffer = calloc(size, 1)));

		while((cur < size) && ((rc = read(fd, buffer + cur++, 1)) != -1)) {
				int last = cur - 1;
				check(rc != -1, "read_line(%d) failed", fd);
				if(downcase) buffer[last] = tolower(buffer[last]);
				if(buffer[last] == '\n') break;
				if(rc == 0) break;
		}
		check(cur < size, "BUG: Line exceeds allocated length of %zd in read_line(%d)", size, fd);

		if((strlen(buffer) == 0) && (rc == 0)) goto eof;

		if(trim) {
				char *new = NULL;
				int start = 0;
				int end = cur;
				while(isspace(buffer[start++]));
				while(isspace(buffer[--end]));
				if(start > 0) start--;
				buffer[++end] = '\0';
				check_mem((new = calloc(strlen(buffer + start), 1)));
				memcpy(new, buffer + start, end - start);
				free(buffer);
				buffer = new;
		}

		return buffer;

eof: /* "Deallocate and return NULL." Different label than error for clarity */
error:
		if(buffer != NULL) free(buffer);
		return NULL;
}

stl_object *stl_read_text_object(int fd) {
		stl_object *obj = NULL;
		char *line = read_line(fd, 0, 1);
		klist_t(stl_facet) *facets = kl_init(stl_facet);

		check(line != NULL, "Failed to read STL/ASCII header.");
		check((obj = stl_alloc(NULL, 0)), "Failed to allocated new STL object.");
		snprintf(obj->header, sizeof(obj->header), "[STL/ASCII]: '%s'", line);
		log_info("Header: [%s]", obj->header);
		free(line);

		size_t lines = 0;
		while((line = read_line(fd, 1, 1))) {
				lines++;
				log_info("Line: [%s]", line);
				free(line);
		}

		kl_destroy(stl_facet, facets);
		return obj;
error:
		kl_destroy(stl_facet, facets);
		return NULL;
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
				stl_facet *facet = stl_read_facet(fd);
				memcpy(&obj->facets[i], facet, sizeof(stl_facet));
				free(facet);
		}

		return obj;
error:
		if(obj) stl_free(obj);
		return NULL;
}

stl_object *stl_read_file(char *path) {
		stl_reader* reader = NULL;
		stl_object *obj = NULL;
		int fd = -1;

		check((reader = stl_detect_reader(path)), "Unable to find reader for format of %s", path);
		check((fd = open(path, O_RDONLY)) != -1, "Unable to open '%s'", path);

		obj = reader(fd);

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

int stl_write_file(stl_object *obj, char *path) {
		int rc = -1;
		int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0755);
		check(fd != -1, "Failed to open '%s' for write", path);

		rc = stl_write_object(obj, fd);

		close(fd);
		return rc;
error:
		if(fd != -1) close(fd);
		return -1;
}

int stl_write_object(stl_object *obj, int fd) {
		int rc = -1;

		rc = write(fd, obj->header, sizeof(obj->header));
		check(rc == sizeof(obj->header), "Failed to write object header");

		rc = write(fd, &obj->facet_count, sizeof(obj->facet_count));
		check(rc == sizeof(obj->facet_count), "Failed to write face count");

		for(uint32_t i = 0; i < obj->facet_count; i++) {
				rc = stl_write_facet(&obj->facets[i], fd);
				check(rc == 0, "Failed to write facet %d", i);
		}

		return 0;
error:
		return -1;
}

int stl_write_facet(stl_facet *facet, int fd) {
		// Pre-computed size since sizeof(struct) falls to padding problems for IO
		const size_t facet_size = sizeof(facet->normal) + sizeof(facet->vertices) + sizeof(facet->attr);
		int rc = -1;

		rc = write(fd, facet, facet_size);
		check(rc == facet_size, "Failed to write facet struct");

		return 0;
error:
		return -1;
}

stl_reader* stl_detect_reader(char *path) {
		stl_reader* reader = stl_read_text_object;
		static const int upto = 100;
		char c = '\0';
		int rc = -1;
		int fd = open(path, O_RDONLY);
		check(fd != -1, "Failed to open %s for format detection.", path);
		for(int i = 0; i < upto; i++) {
				check((rc = read(fd, &c, 1)) == 1, "Failed to read byte %d for reader detection of %s", i, path);
				if(!isprint(c) && !isspace(c)) {
						reader = stl_read_object;
						break;
				}
		}

		if(fd != -1) close(fd);
		return reader;
error:
		if(fd != -1) close(fd);
		return NULL;
}
