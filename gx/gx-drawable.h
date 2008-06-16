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

#ifndef GX_DRAWABLE_H
#define GX_DRAWABLE_H

#include <gx/gx-connection.h>
#include <gx/gx-gcontext.h>
#include <gx/gx-types.h>

#include <glib.h>
#include <glib-object.h>

#include <xcb/xcb.h>

G_BEGIN_DECLS
#define GX_DRAWABLE(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_DRAWABLE, GXDrawable))
#define GX_TYPE_DRAWABLE		  (gx_drawable_get_type())
#define GX_DRAWABLE_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GX_TYPE_DRAWABLE, GXDrawableClass))
#define GX_IS_DRAWABLE(obj)	  (G_TYPE_CHECK_INSTANCE_Type ((obj), GX_TYPE_DRAWABLE))
#define GX_IS_DRAWABLE_CLASS(klass) (G_TYPE_CHECK_CLASS_Type ((klass), GX_TYPE_DRAWABLE))
#define GX_DRAWABLE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GX_TYPE_DRAWABLE, GXDrawableClass))

#ifndef GX_DRAWABLE_TYPEDEF
typedef struct _GXDrawable GXDrawable;
#define GX_DRAWABLE_TYPEDEF
#endif

typedef struct _GXDrawableClass GXDrawableClass;
typedef struct _GXDrawablePrivate GXDrawablePrivate;

struct _GXDrawable
{
  /* add your parent type here */
  GObject parent;

  /* add pointers to new members here */

  guint32 xid;

  /*< private > */
  GXDrawablePrivate *priv;
};

struct _GXDrawableClass
{
  /* add your parent class here */
  GObjectClass parent_class;

  /* add signals here */
  /* void (* signal) (GXDrawable *object); */
};

GType gx_drawable_get_type (void);

/* add additional methods here */
GXDrawable *gx_drawable_new (void);

guint32 gx_drawable_get_xid (GXDrawable * self);

/* TODO - split this into seperate files */
#include <gx/gx-drawable-gen.h>

G_END_DECLS
#endif /* GX_DRAWABLE_H */
