/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ray_intersect.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acharlas <acharlas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/21 20:28:57 by acharlas          #+#    #+#             */
/*   Updated: 2020/02/07 12:32:14 by acharlas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

float	ray_intersect(t_ray ray, t_obj *obj)
{
	if (obj->type == 't')
		return (ray_intersect_triangle(ray.orig, ray.dir, *TRIANGLE));
	else if (obj->type == 's')
		return (ray_intersect_sphere(ray.orig, ray.dir, *SPHERE));
	else if (obj->type == 'p')
		return (ray_intersect_plane(ray.orig, ray.dir, *PLANE));
	else if (obj->type == 'c')
		return (ray_intersect_cylinder(ray.orig, ray.dir, *CYLINDER));
	else if (obj->type == 'k')
		return (ray_intersect_cone(ray.orig, ray.dir, *CONE));
	else
		return (FLT_MAX);
}

float	ray_intersect_sphere(const __m128 orig, const __m128 dir, const t_sphere sphere)
{
	__m128	l;
	float	tca;
	float	d2;
	float	thc;
	float	t1;
	float	t0;

	l = v_minus(sphere.pos, orig);
	tca = v_dot(l, dir);
	if (tca < 0)
		return (FLT_MAX);
	d2 = v_dot(l, l) - (tca * tca);
	if (d2 > powf(sphere.r, 2))
		return (FLT_MAX);
	thc = sqrtf(powf(sphere.r, 2) - d2);
	t0 = tca - thc;
	t1 = tca + thc;
	if (t0 < 0)
		t0 = t1;
	if (t0 < 0)
		return (FLT_MAX);
	return (t0);
}

float	ray_intersect_plane(const __m128 orig, const __m128 dir, const t_plane plane)
{	
	__m128 n = normalize(plane.rot);
	float denom = v_dot(dir, n);
	__m128 polo = v_minus(plane.pos, orig);
	float t0;

		t0 = v_dot(polo, n) / denom;
		if	(t0 > 0)
			return (t0);
	return (FLT_MAX);
}

float	ray_intersect_cylinder(const __m128 orig, const __m128 dir, const t_cylinder cylinder)
{
	__m128 v = normalize(cylinder.rot);
	__m128 x = v_minus(orig, cylinder.pos);
	float a = v_dot(dir, dir) - powf(v_dot(dir, v), 2);
	float b = 2 * (v_dot(dir, x) - (v_dot(dir,v) * v_dot(x,v)));
	float c = v_dot(x,x) - powf(v_dot(x,v), 2) - powf(cylinder.r, 2);
	float delta = b * b - 4 * a * c;
	float t0;

	if (delta < 0)
		return (FLT_MAX);
	float x1 = (-b + sqrtf(delta)) / (a * 2);
	float x2 = (-b - sqrtf(delta)) / (a * 2);
	if (x1 < x2 && x1 > 0)
	{
		t0 = x1;
		float m = v_dot(dir, normalize(cylinder.rot)) * t0 + v_dot(v_minus(orig, cylinder.pos),normalize(cylinder.rot));
		if (m < -cylinder.h / 2 || m > cylinder.h / 2)
		{
			t0 = x2;
			float m = v_dot(dir, normalize(cylinder.rot)) * t0 + v_dot(v_minus(orig, cylinder.pos),normalize(cylinder.rot));
			if (m < -cylinder.h / 2 || m > cylinder.h / 2)
				return (FLT_MAX);
			return (t0);
		}
		return (t0);
	}
	else if (x2 < x1 && x2 > 0)
	{
		t0 = x2;
		float m = v_dot(dir, normalize(cylinder.rot)) * t0 + v_dot(v_minus(orig, cylinder.pos),normalize(cylinder.rot));
		if (m < -cylinder.h / 2 || m > cylinder.h / 2)
		{
			t0 = x1;
			float m = v_dot(dir, normalize(cylinder.rot)) * t0 + v_dot(v_minus(orig, cylinder.pos),normalize(cylinder.rot));
			if (m < -cylinder.h / 2 || m > cylinder.h / 2)
				return (FLT_MAX);
			return (t0);
		}
		return (t0);
	}
	return (FLT_MAX);
}

float	ray_intersect_cone(const __m128 orig, const __m128 dir, const t_cone cone)
{
	__m128 v = normalize(cone.rot);
	float k = tan(((cone.a / 2) * M_PI) / 180);
	__m128 x = v_minus(orig, cone.pos);
	float t0;

	float a = v_dot(dir, dir) - (1 + k * k) * powf(v_dot(dir,v),2);
	float b = 2 * (v_dot(dir,x) - (1 + k * k) * v_dot(dir, v) * v_dot(x,v));
	float c = v_dot(x,x) - (1 + k * k) * powf(v_dot(x,v),2);
	float delta = b * b - 4 * a * c;
	if (delta < 0)
		return (FLT_MAX);
	float x1 = (-b + sqrtf(delta)) / (a * 2);
	float x2 = (-b - sqrtf(delta)) / (a * 2);
	if (x1 < x2)
	{
		t0 = x1;
		return (t0);
	}
	else if (x2 < x1 && x2 > 0)
	{
		t0 = x2;
		return (t0);
	}
	return (FLT_MAX);
}

float	ray_intersect_triangle(const __m128 orig, const __m128 dir, const t_triangle triangle)
{
	__m128 edge1 = v_minus(triangle.c3,triangle.c1);
	__m128 edge2 = v_minus(triangle.c2,triangle.c1);
	__m128 h = v_cross(dir,edge1);
	float a = v_dot(h, edge2);
	float t0;

	if (ft_fabs(a) > 0.001)
	{
	float f = 1./a;
	__m128 s = v_minus(orig, triangle.c1);
	float u = f * v_dot(s, h);
	if (u < 0.0 || u > 1.0)
		return (FLT_MAX);
	__m128 q = v_cross(s, edge2);
	float v = f * v_dot(dir, q);
	if (v < 0.0 || u + v > 1.0)
    	return (FLT_MAX);
	t0 = -f * v_dot(edge1, q);
	return (t0);
	}
	return (FLT_MAX);
}
