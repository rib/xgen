/*
 * vim: tabstop=8 shiftwidth=2 noexpandtab softtabstop=2 cinoptions=>2,{2,:0,t0,(0,W4
 *
 * <copyright_assignments>
 * Copyright (C) 2008  Robert Bragg
 * </copyright_assignments>
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

#ifndef GX_PIXMAP_H
#define GX_PIXMAP_H

#include "gx-drawable.h"

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GX_PIXMAP(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_PIXMAP, GXPixmap))
#define GX_TYPE_PIXMAP		  (gx_pixmap_get_type())
#define GX_PIXMAP_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GX_TYPE_PIXMAP, GXPixmapClass))
#define GX_IS_PIXMAP(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GX_TYPE_PIXMAP))
#define GX_IS_PIXMAP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GX_TYPE_PIXMAP))
#define GX_PIXMAP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GX_TYPE_PIXMAP, GXPixmapClass))

#ifndef GX_DRAWABLE_TYPEDEF
typedef struct _GXPixmap	GXPixmap;
#define GX_PIXMAP_TYPEDEF
#endif

typedef struct _GXPixmapClass	GXPixmapClass;
typedef struct _GXPixmapPrivate GXPixmapPrivate;

struct _GXPixmap
{
  /* add your parent type here */
  GXDrawable parent;

  /* add pointers to new members here */

  /*< private > */
  GXPixmapPrivate *priv;
};

struct _GXPixmapClass
{
  /* add your parent class here */
  GXDrawableClass parent_class;

  /* add signals here */
  /* void (* signal) (GXPixmap *object); */
};

GType gx_pixmap_get_type(void);

/* add additional methods here */
GXPixmap *gx_pixmap_new(void);

G_END_DECLS

#endif /* GX_PIXMAP_H */

