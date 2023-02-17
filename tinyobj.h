#ifndef _TINY_OBJ_H
#define _TINY_OBJ_H

#include "geometry.h"
#include "tinytga.h"
#include <stddef.h>
#include <stdbool.h>

enum has_texture {
    NO_TEXTURE = 0,
    TEXTURE_TGA = 1
};

typedef struct {

	size_t v_capp;
	size_t f_capp;
	size_t n_capp;
	size_t u_capp;

	Vec3f *verts;
	Vec3i **faces; // attention, this Vec3i means vertex/uv/normal
    Vec3f *norms;
    Vec2f *uv;
    tt_image *_diffuse_map;

    int has_texture;

} tobj_model;

tobj_model* tobj_load_model(const char *filename, bool has_texture);

tt_color tobj_diffuse(tobj_model *model, Vec2i uv);

void tobj_load_texture(tobj_model *model, const char* filepath);

int *tobj_get_face(tobj_model *model, int idx);

#endif
