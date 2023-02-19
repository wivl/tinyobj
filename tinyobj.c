#include "tinyobj.h"
#include "geometry.h"
#include "tinytga.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define DEFAULT_CHUNK_SIZE 100



tobj_model* tobj_load_model(const char *filename, bool has_texture) {
	tobj_model* model = NULL;
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "[ERR]File %s not found.\n", filename);
		return NULL;
	}

	model = (tobj_model*)malloc(sizeof(tobj_model));
	model->verts = (Vec3f*)malloc(sizeof(Vec3f)*DEFAULT_CHUNK_SIZE);
	model->faces = (Vec3i **)malloc(sizeof(int*)*DEFAULT_CHUNK_SIZE);
	model->uv = (Vec2f*)malloc(sizeof(Vec2f)*DEFAULT_CHUNK_SIZE);
	model->norms = (Vec3f*)malloc(sizeof(Vec3f)*DEFAULT_CHUNK_SIZE);

	model->v_capp = 0;
	model->f_capp = 0;
	model->u_capp = 0;
	model->n_capp = 0;

	size_t v_chunk_count = 1;
	size_t f_chunk_count = 1;
	size_t u_chunk_count = 1;
	size_t n_chunk_count = 1;

	size_t v_index = 0;
	size_t f_index = 0;
	size_t u_index = 0;
	size_t n_index = 0;

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

			Vec3i *face = (Vec3i*)malloc(sizeof(Vec3i)*3);

			face[0].x = strtof(ptr, &end)-1; ptr = end+1;
			// printf("[DEV]f %ld/", l);
			face[0].y = strtof(ptr, &end)-1; ptr = end+1;
			// printf("%ld/", l);
			face[0].z = strtof(ptr, &end)-1; ptr = end+1;
			// printf("%ld ", l);


			face[1].x = strtof(ptr, &end)-1; ptr = end+1;
			// printf("%ld/", l);
			face[1].y = strtof(ptr, &end)-1; ptr = end+1;
			// printf("%ld/", l);
			face[1].z = strtof(ptr, &end)-1; ptr = end+1;
			// printf("%ld ", l);


			face[2].x = strtof(ptr, &end)-1; ptr = end+1;
			// printf("%ld/", l);
			face[2].y = strtof(ptr, &end)-1; ptr = end+1;
			// printf("%ld/", l);
			face[2].z = strtof(ptr, &end)-1; ptr = end+1;
			// printf("%ld\n", l);

			model->faces[f_index++] = face;


			if (f_index >= f_chunk_count*DEFAULT_CHUNK_SIZE) {
				f_chunk_count++;
				model->faces = (Vec3i **)realloc(model->faces, sizeof(Vec3i*)*f_chunk_count*DEFAULT_CHUNK_SIZE);
			}
		} else if (strncmp(line_buf, "vn ", 3) == 0) {
            char *ptr = line_buf;
            ptr += 3;
            char *end;
            Vec3f n;
            n.x = strtof(ptr, &end); ptr = end+1;
            n.y = strtof(ptr, &end); ptr = end+1;
            n.z = strtof(ptr, &end); ptr = end+1;

            model->norms[n_index++] = n;
			if (n_index >= n_chunk_count*DEFAULT_CHUNK_SIZE) {
				n_chunk_count++;
				model->norms = (Vec3f *)realloc(model->norms, sizeof(Vec3f)*n_chunk_count*DEFAULT_CHUNK_SIZE);
			}

        } else if (strncmp(line_buf, "vt ", 3) == 0) {
            char *ptr = line_buf;
            ptr += 3;
            char *end;
            Vec2f uv;
            uv.x = strtof(ptr, &end); ptr = end+1;
            uv.y = strtof(ptr, &end); ptr = end+1;

            model->uv[u_index++] = uv;
			if (u_index >= u_chunk_count*DEFAULT_CHUNK_SIZE) {
				u_chunk_count++;
				model->uv = (Vec2f *)realloc(model->uv, sizeof(Vec2f)*u_chunk_count*DEFAULT_CHUNK_SIZE);
			}
        }

		line_size = getline(&line_buf, &line_buf_size, file);
	}
	model->v_capp = v_index;
	model->f_capp = f_index;
	model->u_capp = u_index;
	model->n_capp = n_index;
	// _dev_log(model);

    // texture
    model->has_texture = has_texture;

	fclose(file);
	return model;
}

tt_color tobj_diffuse(tobj_model *model, Vec2i uv) {
    assert(model != NULL);
    assert(model->_diffuse_map != NULL);
    tt_color color = tt_make_color(model->_diffuse_map->pixels[(uv.y)*model->_diffuse_map->width+uv.x]);
    return color;
}

void tobj_load_texture(tobj_model *model, const char* filepath) {
    if (model->has_texture == false) {
        return ;
    }
    model->_diffuse_map = tt_load_from_file(filepath);
    // tt_flip_vertically(model->_diffuse_map);
}

int *tobj_get_face(tobj_model *model, int idx) {
    int *face = (int*)malloc(sizeof(int)*3);
    for (int i = 0; i < 3; i++) {
        face[i] = model->faces[idx][i].x;
    }
    return face;
}

Vec2i tobj_get_uv(tobj_model *model, int iface, int nvert) {
    int idx = model->faces[iface][nvert].y;
    Vec2i result = {
        .x = model->uv[idx].x * model->_diffuse_map->width,
        .y = model->uv[idx].y * model->_diffuse_map->height
    };
    return result;
}
