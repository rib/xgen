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

#include <gx/gx-screen.h>

#include <gx/gx-window.h>

/* Macros and defines */
#define GX_SCREEN_GET_PRIVATE(object) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_SCREEN, GXScreenPrivate))

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
  PROP_NUMBER,
  PROP_ROOT_WINDOW,
  PROP_DEFAULT_COLORMAP,
  PROP_BLACK,
  PROP_WHITE,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_WIDTH_IN_MILLIMETERS,
  PROP_HEIGHT_IN_MILLIMETERS,
  PROP_MIN_INSTALLED_MAPS,
  PROP_MAX_INSTALLED_MAPS,
  PROP_ROOT_VISUALID,
  PROP_BACKING_STORES,
  PROP_SAVE_UNDERS,
  PROP_ROOT_DEPTH,
};

struct _GXScreenPrivate
{
  guint number;
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
  GXVisualID root_visual_id;
  guint8 backing_stores;
  gboolean save_unders;
  guint8 root_depth;

  /* TODO: */
  GList *depths;
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


/* static guint gx_screen_signals[LAST_SIGNAL] = { 0 }; */

G_DEFINE_TYPE (GXScreen, gx_screen, G_TYPE_OBJECT);

static void
gx_screen_class_init (GXScreenClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *new_param;

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

  new_param = g_param_spec_uint ("number", /* name */
			         "The Screen Number", /* nick name */
			         "The screen number",
			         0, /* minimum */
			         G_MAXUINT32, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_NUMBER, new_param);

  new_param = g_param_spec_object ("root", /* name */
				   "Root Window", /* nick name */
				   "The root window",
				   GX_TYPE_WINDOW,
				   G_PARAM_READWRITE
				   | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_ROOT_WINDOW, new_param);

  new_param = g_param_spec_uint ("default-colormap", /* name */
			         "Default Colormap", /* nick name */
			         "the default colormap "
				 "associated with the root window",
			         0, /* minimum */
			         G_MAXUINT32, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_DEFAULT_COLORMAP, new_param);

  new_param = g_param_spec_uint ("black-pixel", /* name */
			         "Black Pixel", /* nick name */
			         "The default black pixel color",
			         0, /* minimum */
			         G_MAXUINT32, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_BLACK, new_param);

  new_param = g_param_spec_uint ("white-pixel", /* name */
			         "White Pixel", /* nick name */
			         "The default white pixel color",
			         0, /* minimum */
			         G_MAXUINT32, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_WHITE, new_param);

  new_param = g_param_spec_uint ("width", /* name */
			         "Width", /* nick name */
			         "The screen width",
			         0, /* minimum */
			         G_MAXUINT16, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_WIDTH, new_param);

  new_param = g_param_spec_uint ("height", /* name */
			         "Height", /* nick name */
			         "The screen height",
			         0, /* minimum */
			         G_MAXUINT16, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_HEIGHT, new_param);

  new_param = g_param_spec_uint ("width-in-millimeters", /* name */
			         "Hidth in millimeters", /* nick name */
			         "The screen width in millimeters",
			         0, /* minimum */
			         G_MAXUINT16, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_WIDTH_IN_MILLIMETERS, new_param);

  new_param = g_param_spec_uint ("height-in-millimeters", /* name */
			         "Height in millimeters", /* nick name */
			         "The screen height in millimeters",
			         0, /* minimum */
			         G_MAXUINT16, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_HEIGHT_IN_MILLIMETERS, new_param);

  new_param = g_param_spec_uint ("min-installed-maps", /* name */
			         "Minimal installed colormaps", /* nick name */
			         "The minimum number of installed "
				 "colormaps",
			         0, /* minimum */
			         G_MAXUINT16, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_MIN_INSTALLED_MAPS, new_param);

  new_param = g_param_spec_uint ("max-installed-maps", /* name */
			         "Maximum installed colormaps", /* nick name */
			         "The maximum number of installed "
				 "colormaps",
			         0, /* minimum */
			         G_MAXUINT16, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_MAX_INSTALLED_MAPS, new_param);

  new_param = g_param_spec_uint ("root-visual-id", /* name */
			         "Root Window VisualID", /* nick name */
			         "The root windows VisualID",
			         0, /* minimum */
			         G_MAXUINT32, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_ROOT_VISUALID, new_param);

  new_param = g_param_spec_boolean ("save-unders", /* name */
				    "Save Unders", /* nick name */
				    "TRUE of the screen supports save unders",
				    FALSE, /* default */
				    G_PARAM_READWRITE
				    | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_SAVE_UNDERS, new_param);

  new_param = g_param_spec_uint ("root-depth", /* name */
			         "Root Window Depth", /* nick name */
			         "The root windows depth",
			         0, /* minimum */
			         G_MAXUINT8, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_ROOT_DEPTH, new_param);

  new_param = g_param_spec_uint ("backing-stores", /* name */
			         "Backing Stores", /* nick name */
			         "Backing Stores",
			         0, /* minimum */
			         G_MAXUINT8, /* maximum */
			         0, /* default */
			         G_PARAM_READWRITE
			         | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class,
				   PROP_BACKING_STORES, new_param);

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
  GXScreen* self = GX_SCREEN(object);

  switch (id)
    {
    case PROP_NUMBER:
      g_value_set_uint (value, self->priv->number);
      break;
    case PROP_ROOT_WINDOW:
      g_value_set_object (value, self->priv->root);
      break;
    case PROP_DEFAULT_COLORMAP:
      g_value_set_uint (value, self->priv->default_colormap);
      break;
    case PROP_BLACK:
      g_value_set_uint (value, self->priv->black_pixel);
      break;
    case PROP_WHITE:
      g_value_set_uint (value, self->priv->white_pixel);
      break;
    case PROP_WIDTH:
      g_value_set_uint (value, self->priv->width_in_pixels);
      break;
    case PROP_HEIGHT:
      g_value_set_uint (value, self->priv->height_in_pixels);
      break;
    case PROP_MIN_INSTALLED_MAPS:
      g_value_set_uint (value, self->priv->min_installed_maps);
      break;
    case PROP_MAX_INSTALLED_MAPS:
      g_value_set_uint (value, self->priv->max_installed_maps);
      break;
    case PROP_WIDTH_IN_MILLIMETERS:
      g_value_set_uint (value, self->priv->width_in_millimeters);
      break;
    case PROP_HEIGHT_IN_MILLIMETERS:
      g_value_set_uint (value, self->priv->height_in_millimeters);
      break;
    case PROP_BACKING_STORES:
      g_value_set_uint (value, self->priv->backing_stores);
      break;
    case PROP_SAVE_UNDERS:
      g_value_set_boolean (value, self->priv->save_unders);
      break;
    case PROP_ROOT_DEPTH:
      g_value_set_uint (value, self->priv->root_depth);
      break;
    case PROP_ROOT_VISUALID:
      g_value_set_uint (value, self->priv->root_visual_id);
      break;
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
  GXScreen* self = GX_SCREEN(object);

  switch (property_id)
    {
    case PROP_NUMBER:
      self->priv->number = g_value_get_uint (value);
      break;
    case PROP_ROOT_WINDOW:
      self->priv->root = g_value_get_object (value);
      g_object_ref (self->priv->root);
      break;
    case PROP_DEFAULT_COLORMAP:
      self->priv->default_colormap = g_value_get_uint (value);
      break;
    case PROP_BLACK:
      self->priv->black_pixel = g_value_get_uint (value);
      break;
    case PROP_WHITE:
      self->priv->white_pixel = g_value_get_uint (value);
      break;
    case PROP_WIDTH:
      self->priv->width_in_pixels = g_value_get_uint (value);
      break;
    case PROP_HEIGHT:
      self->priv->height_in_pixels = g_value_get_uint (value);
      break;
    case PROP_WIDTH_IN_MILLIMETERS:
      self->priv->width_in_millimeters = g_value_get_uint (value);
      break;
    case PROP_HEIGHT_IN_MILLIMETERS:
      self->priv->height_in_millimeters = g_value_get_uint (value);
      break;
    case PROP_MIN_INSTALLED_MAPS:
      self->priv->min_installed_maps = g_value_get_uint (value);
      break;
    case PROP_MAX_INSTALLED_MAPS:
      self->priv->max_installed_maps = g_value_get_uint (value);
      break;
    case PROP_BACKING_STORES:
      self->priv->backing_stores = g_value_get_uint (value);
      break;
    case PROP_SAVE_UNDERS:
      self->priv->save_unders = g_value_get_boolean (value);
      break;
    case PROP_ROOT_DEPTH:
      self->priv->root_depth = g_value_get_uint (value);
      break;
    case PROP_ROOT_VISUALID:
      self->priv->root_visual_id = g_value_get_uint (value);
      break;
    default:
      g_warning ("gx_screen_set_property on unknown property");
      return;
    }
}

#if 0
static void
gx_screen_mydoable_interface_init (gpointer interface, gpointer data)
{
  MyDoableIface *mydoable = interface;
  g_assert (G_TYPE_FROM_INTERFACE (mydoable) == MY_TYPE_MYDOABLE);

  mydoable->method1 = gx_screen_method1;
  mydoable->method2 = gx_screen_method2;
}
#endif

static void
gx_screen_init (GXScreen * self)
{
  self->priv = GX_SCREEN_GET_PRIVATE (self);
  /* populate your object here */
}

GXScreen *
gx_screen_new (void)
{
  return GX_SCREEN (g_object_new (gx_screen_get_type (), NULL));
}

void
gx_screen_finalize (GObject * object)
{
  /* GXScreen *self = GX_SCREEN(object); */

  /* destruct your object here */
  G_OBJECT_CLASS (gx_screen_parent_class)->finalize (object);
}

guint
gx_screen_get_number (GXScreen *self)
{
  return self->priv->number;
}

GXWindow *
gx_screen_get_root_window (GXScreen *self)
{
  return g_object_ref (self->priv->root);
}

GXColormap *
gx_screen_get_default_colormap (GXScreen *self)
{
  return self->priv->default_colormap;
}

guint32
gx_screen_get_black_pixel (GXScreen *self)
{
  return self->priv->black_pixel;
}

guint32
gx_screen_get_white_pixel (GXScreen *self)
{
  return self->priv->white_pixel;
}

guint16
gx_screen_get_width (GXScreen *self)
{
  return self->priv->width_in_pixels;
}

guint16
gx_screen_get_height (GXScreen *self)
{
  return self->priv->height_in_pixels;
}

guint16
gx_screen_get_width_in_millimeters (GXScreen *self)
{
  return self->priv->width_in_millimeters;
}

guint16
gx_screen_get_height_in_millimeters (GXScreen *self)
{
  return self->priv->height_in_millimeters;
}

guint16
gx_screen_get_minimum_installed_maps (GXScreen *self)
{
  return self->priv->min_installed_maps;
}

guint16
gx_screen_get_maximum_installed_maps (GXScreen *self)
{
  return self->priv->max_installed_maps;
}

GXVisualID
gx_screen_get_root_visual_id (GXScreen *self)
{
  return self->priv->root_visual_id;
}

guint8
gx_screen_get_backing_stores (GXScreen *self)
{
  return self->priv->backing_stores;
}

gboolean
gx_screen_get_save_unders (GXScreen *self)
{
  return self->priv->save_unders;
}

guint8
gx_screen_get_root_depth (GXScreen *self)
{
  return self->priv->root_depth;
}

