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

#ifndef GX_WINDOW_H
#define GX_WINDOW_H

#include <gx/gx-drawable.h>

#include <glib-object.h>
#include <glib.h>

#include <xcb/xcb.h>

G_BEGIN_DECLS
#define GX_WINDOW(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_WINDOW, GXWindow))
#define GX_TYPE_WINDOW		  (gx_window_get_type())
#define GX_WINDOW_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GX_TYPE_WINDOW, GXWindowClass))
#define GX_IS_WINDOW(obj)	  (G_TYPE_CHECK_INSTANCE_Type ((obj), GX_TYPE_WINDOW))
#define GX_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_Type ((klass), GX_TYPE_WINDOW))
#define GX_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GX_TYPE_WINDOW, _GXWindowClass))

#ifndef GX_WINDOW_TYPEDEF
typedef struct _GXWindow GXWindow;
#define GX_WINDOW_TYPEDEF
#endif

typedef struct __GXWindowClass _GXWindowClass;
typedef struct _GXWindowPrivate GXWindowPrivate;

struct _GXWindow
{
  /* add your parent type here */
  GXDrawable parent;

  /* add pointers to new members here */

  /*< private > */
  GXWindowPrivate *priv;
};

struct __GXWindowClass
{
  /* add your parent class here */
  GXDrawableClass parent_class;

  /* add signals here */
/* #include "gx-window-signal-callbacks-gen.h" */

  void (* event) (GXConnection *object, GXGenericEvent *event);
};

GType gx_window_get_type (void);

/* add additional methods here */
GXWindow *
gx_window_new (GXConnection *connection,
	       GXWindow *parent,
	       guint16 x,
	       guint16 y,
	       guint16 width,
	       guint16 height,
	       guint32 event_mask);

xcb_window_t gx_window_get_xcb_window (GXWindow * self);

GXConnection *
gx_window_get_connection (GXWindow *self);


GXWindow *
_gx_window_find_from_xid (guint32 xid);

/* TODO - split this into seperate files */
#include <gx/gx-window-gen.h>

G_END_DECLS
#endif /* GX_WINDOW_H */

