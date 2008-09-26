/*
 * vim: tabstop=8 shiftwidth=2 noexpandtab softtabstop=2 cinoptions=>2,{2,:0,t0,(0,W4
 *
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

#ifndef GX_COOKIE_H
#define GX_COOKIE_H

#include <glib.h>
#include <glib-object.h>

#include <gx/gx-connection.h>
#include <gx/gx-types.h>

G_BEGIN_DECLS

#define GX_COOKIE(obj)		  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GX_TYPE_COOKIE, GXCookie))
#define GX_TYPE_COOKIE		  (gx_cookie_get_type())
#define GX_COOKIE_CLASS(klass)	  (G_TYPE_CHECK_CLASS_CAST ((klass), GX_TYPE_COOKIE, GXCookieClass))
#define GX_IS_COOKIE(obj)	  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GX_TYPE_COOKIE))
#define GX_IS_COOKIE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GX_TYPE_COOKIE))
#define GX_COOKIE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), GX_TYPE_COOKIE, GXCookieClass))

#if !defined (GX_COOKIE_TYPEDEF)
typedef struct _GXCookie GXCookie;
#define GX_COOKIE_TYPEDEF
#endif
typedef struct _GXCookieClass	GXCookieClass;
typedef struct _GXCookiePrivate GXCookiePrivate;

struct _GXCookie
{
  GInitiallyUnowned parent;

  /*< private > */
  GXCookiePrivate *priv;
};

struct _GXCookieClass
{
  GInitiallyUnownedClass parent_class;

  /* Signals */
  void (* reply) (GXCookie *cookie);
  void (* error) (GXCookie *cookie);
};

/* To handle circular dependency between the gx-connection and gx-cookie
 * headers */
#if !defined (GX_CONNECTION_TYPEDEF)
typedef struct _GXConnection GXConnection;
#define GX_CONNECTION_TYPEDEF
#endif

GType gx_cookie_get_type(void);

#include "gx-cookie-gen.h"

GXCookie *gx_cookie_new (GXConnection *connection,
			 GXCookieType type,
			 unsigned int sequence);

GXConnection *gx_cookie_get_connection (GXCookie *self);

GXCookieType gx_cookie_get_cookie_type (GXCookie *self);

unsigned int gx_cookie_get_sequence (GXCookie *self);


GXGenericReply *
gx_cookie_get_reply (GXCookie *self);

void
gx_cookie_set_reply (GXCookie *self, xcb_generic_reply_t *reply);


GXGenericError *
gx_cookie_get_error (GXCookie *self);

void
gx_cookie_set_error (GXCookie *self, xcb_generic_error_t *error);

G_END_DECLS

#endif /* GX_COOKIE_H */

