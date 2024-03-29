#include "tinyobj.h"
#include "geometry.h"
#include "tinytga.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
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
			face[0].y = strtof(ptr, &end)-1; ptr = end+1;
			face[0].z = strtof(ptr, &end)-1; ptr = end+1;


			face[1].x = strtof(ptr, &end)-1; ptr = end+1;
			face[1].y = strtof(ptr, &end)-1; ptr = end+1;
			face[1].z = strtof(ptr, &end)-1; ptr = end+1;


			face[2].x = strtof(ptr, &end)-1; ptr = end+1;
			face[2].y = strtof(ptr, &end)-1; ptr = end+1;
			face[2].z = strtof(ptr, &end)-1; ptr = end+1;

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

    // texture
    model->has_texture = has_texture;

	fclose(file);
	return model;
}

tt_color tobj_diffuse(tobj_model *model, Vec2f uv) {
    assert(model != NULL);
    assert(model->_diffuse_map != NULL);
    Vec2i xy = vec2i_make((int)(uv.x*model->_diffuse_map->width),
            (int)(uv.y*model->_diffuse_map->height));
    tt_color color = tt_make_color(
            model->_diffuse_map->pixels[(xy.y)*model->_diffuse_map->width+xy.x]
            );
    return color;
}

void tobj_load_diffusemap(tobj_model *model, const char* filepath) {
    if (model->has_texture == false) {
        return ;
    }
    model->_diffuse_map = tt_load_from_file(filepath);
}
void tobj_load_normalmap(tobj_model *model, const char* filepath) {
    model->_normal_map = tt_load_from_file(filepath);
    tt_flip_vertically(model->_normal_map);
}

void tobj_load_specularmap(tobj_model *model, const char* filepath) {
    model->_specular_map = tt_load_from_file(filepath);
}


int *tobj_get_face(tobj_model *model, int idx) {
    int *face = (int*)malloc(sizeof(int)*3);
    for (int i = 0; i < 3; i++) {
        face[i] = model->faces[idx][i].x;
    }
    return face;
}

Vec2f tobj_get_uv(tobj_model *model, int iface, int nvert) {
    int idx = model->faces[iface][nvert].y;
    Vec2f result = {
        .x = model->uv[idx].x,
        .y = model->uv[idx].y
    };
    return result;
}

Vec3f tobj_get_vert(tobj_model *model, int i) {
    return model->verts[i];
}

Vec3f tobj_get_vert_from_face(tobj_model *model, int iface, int nthvert) {
    return model->verts[model->faces[iface][nthvert].x];
}

Vec3f tobj_get_normal_from_map(tobj_model *model, Vec2f uvf) {
    Vec2i uv = vec2i_make((int)(uvf.x*model->_normal_map->width), (int)(uvf.y*model->_normal_map->height));
    tt_color c = tt_get_color_from(model->_normal_map, uv.x, uv.y);
    Vec3f res;
    // color.x: 0~255
    // normal.x: -1~1
    vec3f_set(&res, 2, (float)c.b/255.f*2.f - 1.f);
    vec3f_set(&res, 1, (float)c.g/255.f*2.f - 1.f);
    vec3f_set(&res, 0, (float)c.r/255.f*2.f - 1.f);
    return res;
}

float tobj_get_specular(tobj_model *model, Vec2f uvf) {
    Vec2i uv = vec2i_make((int)(uvf.x*model->_specular_map->width), (int)(uvf.y*model->_specular_map->height));
    tt_color c = tt_get_color_from(model->_specular_map, uv.x, uv.y);
    return c.b / 1.f;
    
}

Vec3f tobj_get_normal(tobj_model *model, int iface, int nvert) {
    int idx = model->faces[iface][nvert].z;
    Vec3f temp = model->norms[idx];
    vec3f_normalize(&temp, 1);
    return temp;
}




