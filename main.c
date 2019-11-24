/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: acharlas <acharlas@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/10/27 17:02:26 by acharlas          #+#    #+#             */
/*   Updated: 2019/11/24 13:54:52 by acharlas         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "miniRT.h"

int		ray_intersect(const vect3f *orig, const vect3f *dir, float *t0, t_listobj *listobj)
{
	vect3f	l;
	float	tca;
	float	d2;
	float	thc;
	float	t1;

	l = v_minus(((t_sphere *)(listobj->data))->pos, *orig);
	tca = v_dot(l, *dir);
	d2 = v_dot(l, l) - (tca * tca);
	if (d2 > powf(((t_sphere *)(listobj->data))->r, 2))
		return (0);
	thc = sqrtf(powf(((t_sphere *)(listobj->data))->r, 2) - d2);
	*t0 = tca - thc;
	t1 = tca + thc;
	if (*t0 < 0)
		*t0 = t1;
	if (*t0 < 0)
		return (0);
	return (1);
}

vect3f reflect(vect3f *I, vect3f *N)
{
	vect3f out;
	out = v_minus(*I, v_mult(*N, (2.f * v_dot(*I, *N))));
    return (out);
}

int		scene_intersect(const vect3f *orig, vect3f *dir, t_listobj *listobj, vect3f *hit, vect3f *n, vect3f *color)
{
	float	spheres_dist = FLT_MAX;
	float	dist_i;

	while (listobj)
	{
		if (ray_intersect(orig, dir, &dist_i, listobj) && dist_i < spheres_dist)
		{
			*color = ((t_sphere *)(listobj->data))->color;
			spheres_dist = dist_i;
			*hit = v_plus(*orig, v_mult(*dir, dist_i));
			*n = normalize(v_minus(*hit, ((t_sphere *)(listobj->data))->pos));
		}
		listobj = listobj->next;
	}
	return (spheres_dist < 1000);
}

vect3f	cast_ray(vect3f orig, vect3f dir, t_listobj *listobj, t_listobj *listlight, size_t depth)
{
	float	diffuse_light_intensity;
	float	specular_light_intensity;
	vect3f	color;
	vect3f	point;
	vect3f	n;
	vect3f	light_dir;
	vect3f reflect_dir;
	vect3f reflect_orig;
	
	if (depth > 4 || !scene_intersect(&orig, &dir, listobj, &point, &n, &color))
		return (bg_color(164,202,255)); //color background
	
	diffuse_light_intensity = 0;
	specular_light_intensity = 0;
	reflect_dir = normalize(reflect(&dir, &n));
	reflect_orig = v_dot(reflect_dir, n) < 0 ? v_minus(point, v_mult(n, 0.001)) : v_plus(point, v_mult(n, 0.001));
	vect3f reflect_color = cast_ray(reflect_orig, reflect_dir, listobj, listlight, (depth + 1));
	while (listlight)
	{
		light_dir = normalize(v_minus(((t_light *)(listlight->data))->pos, point));
		float light_distance = norm(v_minus(((t_light *)(listlight->data))->pos, point));
		vect3f shadow_origi = v_dot(light_dir, n) < 0 ? v_minus(point, v_mult(n, 0.001)) : v_plus(point, v_mult(n, 0.001));
		vect3f shadow_pt = c_vect3f(0, 0, 0);
		vect3f shadow_N = c_vect3f(0, 0, 0);
		vect3f tmpmaterial;
		if (scene_intersect(&shadow_origi, &light_dir, listobj, &shadow_pt, &shadow_N, &tmpmaterial) && (norm(v_minus(shadow_pt, shadow_origi))) < light_distance)
		{
			listlight = listlight->next;
		 	continue;
		}
		diffuse_light_intensity += ((t_light *)(listlight->data))->intensity * maxf(0.f, v_dot(light_dir, n));
		specular_light_intensity += powf(maxf(0.f, v_dot(reflect(&light_dir, &n), dir)), 1425) * ((t_light *)(listlight->data))->intensity; // 50. = specular_light_exposant
		listlight = listlight->next;
	}
	color = v_plus(v_plus(v_mult(color, (diffuse_light_intensity * ((t_sphere *)(listobj->data))->albedo.x)), v_mult(c_vect3f(1, 1, 1),(specular_light_intensity * ((t_sphere *)(listobj->data))->albedo.y))),v_mult(reflect_color, ((t_sphere *)(listobj->data))->albedo.z)); // 0.6 = albedo[0] / 0.3 = albedo[1]
	color = verif_color(color);
	return (color); // color sphere
}

void	render(t_listobj *listobj, t_listobj *listlight, const int width, const int height, void *mlx, void *mlx_window)
{
	const int		fov = M_PI / 2; // int donc = 1 mais bientot float
	const vect3f	orig = c_vect3f(0, 0, 0);
	float			vue[2];
	vect3f			dir;
	vect3f			*framebuffer[width];
	int				color;
	
	for (size_t i = 0 ; i < width; i++)
		framebuffer[i] = malloc(sizeof(vect3f) * height);
	for (size_t i = 0; i < width; i++)
	{
		for (size_t j = 0; j < height; j++)
		{
			vue[0] =  (2 * (i + 0.5) / (float)width  - 1) * tan(fov / 2.) * width / (float)height;
			vue[1] = -(2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.);
			dir = normalize(c_vect3f(vue[0], vue[1], -1));
			framebuffer[i][j] = cast_ray(orig, dir, listobj, listlight, 0);
        }
    }
	for(size_t i = 0; i < width; i++)
	{
		vect3f *c = framebuffer[i];
			float max = maxf(c->x, maxf(c->y, c->z));
			if (max > 1)
				*c = v_mult(*c, 1.0/max);
		for(size_t j = 0; j < height; j++)
		{
			color = c_color(framebuffer[i][j]); 
			mlx_pixel_put(mlx, mlx_window, i, j, color);
        }
		free(framebuffer[i]);
    }
}

int		main(void)
{
	const int width = 1024;
	const int height = 768;
	t_listobj *listobj = NULL;
	t_listobj *listlight = NULL;
	
	t_sphere *sphere;
	t_sphere *sphere2;
	t_sphere *sphere3;
	t_sphere *sphere4;
	sphere = malloc(sizeof(t_sphere));
	sphere->pos = c_vect3f(-3, 0, -16);
	sphere->r = 2;
	sphere->color = c_vect3f(0.4, 0.4, 0.3); // bg_color(150, 11, 64);
	sphere->albedo = c_vect3f(0.6, 0.3, 0.1); // albedo[0] : diffuse light | albedo[1] : specular light | albedo[2] : reflect light
	ft_lstadd_front(&listobj, ft_lstnew(sphere));

	sphere2 = malloc(sizeof(t_sphere));
	sphere2->pos = c_vect3f(-1, -1.5, -12);
	sphere2->r = 2;
	sphere2->color = c_vect3f(1, 1, 1); // bg_color(255, 131, 220);
	sphere2->albedo = c_vect3f(0, 10.0, 0.8); // albedo[0] : diffuse light | albedo[1] : specular light | albedo[2] : reflect light
	ft_lstadd_front(&listobj, ft_lstnew(sphere2));

	sphere3 = malloc(sizeof(t_sphere));
	sphere3->pos = c_vect3f(1.5, -0.5, -18);
	sphere3->r = 3;
	sphere3->color = c_vect3f(0.3, 0.1, 0.1); // bg_color(210, 51, 108);
	sphere3->albedo = c_vect3f(0.9, 0.1, 0.0); // albedo[0] : diffuse light | albedo[1] : specular light | albedo[2] : reflect light
	ft_lstadd_front(&listobj, ft_lstnew(sphere3));

	sphere4 = malloc(sizeof(t_sphere));
	sphere4->pos = c_vect3f(7, 5, -18);
	sphere4->r = 4;
	sphere4->color = c_vect3f(1, 1, 1); // bg_color(210, 51, 108);
	sphere4->albedo = c_vect3f(0.9, 0.5, 0.5); // albedo[0] : diffuse light | albedo[1] : specular light | albedo[2] : reflect light
	ft_lstadd_front(&listobj, ft_lstnew(sphere4));

	t_light *light;
	light = malloc(sizeof(t_light));
	light->pos = c_vect3f(-20, 20, 20);
	light->intensity = 1.5;
	ft_lstadd_front(&listlight, ft_lstnew(light));

	t_light *light2;
	light2 = malloc(sizeof(t_light));
	light2->pos = c_vect3f(30, 50, -25);
	light2->intensity = 1.8;
	ft_lstadd_front(&listlight, ft_lstnew(light2)); 

	t_light *light3;
	light3 = malloc(sizeof(t_light));
	light3->pos = c_vect3f(30, 20, 30);
	light3->intensity = 1.7;
	ft_lstadd_front(&listlight, ft_lstnew(light3)); 
	void *mlx = mlx_init();
	void *mlx_window = mlx_new_window(mlx, width, height, "Image");
	render(listobj, listlight, width, height, mlx, mlx_window);
	mlx_loop(mlx);
}