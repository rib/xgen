/*
 * vim: tabstop=8 shiftwidth=2 noexpandtab softtabstop=2 cinoptions=>2,{2,:0,t0,(0,W4
 *
 * <preamble>
 * gswat - A graphical program debugger for Gnome
 * Copyright (C) 2006  Robert Bragg
 * </preamble>
 *
 * <license>
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * </license>
 *
 */

#ifndef GX_COOKIE_H
#define GX_COOKIE_H

#include <glib.h>
#include <glib-object.h>
/* include your parent object here */

G_BEGIN_DECLS

#define GX_COOKIE(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_COOKIE, GXCookie))
#define GX_TYPE_COOKIE		  (gx_cookie_get_type())
#define GX_COOKIE_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GX_TYPE_COOKIE, GXCookieClass))
#define GX_IS_COOKIE(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GX_TYPE_COOKIE))
#define GX_IS_COOKIE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GX_TYPE_COOKIE))
#define GX_COOKIE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GX_TYPE_COOKIE, GXCookieClass))

typedef struct _GXCookie	GXCookie;
typedef struct _GXCookieClass	GXCookieClass;
typedef struct _GXCookiePrivate GXCookiePrivate;

struct _GXCookie
{
  /* add your parent type here */
  GObject parent;

  /* add pointers to new members here */

  /*< private > */
  GXCookiePrivate *priv;
};

struct _GXCookieClass
{
  /* add your parent class here */
  GObjectClass parent_class;

  /* add signals here */
  /* void (* signal) (GXCookie *object); */
};

GType gx_cookie_get_type(void);

/* add additional methods here */
GXCookie *gx_cookie_new(void);

G_END_DECLS

#endif /* GX_COOKIE_H */

