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

#include <gx/gx-screen.h>

#include <gx/gx-window.h>

/* Macros and defines */
#define GX_SCREEN_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_SCREEN, GXScreenPrivate))

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

struct _GXScreenPrivate
{
  GXWindow *root;
  GXColormap default_colormap;
  guint32 white_pixel;
  guint32 black_pixel;
  guint32 current_input_masks;
  guint16 width_in_pixels;
  guint16 height_in_pixels;
  guint16 width_in_millimeters;
  guint16 height_in_millimeters;
  guint16 min_installed_maps;
  guint16 max_installed_maps;
  GXVisualID root_visual;
  guint8 backing_stores;
  gboolean save_unders;
  guint8 root_depth;
  guint8 allowed_depths_len;
};


static void gx_screen_get_property (GObject * object,
				    guint id,
				    GValue * value, GParamSpec * pspec);
static void gx_screen_set_property (GObject * object,
				    guint property_id,
				    const GValue * value, GParamSpec * pspec);
/* static void gx_screen_mydoable_interface_init(gpointer interface,
   gpointer data); */
static void gx_screen_init (GXScreen * self);
static void gx_screen_finalize (GObject * self);


static GObjectClass *parent_class = NULL;
/* static guint gx_screen_signals[LAST_SIGNAL] = { 0 }; */

G_DEFINE_TYPE (GXScreen, gx_screen, G_TYPE_OBJECT);

static void
gx_screen_class_init (GXScreenClass * klass)	/* Class Initialization */
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  /* GParamSpec *new_param; */

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gx_screen_finalize;

  gobject_class->get_property = gx_screen_get_property;
  gobject_class->set_property = gx_screen_set_property;

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
  gx_screen_signals[SIGNAL_NAME] =
    g_signal_new ("signal_name", /* name */
		  G_TYPE_FROM_CLASS (klass), /* interface GType */
		  G_SIGNAL_RUN_LAST,	/* signal flags */
		  /* accumulator */
		  G_STRUCT_OFFSET (GXScreenClass, signal_member),
		  NULL,
		  NULL,	/* accumulator data */
		  g_cclosure_marshal_VOID__VOID, /* c marshaller */
		  G_TYPE_NONE, /* return type */
		  0 /* number of parameters */
		  /* vararg, list of param types */
    );
#endif

  g_type_class_add_private (klass, sizeof (GXScreenPrivate));
}

static void
gx_screen_get_property (GObject * object,
			guint id, GValue * value, GParamSpec * pspec)
{
  /* GXScreen* self = GX_SCREEN(object); */

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
gx_screen_set_property (GObject * object,
			guint property_id,
			const GValue * value, GParamSpec * pspec)
{
  /* GXScreen* self = GX_SCREEN(object); */

  switch (property_id)
    {
#if 0				/* template code */
    case PROP_NAME:
      gx_screen_set_property (self, g_value_get_int (value));
      break;
#endif
    default:
      g_warning ("gx_screen_set_property on unknown property");
      return;
    }
}

#if 0				/* template code */
static void
gx_screen_mydoable_interface_init (gpointer interface, gpointer data)
{
  MyDoableIface *mydoable = interface;
  g_assert (G_TYPE_FROM_INTERFACE (mydoable) == MY_TYPE_MYDOABLE);

  mydoable->method1 = gx_screen_method1;
  mydoable->method2 = gx_screen_method2;
}
#endif

/* Instance Construction */
static void
gx_screen_init (GXScreen * self)
{
  self->priv = GX_SCREEN_GET_PRIVATE (self);
  /* populate your object here */
}

/* Instantiation wrapper */
GXScreen *
gx_screen_new (void)
{
  return GX_SCREEN (g_object_new (gx_screen_get_type (), NULL));
}

/* Instance Destruction */
void
gx_screen_finalize (GObject * object)
{
  /* GXScreen *self = GX_SCREEN(object); */

  /* destruct your object here */
  G_OBJECT_CLASS (parent_class)->finalize (object);
}
