#ifndef _TINY_OBJ_H
#define _TINY_OBJ_H

#include "geometry.h"
#include <stddef.h>

typedef struct {
	size_t v_capp;
	size_t f_capp;
	Vec3f *verts;
	int **faces;
} tobj_model;

tobj_model* tobj_load_model(const char *filename);

#endif
