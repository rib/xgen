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

#include "gx-drawable.h"
#include "gx-gcontext.h"

#include <xcb/xcb.h>
#include <string.h>

#define GX_DRAWABLE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_DRAWABLE, GXDrawablePrivate))

#if 0
enum
{
  SIGNAL_NAME,
  LAST_SIGNAL
};
#endif

enum
{
  PROP_0,
  PROP_CONNECTION,
  PROP_XID
};

struct _GXDrawablePrivate
{
    GXConnection *connection;

};


/* Function definitions */
static void gx_drawable_class_init (GXDrawableClass * klass);
static void gx_drawable_get_property (GObject * object,
				      guint id,
				      GValue * value, GParamSpec * pspec);
static void gx_drawable_set_property (GObject * object,
				      guint property_id,
				      const GValue * value,
				      GParamSpec * pspec);
/* static void gx_drawable_mydoable_interface_init(gpointer interface,
   gpointer data); */
static void gx_drawable_finalize (GObject * self);


/* Variables */
static GObjectClass *parent_class = NULL;
/* static guint gx_drawable_signals[LAST_SIGNAL] = { 0 }; */


G_DEFINE_TYPE(GXDrawable, gx_drawable, G_TYPE_OBJECT);


static void
gx_drawable_class_init (GXDrawableClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *new_param;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gx_drawable_finalize;

  gobject_class->get_property = gx_drawable_get_property;
  gobject_class->set_property = gx_drawable_set_property;

  new_param = g_param_spec_object("connection", /* name */
				  "Connection",	/* nick name */
				  "Connection",	/* description */
				  GX_TYPE_CONNECTION,	/* GType */
				  G_PARAM_READABLE	/* flags */
				  | G_PARAM_WRITABLE	/* flags */
				  | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_CONNECTION, new_param);

  new_param = g_param_spec_uint("xid", /* name */
			       "XID",	/* nick name */
			       "XID to send when creating a window",
			       0,	/* minimum */
			       G_MAXUINT32,	/* maximum */
			       0,	/* default */
			       G_PARAM_WRITABLE
			       | G_PARAM_READABLE
			       | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_XID, new_param);



  /* set up signals */
#if 0 /* template code */
  klass->signal_member = signal_default_handler;
  gx_drawable_signals[SIGNAL_NAME] =
    g_signal_new ("signal_name", /* name */
		  G_TYPE_FROM_CLASS (klass), /* interface GType */
		  G_SIGNAL_RUN_LAST, /* signal flags */
		  /* accumulator */
		  G_STRUCT_OFFSET (GXDrawableClass, signal_member), NULL,
		  NULL,	/* accumulator data */
		  g_cclosure_marshal_VOID__VOID, /* c marshaller */
		  G_TYPE_NONE,	/* return type */
		  0	/* number of parameters */
		  /* vararg, list of param types */
    );
#endif

  g_type_class_add_private (klass, sizeof (GXDrawablePrivate));
}

static void
gx_drawable_get_property (GObject * object,
			  guint id, GValue * value, GParamSpec * pspec)
{
  GXDrawable* self = GX_DRAWABLE(object);

  switch (id)
    {
#if 0 /* template code */
    case PROP_NAME:
      g_value_set_int (value, self->priv->property);
      break;
#endif
    case PROP_CONNECTION:
      g_value_set_object (value, self->priv->connection);
      break;
    case PROP_XID:
      g_value_set_uint (value, self->xid);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, id, pspec);
      break;
    }
}

static void
gx_drawable_set_property (GObject * object,
			  guint property_id,
			  const GValue * value, GParamSpec * pspec)
{
  GXDrawable* self = GX_DRAWABLE(object);

  switch (property_id)
    {
#if 0				/* template code */
    case PROP_NAME:
      gx_drawable_set_property (self, g_value_get_int (value));
      break;
#endif
    case PROP_CONNECTION:
      self->priv->connection = g_value_get_object (value);
      break;
    case PROP_XID:
      self->xid = g_value_get_uint (value);
      break;
    default:
      g_warning ("gx_drawable_set_property on unknown property");
      return;
    }
}

#if 0 /* template code */
static void
gx_drawable_mydoable_interface_init (gpointer interface, gpointer data)
{
  MyDoableIface *mydoable = interface;
  g_assert (G_TYPE_FROM_INTERFACE (mydoable) == MY_Type_MYDOABLE);

  mydoable->method1 = gx_drawable_method1;
  mydoable->method2 = gx_drawable_method2;
}
#endif

static void
gx_drawable_init (GXDrawable * self)
{
  self->priv = GX_DRAWABLE_GET_PRIVATE (self);
  /* populate object here */
}

GXDrawable *
gx_drawable_new (void)
{
  return GX_DRAWABLE (g_object_new (gx_drawable_get_type (), NULL));
}

void
gx_drawable_finalize (GObject * object)
{
  /* GXDrawable *self = GX_DRAWABLE(object); */

  /* destruct your object here */
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

guint32
gx_drawable_get_xid (GXDrawable * self)
{
  return self->xid;
}

GXConnection *
gx_drawable_get_connection (GXDrawable *self)
{
  return g_object_ref (self->priv->connection);
}

#include "gx-drawable-gen.c"

