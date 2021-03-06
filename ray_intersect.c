/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ray_intersect.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acharlas <acharlas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/01/21 20:28:57 by acharlas          #+#    #+#             */
/*   Updated: 2020/01/21 20:29:45 by acharlas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

bool	ray_intersect_sphere(const vect3f *orig, const vect3f *dir, float *t0, const t_sphere sphere)
{
	vect3f	l;
	float	tca;
	float	d2;
	float	thc;
	float	t1;

	l = v_minus(sphere.pos, *orig);
	tca = v_dot(l, *dir);
	if (tca < 0)
		return (0);
	d2 = v_dot(l, l) - (tca * tca);
	if (d2 > powf(sphere.r, 2))
		return (0);
	thc = sqrtf(powf(sphere.r, 2) - d2);
	*t0 = tca - thc;
	t1 = tca + thc;
	if (*t0 < 0)
		*t0 = t1;
	if (*t0 < 0)
		return (0);
	return (1);
}

bool	ray_intersect_plane(const vect3f *orig, const vect3f *dir, float *t0, const t_plane plane)
{	
	vect3f n = normalize(plane.rot);
	float denom = v_dot(*dir, n);
	vect3f polo = v_minus(plane.pos, *orig);
	*t0 = v_dot(polo, n) / denom;
	vect3f pt = v_plus(*orig,v_mult(*dir,*t0));
	if	(*t0 > 0)
		return (1);
	return (0);
}

bool	ray_intersect_cylinder(const vect3f *orig, const vect3f *dir, float *t0, const t_cylinder cylinder)
{
	vect3f v = normalize(cylinder.rot);
	vect3f x = v_minus(*orig, cylinder.pos);
	float a = v_dot(*dir, *dir) - powf(v_dot(*dir, v), 2);
	float b = 2 * (v_dot(*dir, x) - (v_dot(*dir,v) * v_dot(x,v)));
	float c = v_dot(x,x) - powf(v_dot(x,v), 2) - powf(cylinder.r, 2);

	float delta = b * b - 4 * a * c;
	if (delta < 0)
		return (0);
	float x1 = (-b + sqrtf(delta)) / (a * 2);
	float x2 = (-b - sqrtf(delta)) / (a * 2);
	if (x1 < x2 && x1 > 0)
	{
		*t0 = x1;
		float m = v_dot(*dir, normalize(cylinder.rot)) * *t0 + v_dot(v_minus(*orig, cylinder.pos),normalize(cylinder.rot));
		if (m < 0 || m > cylinder.h)
		{
			*t0 = x2;
			float m = v_dot(*dir, normalize(cylinder.rot)) * *t0 + v_dot(v_minus(*orig, cylinder.pos),normalize(cylinder.rot));
			if (m < 0 || m > cylinder.h)
				return (0);
			return (1);
		}
		return (1);
	}
	else if (x2 < x1 && x2 > 0)
	{
		*t0 = x2;
		float m = v_dot(*dir, normalize(cylinder.rot)) * *t0 + v_dot(v_minus(*orig, cylinder.pos),normalize(cylinder.rot));
		if (m < 0 || m > cylinder.h)
		{
			*t0 = x1;
			float m = v_dot(*dir, normalize(cylinder.rot)) * *t0 + v_dot(v_minus(*orig, cylinder.pos),normalize(cylinder.rot));
			if (m < 0 || m > cylinder.h)
				return (0);
			return (1);
		}
		return (1);
	}
	return (0);
}

bool	ray_intersect_cone(const vect3f *orig, const vect3f *dir, float *t0, const t_cone cone)
{
	vect3f v = normalize(cone.rot);
	float k = tan(((cone.a / 2) * M_PI) / 180);
	vect3f x = v_minus(*orig, cone.pos);

	float a = v_dot(*dir, *dir) - (1 + k * k) * powf(v_dot(*dir,v),2);
	float b = 2 * (v_dot(*dir,x) - (1 + k * k) * v_dot(*dir, v) * v_dot(x,v));
	float c = v_dot(x,x) - (1 + k * k) * powf(v_dot(x,v),2);
	float delta = b * b - 4 * a * c;
	if (delta < 0)
		return (0);
	float x1 = (-b + sqrtf(delta)) / (a * 2);
	float x2 = (-b - sqrtf(delta)) / (a * 2);
	if (x1 < x2 && x1 > 0)
	{
		*t0 = x1;
		return (1);
	}
	else if (x2 < x1 && x2 > 0)
	{
		*t0 = x2;
		return (1);
	}
	return (0);
}

bool	ray_intersect_triangle(const vect3f *orig, const vect3f *dir, float *t0, const t_triangle triangle)
{
	vect3f edge1 = v_minus(triangle.c3,triangle.c1);
	vect3f edge2 = v_minus(triangle.c2,triangle.c1);

	vect3f h = v_cross(*dir,edge1);
	float a = v_dot(h, edge2);
	if (ft_fabs(a) > 0.001)
	{
	float f = 1./a;
	vect3f s = v_minus(*orig, triangle.c1);
	float u = f * v_dot(s, h);
	if (u < 0.0 || u > 1.0)
		return (0);
	vect3f q = v_cross(s, edge2);
	float v = f * v_dot(*dir, q);
	if (v < 0.0 || u + v > 1.0)
    	return (0);
	*t0 = -f * v_dot(edge1, q);
	return (1);
	}
	return (0);
}