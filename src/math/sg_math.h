#ifndef SG_MATH_H
#define SG_MATH_H 1

#include <stdint.h>

typedef float v2[2];
typedef float v3[3];
typedef float v4[4];
typedef v4 m4[4];

typedef uint32_t v2_i[2];
typedef uint32_t v3_i[3];
typedef uint32_t v4_i[4];

/* @brief: return randians of a degree */
float
deg_to_rad(const float deg);

/* @brief: return degree of randians */
float
rad_to_deg(const float rad);

/* @brief set the result of scaling v2src by scale to v2dst */
void
v2_scale_by(v2 v2dst, const float scale, const v2 v2src);

/* @brief: return a dot product of two vectors */
float
v3_dot(const v3 a, const v3 b);

/* @brief: get the magnitude (equclidian norm) of a vector */
float
v3_get_mag(const v3 v3src);

/* @brief: set value of v3src to 0 */
void
v3_to_zero(v3 v3src);

/* @brief: substract vector b from vector a and store in v3dst */
void
v3_sub(v3 v3dst, const v3 a, const v3 b);

/* @brief: add vector b and vector a and store in v3dst */
void
v3_add(v3 v3dst, const v3 a, const v3 b);

/* @brief: multiply components of vector b with components of vector a and store them in v3dst */
void
v3_mul(v3 v3dst, const v3 a, const v3 b);

/* @brief: set the result of scaling v3src by scale to v3dst */
void
v3_scale_by(v3 v3dst, const float scale, const v3 v3src);

/* @brief: get the cross product of a and b and store it in v3dst */
void
v3_cross(v3 v3dst, const v3 a, const v3 b);

/* @brief: normalize v3dst by a magnitude of v3src */
void
v3_normalize_to(v3 v3dst, const v3 v3src);

/* @brief: set m4src matrix elements to zero */
void
m4_to_zero(m4 m4src);

/* @brief: copy m4src matrix elements to m4dst */
void
m4_copy(m4 m4dst, const m4 m4src);

/* @brief: multiply m4first and m4second to m4dst */
void
m4_mul(m4 m4dst, const m4 m4a, const m4 m4b);

/* @brief: m4_mul with less multiplications */
void
m4_rotation_mul(m4 m4dst, const m4 m4a, const m4 m4b);

/* @brief create rotation matrix from angle and axis
 * axis will be normalized */
void
m4_rotation_make(m4 m4src, const float angle, const v3 axis);

void
rotate_angle(m4 m4src, float angle, v3 axis);

void
lookat(m4 m4src, const v3 v3eye /*where camera is*/,
       const v3 v3at /*where camera is pointing to*/, const v3 v3up);
void
perspective(m4 m4src, const float fov_y, const float aspect_ratio,
            const float nearVal, const float farVal);


#endif
