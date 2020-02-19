/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   trace.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vkaron <vkaron@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2019/11/21 21:30:06 by vkaron            #+#    #+#             */
/*   Updated: 2019/11/22 20:54:02 by vkaron           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "rtv1.h"

/*
** calculate intersection and return figure and distatnce of intersection
*/

t_isec	cls_isec(t_lst *lst, t_trc trc)
{
	t_fig	*cur_fig;
	t_vec3	t;
	t_isec	cisec;

	cisec.t = INFINITY;
	cisec.fig = NULL;
	cur_fig = lst->scn->figs;
	while (cur_fig)
	{
		sel_fig_check(&t, trc.o, trc.d, cur_fig);
		if (t.x >= trc.min && t.x <= trc.max && t.x < cisec.t)
		{
			cisec.t = t.x;
			cisec.fig = cur_fig;
		}
		if (t.y >= trc.min && t.y <= trc.max && t.y < cisec.t)
		{
			cisec.t = t.y;
			cisec.fig = cur_fig;
		}
		cur_fig = cur_fig->next;
	}
	return (cisec);
}

/*
** calculate normal for intersection dot of figure
*/

t_vec3	get_normal(t_isec *cisec, t_trc trc)
{
	t_vec3	n;
	t_fig	fig;
	float	m;
	t_vec3	vt;
	t_vec3	c;

	fig = *(cisec->fig);
	n.x = 0;
	if (fig.type == plane)
		n = set_vec3(fig.dir);
	else if (fig.type == sphere)
		n = minus_vec3(trc.p, fig.pos);
	else if ((fig.type == cylinder) || (fig.type == conus))
	{
		vt = mult_vec3f(fig.dir, cisec->t);
		c = set_vec3(fig.pos);
		m = dot(trc.d, vt) + dot(minus_vec3(trc.o, c), fig.dir);
		n = minus_vec3(minus_vec3(trc.p, c), mult_vec3f(fig.dir, m));
	}
	return (div_vec3f(n, len_vec3(n)));
}

/*
** calculate reflection color for dot
*/

t_col	get_refl_col(t_lst *lst, t_trc trc, t_vec3 n, int depth)
{
	int		r_col;
	t_col	refl_col;

	trc.min = MIN_OFFSET;
	trc.max = INFINITY;
	trc.d = set_vec3(refl_r(trc.d, n));
	r_col = trace(lst, trc, depth);
	refl_col.r = (r_col & 0xFF0000) >> 16;
	refl_col.g = (r_col & 0xFF00) >> 8;
	refl_col.b = r_col & 0xFF;
	return (refl_col);
}

/*
** set parametr for call light function
*/

t_l_prm	set_l_prm(t_trc trc, t_vec3 n)
{
	t_l_prm b;

	b.n = set_vec3(n);
	b.p = set_vec3(trc.p);
	b.v = set_vec3(trc.d);
	return (b);
}

/*
** ray trace function
*/

int		trace(t_lst *lst, t_trc trc, int depth)
{
	t_col	res;
	t_vec3	n;
	float	l;
	t_isec	cisec;
	t_col	refl_col;

	cisec = cls_isec(lst, trc);
	if (cisec.fig == NULL)
		return (lst->scn->bgc);
	trc.p = plus_vec3(mult_vec3f(trc.d, cisec.t), (trc.o));
	n = get_normal(&cisec, trc);
	trc.d = invert_vec3(trc.d);
	l = light(lst, set_l_prm(trc, n), cisec.fig);
	res.r = clamp(cisec.fig->col.r * l, 0, 255);
	res.g = clamp(cisec.fig->col.g * l, 0, 255);
	res.b = clamp(cisec.fig->col.b * l, 0, 255);
	if (depth <= 0 || cisec.fig->refl <= 0)
		return ((res.r << 16) + (res.g << 8) + res.b);
	trc.o = set_vec3(trc.p);
	refl_col = get_refl_col(lst, trc, n, depth - 1);
	res.r = res.r * (1 - cisec.fig->refl) + refl_col.r * cisec.fig->refl;
	res.g = res.g * (1 - cisec.fig->refl) + refl_col.g * cisec.fig->refl;
	res.b = res.b * (1 - cisec.fig->refl) + refl_col.b * cisec.fig->refl;
	return ((res.r << 16) + (res.g << 8) + res.b);
}
