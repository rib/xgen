/*
 * <preamble>
 * Copyright (C) 2008  Robert Bragg
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

#ifndef GX_CONNECTION_H
#define GX_CONNECTION_H

#include <gx/gx-cookie.h>
#include <gx/gx-gcontext.h>
#include <gx/gx-screen.h>
#include <gx/gx-types.h>
#include <gx/gx-mask-value-item.h>

#include <glib.h>
#include <glib-object.h>

#include <gx/gx-xcb-dependencies-gen.h>

G_BEGIN_DECLS

#ifndef GX_DRAWABLE_TYPEDEF
typedef struct _GXDrawable          GXDrawable;
#define GX_DRAWABLE_TYPEDEF
#endif

#ifndef GX_PIXMAP_TYPEDEF
typedef struct _GXPixmap             GXPixmap;
#define GX_PIXMAP_TYPEDEF
#endif

#ifndef GX_WINDOW_TYPEDEF
typedef struct _GXWindow            GXWindow;
#define GX_WINDOW_TYPEDEF
#endif

#define GX_CONNECTION(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_CONNECTION, GXConnection))
#define GX_TYPE_CONNECTION            (gx_connection_get_type())
#define GX_CONNECTION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GX_TYPE_CONNECTION, GXConnectionClass))
#define GX_IS_CONNECTION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GX_TYPE_CONNECTION))
#define GX_IS_CONNECTION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GX_TYPE_CONNECTION))
#define GX_CONNECTION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GX_TYPE_CONNECTION, GXConnectionClass))

/* To handle circular dependency between the gx-connection and gx-cookie
 * headers */
#if !defined (GX_CONNECTION_TYPEDEF)
typedef struct _GXConnection GXConnection;
#define GX_CONNECTION_TYPEDEF
#endif
typedef struct _GXConnectionClass   GXConnectionClass;
typedef struct _GXConnectionPrivate GXConnectionPrivate;

struct _GXConnection
{
  GObject parent;

  /*< private > */
  GXConnectionPrivate *priv;
};

struct _GXConnectionClass
{
  GObjectClass parent_class;

  /* Signals */
  void (* event) (GXConnection *object, GXGenericEvent *event);
#if 0
  void (* reply) (GXConnection *object, GXCookie *cookie);
  void (* error) (GXConnection *object, GXCookie *cookie);
#endif
};

#if !defined (GX_COOKIE_TYPEDEF)
typedef struct _GXCookie GXCookie;
#define GX_COOKIE_TYPEDEF
#endif

GType gx_connection_get_type(void);

GXConnection *gx_connection_new (const char *display);

xcb_connection_t *
gx_connection_get_xcb_connection (GXConnection *connection);

void
gx_connection_flush (GXConnection *connection, gboolean flush_server);

gboolean
gx_connection_has_error (GXConnection *self);

void
gx_connection_register_cookie (GXConnection *self, GXCookie *cookie);
void
gx_connection_unregister_cookie (GXConnection *self, GXCookie *cookie);

GXScreen *gx_connection_get_default_screen (GXConnection *self);

GList *gx_connection_get_screens (GXConnection *self);

/* TODO - split this into seperate files */
#include <gx/gx-connection-gen.h>


G_END_DECLS

#endif /* GX_CONNECTION_H */

