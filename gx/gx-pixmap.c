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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 * </license>
 *
 */

#include <gx/gx-pixmap.h>
#include <gx/gx-connection.h>
#include <gx/gx-protocol-error.h>

#include <string.h>

/* Macros and defines */
#define GX_PIXMAP_GET_PRIVATE(object) \
    (G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_PIXMAP, GXPixmapPrivate))

#if 0
enum
{
  SIGNAL_NAME,
  LAST_SIGNAL
};
#endif

#if 0
enum
{
  PROP_0,
  PROP_NAME,
};
#endif

struct _GXPixmapPrivate
{
  guint padding0;
};


static void gx_pixmap_get_property (GObject * object,
				    guint id,
				    GValue * value, GParamSpec * pspec);
static void gx_pixmap_set_property (GObject * object,
				    guint property_id,
				    const GValue * value, GParamSpec * pspec);
/* static void gx_pixmap_mydoable_interface_init(gpointer interface,
   gpointer data); */
static void gx_pixmap_init (GXPixmap * self);
static void gx_pixmap_finalize (GObject * self);


/* Variables */
static GXDrawableClass *parent_class = NULL;
/* static guint gx_pixmap_signals[LAST_SIGNAL] = { 0 }; */

G_DEFINE_TYPE (GXPixmap, gx_pixmap, GX_TYPE_DRAWABLE);

static void
gx_pixmap_class_init (GXPixmapClass * klass)	/* Class Initialization */
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  /* GParamSpec *new_param; */

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gx_pixmap_finalize;

  gobject_class->get_property = gx_pixmap_get_property;
  gobject_class->set_property = gx_pixmap_set_property;

  /* set up properties */
#if 0
  //new_param = g_param_spec_int("name", /* name */
  //new_param = g_param_spec_uint("name", /* name */
  //new_param = g_param_spec_boolean("name", /* name */
  //new_param = g_param_spec_object("name", /* name */
  new_param = g_param_spec_pointer ("name",	/* name */
				    "Name",	/* nick name */
				    "Name",	/* description */
#if INT/UINT/CHAR/LONG/FLOAT...
				    10,	/* minimum */
				    100,	/* maximum */
				    0,	/* default */
#elif BOOLEAN
				    FALSE,	/* default */
#elif STRING
				    NULL,	/* default */
#elif OBJECT
				    MY_TYPE_PARAM_OBJ,	/* GType */
#elif POINTER
				    /* nothing extra */
#endif
				    MY_PARAM_READABLE	/* flags */
				    MY_PARAM_WRITEABLE	/* flags */
				    MY_PARAM_READWRITE	/* flags */
				    | G_PARAM_CONSTRUCT
				    | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_NAME, new_param);
#endif

  /* set up signals */
#if 0 /* template code */
  klass->signal_member = signal_default_handler;
  gx_pixmap_signals[SIGNAL_NAME] =
    g_signal_new ("signal_name", /* name */
		  G_TYPE_FROM_CLASS(klass), /* interface GType */
		  G_SIGNAL_RUN_LAST, /* signal flags */
		  G_STRUCT_OFFSET(GXPixmapClass, signal_member),
		  NULL, /* accumulator */
		  NULL, /* accumulator data */
		  g_cclosure_marshal_VOID__VOID, /* c marshaller */
		  G_TYPE_NONE, /* return type */
		  0 /* number of parameters */
		  /* vararg, list of param types */
    );
#endif

  g_type_class_add_private (klass, sizeof (GXPixmapPrivate));
}

static void
gx_pixmap_get_property (GObject * object,
			guint id, GValue * value, GParamSpec * pspec)
{
  /* GXPixmap* self = GX_PIXMAP(object); */

  switch (id)
    {
#if 0				/* template code */
    case PROP_NAME:
      g_value_set_int (value, self->priv->property);
      break;
#endif
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, id, pspec);
      break;
    }
}

static void
gx_pixmap_set_property (GObject * object,
			guint property_id,
			const GValue * value, GParamSpec * pspec)
{
  /* GXPixmap* self = GX_PIXMAP(object); */

  switch (property_id)
    {
#if 0				/* template code */
    case PROP_NAME:
      gx_pixmap_set_property (self, g_value_get_int (value));
      break;
#endif
    default:
      g_warning ("gx_pixmap_set_property on unknown property");
      return;
    }
}

#if 0				/* template code */
static void
gx_pixmap_mydoable_interface_init (gpointer interface, gpointer data)
{
  MyDoableIface *mydoable = interface;
  g_assert (G_TYPE_FROM_INTERFACE (mydoable) == MY_TYPE_MYDOABLE);

  mydoable->method1 = gx_pixmap_method1;
  mydoable->method2 = gx_pixmap_method2;
}
#endif

/* Instance Construction */
static void
gx_pixmap_init (GXPixmap * self)
{
  self->priv = GX_PIXMAP_GET_PRIVATE (self);
  /* populate object here */
}

/* Instantiation wrapper */
GXPixmap *
gx_pixmap_new (void)
{
  return GX_PIXMAP (g_object_new (GX_TYPE_PIXMAP, NULL));
}

/* Instance Destruction */
void
gx_pixmap_finalize (GObject * object)
{
  /* GXPixmap *self = GX_PIXMAP(object); */

  /* destruct your object here */
  G_OBJECT_CLASS (parent_class)->finalize (object);
}
