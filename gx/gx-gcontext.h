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

#ifndef GX_GCONTEXT_H
#define GX_GCONTEXT_H

#include <glib.h>
#include <glib-object.h>

#include <gx/gx-xcb-dependencies-gen.h>

G_BEGIN_DECLS

#define GX_GCONTEXT(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_GCONTEXT, GXGContext))
#define GX_TYPE_GCONTEXT		  (gx_gcontext_get_type())
#define GX_GCONTEXT_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GX_TYPE_GCONTEXT, GXGContextClass))
#define GX_IS_GCONTEXT(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GX_TYPE_GCONTEXT))
#define GX_IS_GCONTEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GX_TYPE_GCONTEXT))
#define GX_GCONTEXT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GX_TYPE_GCONTEXT, GXGContextClass))

typedef struct _GXGContext	GXGContext;
typedef struct _GXGContextClass	GXGContextClass;
typedef struct _GXGContextPrivate GXGContextPrivate;

struct _GXGContext
{
  /* add your parent type here */
  GObject parent;

  /* add pointers to new members here */

  /*< private > */
  GXGContextPrivate *priv;
};

struct _GXGContextClass
{
  /* add your parent class here */
  GObjectClass parent_class;

  /* add signals here */
  /* void (* signal) (GXGContext *object); */
};

GType gx_gcontext_get_type(void);

/* add additional methods here */
GXGContext *gx_gcontext_new(void);

xcb_gcontext_t
gx_gc_get_xcb_gcontext (GXGContext *gc);

G_END_DECLS

#endif /* GX_GCONTEXT_H */

