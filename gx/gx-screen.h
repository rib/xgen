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

#ifndef GX_SCREEN_H
#define GX_SCREEN_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GX_SCREEN(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_SCREEN, GXScreen))
#define GX_TYPE_SCREEN		  (gx_screen_get_type())
#define GX_SCREEN_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GX_TYPE_SCREEN, GXScreenClass))
#define GX_IS_SCREEN(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GX_TYPE_SCREEN))
#define GX_IS_SCREEN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GX_TYPE_SCREEN))
#define GX_SCREEN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GX_TYPE_SCREEN, GXScreenClass))

typedef struct _GXScreen	GXScreen;
typedef struct _GXScreenClass	GXScreenClass;
typedef struct _GXScreenPrivate GXScreenPrivate;

struct _GXScreen
{
  /* add your parent type here */
  GObject parent;

  /* add pointers to new members here */

  /*< private > */
  GXScreenPrivate *priv;
};

struct _GXScreenClass
{
  /* add your parent class here */
  GObjectClass parent_class;

  /* add signals here */
  /* void (* signal) (GXScreen *object); */
};

GType gx_screen_get_type(void);

/* add additional methods here */
GXScreen *gx_screen_new(void);

G_END_DECLS

#endif /* GX_SCREEN_H */

