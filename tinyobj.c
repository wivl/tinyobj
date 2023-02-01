#include "tinyobj.h"
#include "geometry.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define DEFAULT_CHUNK_SIZE 100


void _dev_log(tobj_model *model);



tobj_model* tobj_load_model(const char *filename) {
	tobj_model* model = NULL;
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "[ERR]File %s not found.\n", filename);
		return NULL;
	}

	model = (tobj_model*)malloc(sizeof(tobj_model));
	model->verts = (Vec3f*)malloc(sizeof(Vec3f)*DEFAULT_CHUNK_SIZE);
	model->faces = (int **)malloc(sizeof(int*)*DEFAULT_CHUNK_SIZE);
	model->v_capp = 0;
	model->f_capp = 0;
	size_t v_chunk_count = 1;
	size_t f_chunk_count = 1;
	size_t v_index = 0;
	size_t f_index = 0;

	char* line_buf = NULL;
	size_t line_buf_size = 0;
	ssize_t line_size = getline(&line_buf, &line_buf_size, file);

	while (line_size >= 0) {
		if (line_buf == NULL) {
			break;
		}
		if (strncmp(line_buf, "v ", 2) == 0) {
			// get v value
			char *ptr = line_buf;
			ptr += 2;
			char *end;
			double f;
			f = strtof(ptr, &end); ptr = end;
			model->verts[v_index].x = f;
			f = strtof(ptr, &end); ptr = end;
			model->verts[v_index].y = f;
			f = strtof(ptr, &end); ptr = end;
			model->verts[v_index].z = f;

			v_index++;

			if (v_index >= v_chunk_count*DEFAULT_CHUNK_SIZE) {
				v_chunk_count++;
				model->verts = (Vec3f*)realloc(model->verts, sizeof(Vec3f)*v_chunk_count*DEFAULT_CHUNK_SIZE);
			}
		} else if (strncmp(line_buf, "f ", 2) == 0) {
			char *ptr = line_buf;
			ptr += 2;
			char *end;

			int *face = (int*)malloc(sizeof(int)*3);

			long l;
			l = strtof(ptr, &end); ptr = end+1;
			face[0] = l;
			// printf("[DEV]f %ld/", l);
			l = strtof(ptr, &end); ptr = end+1;
			// printf("%ld/", l);
			l = strtof(ptr, &end); ptr = end+1;
			// printf("%ld ", l);


			l = strtof(ptr, &end); ptr = end+1;
			face[1] = l;
			// printf("%ld/", l);
			l = strtof(ptr, &end); ptr = end+1;
			// printf("%ld/", l);
			l = strtof(ptr, &end); ptr = end+1;
			// printf("%ld ", l);


			l = strtof(ptr, &end); ptr = end+1;
			face[2] = l;
			// printf("%ld/", l);
			l = strtof(ptr, &end); ptr = end+1;
			// printf("%ld/", l);
			l = strtof(ptr, &end); ptr = end+1;
			// printf("%ld\n", l);

			model->faces[f_index] = face;

			f_index++;

			if (f_index >= f_chunk_count*DEFAULT_CHUNK_SIZE) {
				f_chunk_count++;
				model->faces = (int **)realloc(model->faces, sizeof(int*)*f_chunk_count*DEFAULT_CHUNK_SIZE);
			}
		}

		line_size = getline(&line_buf, &line_buf_size, file);
	}
	model->v_capp = v_index;
	model->f_capp = f_index;
	_dev_log(model);

	fclose(file);
	return model;
}

void _dev_log(tobj_model *model) {
	for (int i = 0; i < model->v_capp; i++) {
		printf("v %f %f %f\n", model->verts[i].x, model->verts[i].y, model->verts[i].z);
	}
	for (int i = 0; i < model->f_capp; i++) {
		printf("f %d/-/- %d/-/- %d/-/-\n", model->faces[i][0], model->faces[i][1], model->faces[i][2]);
	}
	printf("v: %ld f: %ld\n", model->v_capp, model->f_capp);
}
