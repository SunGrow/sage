#include "sg_math.h"

#include <math.h>

#ifndef M_PI
#define PI 3.1415926535
#else
#define PI M_PI
#endif

float deg_to_rad(const float deg) {
	return deg * PI / 180.0f;
}

float rad_to_deg(const float rad) {
	return rad * 180.0f / PI;
}

void v2_scale_by(v2 v2dst, const float scale, const v2 v2src) {
	v2dst[0] = v2src[0] * scale;
	v2dst[1] = v2src[1] * scale;
}

float v3_dot(const v3 a, const v3 b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float v3_get_mag(const v3 v3src) {
	return sqrtf(v3_dot(v3src, v3src));
}

void v3_scale_by(v3 v3dst, const float scale, const v3 v3src) {
	v3dst[0] = v3src[0] * scale;
	v3dst[1] = v3src[1] * scale;
	v3dst[2] = v3src[2] * scale;

	return;
}

void v3_sub(v3 v3dst, const v3 a, const v3 b) {
	v3dst[0] = a[0] - b[0];
	v3dst[1] = a[1] - b[1];
	v3dst[2] = a[2] - b[2];

	return;
}

void v3_add(v3 v3dst, const v3 a, const v3 b) {
	v3dst[0] = a[0] + b[0];
	v3dst[1] = a[1] + b[1];
	v3dst[2] = a[2] + b[2];

	return;
}

void v3_mul(v3 v3dst, const v3 a, const v3 b) {
	v3dst[0] = a[0] * b[0];
	v3dst[1] = a[1] * b[1];
	v3dst[2] = a[2] * b[2];

	return;
}

void v3_to_zero(v3 v3src) {
	v3src[0] = 0.0f;
	v3src[1] = 0.0f;
	v3src[2] = 0.0f;

	return;
}

void v3_cross(v3 v3dst, const v3 a, const v3 b) {
	v3dst[0] = a[1] * b[2] - a[2] * b[1];
	v3dst[1] = a[2] * b[0] - a[0] * b[2];
	v3dst[2] = a[0] * b[1] - a[1] * b[0];

	return;
}

void v3_normalize_to(v3 v3dst, const v3 v3src) {
	float mag;
	mag      = v3_get_mag(v3src);
	int a    = (mag ? 1.0f : 0);
	int b    = (mag ? 0 : 1.0f);
	v3dst[0] = v3src[0] * a / mag + b;
	v3dst[1] = v3src[1] * a / mag + b;
	v3dst[2] = v3src[2] * a / mag + b;
	return;
}

void m4_to_zero(m4 m4src) {
	m4 m4tmp = {0};
	m4_copy(m4src, m4tmp);
	return;
}

void m4_copy(m4 m4dst, const m4 m4src) {
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			m4dst[i][j] = m4src[i][j];
	return;
}

void m4_rotation_make(m4 m4src, const float angle, const v3 axis) {
	v3 axis_norm, v, vs;
	float cosin;

	cosin = cosf(angle);
	v3_normalize_to(axis_norm, axis);
	v3_scale_by(v, 1.0f - cosin, axis_norm);
	v3_scale_by(vs, sinf(angle), axis_norm);

	v3_scale_by(m4src[0], v[0], axis_norm);
	v3_scale_by(m4src[1], v[1], axis_norm);
	v3_scale_by(m4src[2], v[2], axis_norm);

	m4src[0][0] += cosin;
	m4src[1][0] -= vs[2];
	m4src[2][0] += vs[1];
	m4src[0][1] += vs[2];
	m4src[1][1] += cosin;
	m4src[2][1] -= vs[0];
	m4src[0][2] -= vs[1];
	m4src[1][2] += vs[0];
	m4src[2][2] += cosin;

	m4src[0][3] = m4src[1][3] = m4src[2][3] = m4src[3][0] = m4src[3][1] =
	    m4src[3][2]                                       = 0.0f;
	m4src[3][3]                                           = 1.0f;

	return;
}

void m4_mul(m4 m4dst, const m4 m4a, const m4 m4b) {
	float a00 = m4a[0][0], a01 = m4a[0][1], a02 = m4a[0][2], a03 = m4a[0][3],
	      a10 = m4a[1][0], a11 = m4a[1][1], a12 = m4a[1][2], a13 = m4a[1][3],
	      a20 = m4a[2][0], a21 = m4a[2][1], a22 = m4a[2][2], a23 = m4a[2][3],
	      a30 = m4a[3][0], a31 = m4a[3][1], a32 = m4a[3][2], a33 = m4a[3][3],

	      b00 = m4b[0][0], b01 = m4b[0][1], b02 = m4b[0][2], b03 = m4b[0][3],
	      b10 = m4b[1][0], b11 = m4b[1][1], b12 = m4b[1][2], b13 = m4b[1][3],
	      b20 = m4b[2][0], b21 = m4b[2][1], b22 = m4b[2][2], b23 = m4b[2][3],
	      b30 = m4b[3][0], b31 = m4b[3][1], b32 = m4b[3][2], b33 = m4b[3][3];

	m4dst[0][0] = a00 * b00 + a10 * b01 + a20 * b02 + a30 * b03;
	m4dst[0][1] = a01 * b00 + a11 * b01 + a21 * b02 + a31 * b03;
	m4dst[0][2] = a02 * b00 + a12 * b01 + a22 * b02 + a32 * b03;
	m4dst[0][3] = a03 * b00 + a13 * b01 + a23 * b02 + a33 * b03;
	m4dst[1][0] = a00 * b10 + a10 * b11 + a20 * b12 + a30 * b13;
	m4dst[1][1] = a01 * b10 + a11 * b11 + a21 * b12 + a31 * b13;
	m4dst[1][2] = a02 * b10 + a12 * b11 + a22 * b12 + a32 * b13;
	m4dst[1][3] = a03 * b10 + a13 * b11 + a23 * b12 + a33 * b13;
	m4dst[2][0] = a00 * b20 + a10 * b21 + a20 * b22 + a30 * b23;
	m4dst[2][1] = a01 * b20 + a11 * b21 + a21 * b22 + a31 * b23;
	m4dst[2][2] = a02 * b20 + a12 * b21 + a22 * b22 + a32 * b23;
	m4dst[2][3] = a03 * b20 + a13 * b21 + a23 * b22 + a33 * b23;
	m4dst[3][0] = a00 * b30 + a10 * b31 + a20 * b32 + a30 * b33;
	m4dst[3][1] = a01 * b30 + a11 * b31 + a21 * b32 + a31 * b33;
	m4dst[3][2] = a02 * b30 + a12 * b31 + a22 * b32 + a32 * b33;
	m4dst[3][3] = a03 * b30 + a13 * b31 + a23 * b32 + a33 * b33;

	return;
}

void m4_rotation_mul(m4 m4dst, const m4 m4a, const m4 m4b) {
	float a00 = m4a[0][0], a01 = m4a[0][1], a02 = m4a[0][2], a03 = m4a[0][3],
	      a10 = m4a[1][0], a11 = m4a[1][1], a12 = m4a[1][2], a13 = m4a[1][3],
	      a20 = m4a[2][0], a21 = m4a[2][1], a22 = m4a[2][2], a23 = m4a[2][3],
	      a30 = m4a[3][0], a31 = m4a[3][1], a32 = m4a[3][2], a33 = m4a[3][3],

	      b00 = m4b[0][0], b01 = m4b[0][1], b02 = m4b[0][2], b10 = m4b[1][0],
	      b11 = m4b[1][1], b12 = m4b[1][2], b20 = m4b[2][0], b21 = m4b[2][1],
	      b22 = m4b[2][2];

	m4dst[0][0] = a00 * b00 + a10 * b01 + a20 * b02;
	m4dst[0][1] = a01 * b00 + a11 * b01 + a21 * b02;
	m4dst[0][2] = a02 * b00 + a12 * b01 + a22 * b02;
	m4dst[0][3] = a03 * b00 + a13 * b01 + a23 * b02;

	m4dst[1][0] = a00 * b10 + a10 * b11 + a20 * b12;
	m4dst[1][1] = a01 * b10 + a11 * b11 + a21 * b12;
	m4dst[1][2] = a02 * b10 + a12 * b11 + a22 * b12;
	m4dst[1][3] = a03 * b10 + a13 * b11 + a23 * b12;

	m4dst[2][0] = a00 * b20 + a10 * b21 + a20 * b22;
	m4dst[2][1] = a01 * b20 + a11 * b21 + a21 * b22;
	m4dst[2][2] = a02 * b20 + a12 * b21 + a22 * b22;
	m4dst[2][3] = a03 * b20 + a13 * b21 + a23 * b22;

	m4dst[3][0] = a30;
	m4dst[3][1] = a31;
	m4dst[3][2] = a32;
	m4dst[3][3] = a33;

	return;
}

void rotate_angle(m4 m4src, float angle, v3 axis) {
	m4 m4rotation;
	m4_rotation_make(m4rotation, angle, axis);
	m4_rotation_mul(m4src, m4rotation, m4src);
	return;
}

void lookat(m4 m4src, const v3 v3eye, const v3 v3at, const v3 v3up) {
	v3 v3z_base, v3y_base, v3x_base;
	/* Create basis vector opposite to basis vector z by first substracting
	   camera position from the vector of a position to look at */
	v3_sub(v3z_base, v3at, v3eye);
	/* And then normalizing it */
	v3_normalize_to(v3z_base, v3z_base);
	/* Now we get a normalized vector pointing to where we look at. */

	/* Get an x vector (pointing right) */
	v3_cross(v3x_base, v3z_base, v3up);
	v3_normalize_to(v3x_base, v3x_base);
	/* Get a y vector (pointing up) */
	v3_cross(v3y_base, v3x_base, v3z_base);

	m4src[0][0] = v3x_base[0];
	m4src[0][1] = v3y_base[0];
	m4src[0][2] = -v3z_base[0];

	m4src[1][0] = v3x_base[1];
	m4src[1][1] = v3y_base[1];
	m4src[1][2] = -v3z_base[1];

	m4src[2][0] = v3x_base[2];
	m4src[2][1] = v3y_base[2];
	m4src[2][2] = -v3z_base[2];

	m4src[3][0] = -v3_dot(v3eye, v3x_base);
	m4src[3][1] = -v3_dot(v3eye, v3y_base);
	m4src[3][2] = v3_dot(v3eye, v3z_base);

	m4src[0][3] = m4src[1][3] = m4src[2][3] = 0.0f;
	m4src[3][3]                             = 1.0f;

	return;
}

void perspective(m4 m4src,
                 const float fov_y,
                 const float aspect_ratio,
                 const float nearVal,
                 const float farVal) {
	float f, fn;

	m4_to_zero(m4src);

	f  = 1.0f / tanf(fov_y * 0.5f);
	fn = 1.0f / (nearVal - farVal);

	m4src[0][0] = f / aspect_ratio;
	m4src[1][1] = -f;
	m4src[2][2] = (nearVal + farVal) * fn;
	m4src[2][3] = -1.0f;
	m4src[3][2] = 2.0f * nearVal * farVal * fn;

	return;
}
