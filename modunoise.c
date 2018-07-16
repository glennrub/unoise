// Copyright (c) 2008, Casey Duncan (casey dot duncan at gmail dot com)
// Copyright (c) 2018 Glenn Ruben Bakke
// see LICENSE.txt for details
// $Id$<
#include <math.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "noise.h"

#define lerp(t, a, b) ((a) + (t) * ((b) - (a)))

float inline
grad1(mp_int_t hash, mp_float_t x)
{
	mp_float_t g = (hash & 7) + 1.0f;
	if (hash & 8)
		g = -1;
	return (g * x);
}

mp_float_t
noise1(mp_float_t x, mp_int_t repeat, mp_int_t base)
{
	mp_float_t fx;
	mp_int_t i = (mp_int_t)floorf(x) % repeat;
	mp_int_t ii = (i + 1) % repeat;
	i = (i & 255) + base;
	ii = (ii & 255) + base;

	x -= floorf(x);
	fx = x*x*x * (x * (x * 6 - 15) + 10);

	return lerp(fx, grad1(PERM[i], x), grad1(PERM[ii], x - 1)) * 0.4f;
}

static mp_obj_t py_noise1(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_x, ARG_octaves, ARG_persistence, ARG_lucanarity, ARG_repeat, ARG_base };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,           MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_octaves,     MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1} }, // 1 second
        { MP_QSTR_persistence, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_lucanarity,  MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_obj = mp_const_none} },    
        { MP_QSTR_repeat,      MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 1024} },
        { MP_QSTR_base,        MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    mp_float_t x = mp_obj_get_float(args[ARG_x].u_obj);
    mp_int_t octaves = args[ARG_octaves].u_int;

    mp_float_t persistence;
    if (args[ARG_persistence].u_obj != mp_const_none) {
        persistence = mp_obj_get_float(args[ARG_persistence].u_obj);
    } else {
	persistence = 0.5f;
    }

    mp_float_t lacunarity;
    if (args[ARG_lucanarity].u_obj != mp_const_none) {
        lacunarity = mp_obj_get_float(args[ARG_lucanarity].u_obj);
    } else {
        lacunarity = 2.0f;
    }

    mp_int_t repeat = args[ARG_repeat].u_int; // arbitrary
    mp_int_t base = args[ARG_base].u_int;

    if (octaves == 1) {
        // Single octave, return simple noise
        return mp_obj_new_float((double) noise1(x, repeat, base));
    } else if (args[ARG_octaves].u_int > 1) {
        int i;
        mp_float_t freq = 1.0f;
        mp_float_t amp = 1.0f;
        mp_float_t max = 0.0f;
        mp_float_t total = 0.0f;

        for (i = 0; i < octaves; i++) {
            total += noise1(x * freq, (const int)(repeat * freq), base) * amp;
            max += amp;
            freq *= lacunarity;
            amp *= persistence;
	}
        return mp_obj_new_float((double) (total / max));
    } else {
        mp_raise_ValueError("Expected octaves value > 0");
        return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(unoise_pnoise1_obj, 1, py_noise1);

//float inline
//grad2(const int hash, const float x, const float y)
//{
//	const int h = hash & 15;
//	return x * GRAD3[h][0] + y * GRAD3[h][1];
//}
//
//float
//noise2(float x, float y, const float repeatx, const float repeaty, const int base)
//{
//	float fx, fy;
//	int A, AA, AB, B, BA, BB;
//	int i = (int)floorf(fmodf(x, repeatx));
//	int j = (int)floorf(fmodf(y, repeaty));
//	int ii = (int)fmodf(i + 1, repeatx);
//	int jj = (int)fmodf(j + 1, repeaty);
//	i = (i & 255) + base;
//	j = (j & 255) + base;
//	ii = (ii & 255) + base;
//	jj = (jj & 255) + base;
//
//	x -= floorf(x); y -= floorf(y);
//	fx = x*x*x * (x * (x * 6 - 15) + 10);
//	fy = y*y*y * (y * (y * 6 - 15) + 10);
//
//	A = PERM[i];
//	AA = PERM[A + j];
//	AB = PERM[A + jj];
//	B = PERM[ii];
//	BA = PERM[B + j];
//	BB = PERM[B + jj];
//		
//	return lerp(fy, lerp(fx, grad2(PERM[AA], x, y),
//							 grad2(PERM[BA], x - 1, y)),
//					lerp(fx, grad2(PERM[AB], x, y - 1),
//							 grad2(PERM[BB], x - 1, y - 1)));
//}
//
//static PyObject *
//py_noise2(PyObject *self, PyObject *args, PyObject *kwargs)
//{
//	float x, y;
//	int octaves = 1;
//	float persistence = 0.5f;
//    float lacunarity = 2.0f;
//	float repeatx = 1024; // arbitrary
//	float repeaty = 1024; // arbitrary
//	int base = 0;
//
//	static char *kwlist[] = {"x", "y", "octaves", "persistence", "lacunarity", "repeatx", "repeaty", "base", NULL};
//
//	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "ff|iffffi:noise2", kwlist,
//		&x, &y, &octaves, &persistence, &lacunarity, &repeatx, &repeaty, &base))
//		return NULL;
//	
//	if (octaves == 1) {
//		// Single octave, return simple noise
//		return (PyObject *) PyFloat_FromDouble((double) noise2(x, y, repeatx, repeaty, base));
//	} else if (octaves > 1) {
//		int i;
//		float freq = 1.0f;
//		float amp = 1.0f;
//		float max = 0.0f;
//		float total = 0.0f;
//
//		for (i = 0; i < octaves; i++) {
//			total += noise2(x * freq, y * freq, repeatx * freq, repeaty * freq, base) * amp;
//			max += amp;
//			freq *= lacunarity;
//			amp *= persistence;
//		}
//		return (PyObject *) PyFloat_FromDouble((double) (total / max));
//	} else {
//		PyErr_SetString(PyExc_ValueError, "Expected octaves value > 0");
//		return NULL;
//	}
//}
//
//float inline
//grad3(const int hash, const float x, const float y, const float z)
//{
//	const int h = hash & 15;
//	return x * GRAD3[h][0] + y * GRAD3[h][1] + z * GRAD3[h][2];
//}
//
//float
//noise3(float x, float y, float z, const int repeatx, const int repeaty, const int repeatz, 
//	const int base)
//{
//	float fx, fy, fz;
//	int A, AA, AB, B, BA, BB;
//	int i = (int)floorf(fmodf(x, repeatx));
//	int j = (int)floorf(fmodf(y, repeaty));
//	int k = (int)floorf(fmodf(z, repeatz));
//	int ii = (int)fmodf(i + 1,  repeatx);
//	int jj = (int)fmodf(j + 1, repeaty);
//	int kk = (int)fmodf(k + 1, repeatz);
//	i = (i & 255) + base;
//	j = (j & 255) + base;
//	k = (k & 255) + base;
//	ii = (ii & 255) + base;
//	jj = (jj & 255) + base;
//	kk = (kk & 255) + base;
//
//	x -= floorf(x); y -= floorf(y); z -= floorf(z);
//	fx = x*x*x * (x * (x * 6 - 15) + 10);
//	fy = y*y*y * (y * (y * 6 - 15) + 10);
//	fz = z*z*z * (z * (z * 6 - 15) + 10);
//
//	A = PERM[i];
//	AA = PERM[A + j];
//	AB = PERM[A + jj];
//	B = PERM[ii];
//	BA = PERM[B + j];
//	BB = PERM[B + jj];
//		
//	return lerp(fz, lerp(fy, lerp(fx, grad3(PERM[AA + k], x, y, z),
//									  grad3(PERM[BA + k], x - 1, y, z)),
//							 lerp(fx, grad3(PERM[AB + k], x, y - 1, z),
//									  grad3(PERM[BB + k], x - 1, y - 1, z))),
//					lerp(fy, lerp(fx, grad3(PERM[AA + kk], x, y, z - 1),
//									  grad3(PERM[BA + kk], x - 1, y, z - 1)),
//							 lerp(fx, grad3(PERM[AB + kk], x, y - 1, z - 1),
//									  grad3(PERM[BB + kk], x - 1, y - 1, z - 1))));
//}
//
//static PyObject *
//py_noise3(PyObject *self, PyObject *args, PyObject *kwargs)
//{
//	float x, y, z;
//	int octaves = 1;
//	float persistence = 0.5f;
//    float lacunarity = 2.0f;
//	int repeatx = 1024; // arbitrary
//	int repeaty = 1024; // arbitrary
//	int repeatz = 1024; // arbitrary
//	int base = 0;
//
//	static char *kwlist[] = {"x", "y", "z", "octaves", "persistence", "lacunarity",
//		"repeatx", "repeaty", "repeatz", "base", NULL};
//
//	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "fff|iffiiii:noise3", kwlist,
//		&x, &y, &z, &octaves, &persistence, &lacunarity, &repeatx, &repeaty, &repeatz, &base))
//		return NULL;
//	
//	if (octaves == 1) {
//		// Single octave, return simple noise
//		return (PyObject *) PyFloat_FromDouble((double) noise3(x, y, z, 
//			repeatx, repeaty, repeatz, base));
//	} else if (octaves > 1) {
//		int i;
//		float freq = 1.0f;
//		float amp = 1.0f;
//		float max = 0.0f;
//		float total = 0.0f;
//
//		for (i = 0; i < octaves; i++) {
//			total += noise3(x * freq, y * freq, z * freq, 
//				(const int)(repeatx*freq), (const int)(repeaty*freq), (const int)(repeatz*freq), base) * amp;
//			max += amp;
//			freq *= lacunarity;
//			amp *= persistence;
//		}
//		return (PyObject *) PyFloat_FromDouble((double) (total / max));
//	} else {
//		PyErr_SetString(PyExc_ValueError, "Expected octaves value > 0");
//		return NULL;
//	}
//}

STATIC const mp_rom_map_elem_t module_unoise_globals_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR___name__), MP_OBJ_NEW_QSTR(MP_QSTR_unoise) },
    { MP_ROM_QSTR(MP_QSTR_pnoise1),     MP_ROM_PTR(&unoise_pnoise1_obj) },
};

STATIC MP_DEFINE_CONST_DICT(module_unoise_globals, module_unoise_globals_table);

const mp_obj_module_t unoise_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&module_unoise_globals,
};  
