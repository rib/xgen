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

#include "gx-window.h"
#include "gx-connection.h"

#include <xcb/xcb.h>
#include <string.h>

/* Macros and defines */
#define GX_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_WINDOW, GXWindowPrivate))

/* Enums/Typedefs */
/* add your signals here */
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
  PROP_WRAP_XID,
  PROP_PARENT,
  PROP_X,
  PROP_Y,
  PROP_WIDTH,
  PROP_HEIGHT
};

struct _GXWindowPrivate
{
  /* Note: I think it will lead to trouble if we try and expose these
   * attributes as read/write properties, since I can't see a way to
   * auto generate such code.
   *
   * As the name suggests they are only used during window construction,
   * after which their values should be discarded. Like XCB we should
   * not try and provide any clever caching of state, instead we can
   * leave that to higher levels.
   */
  GXWindow *parent_construct;
  gboolean wrap_construct;
  gint16 x_construct;
  gint16 y_construct;
  guint16 width_construct;
  guint16 height_construct;
};


/* Function definitions */
static void gx_window_class_init (GXWindowClass * klass);
static void gx_window_get_property (GObject * object,
				    guint id,
				    GValue * value, GParamSpec * pspec);
static void gx_window_set_property (GObject * object,
				    guint property_id,
				    const GValue * value, GParamSpec * pspec);
/* static void gx_window_mydoable_interface_init(gpointer interface,
   gpointer data); */
static void gx_window_init (GXWindow * self);
static GObject *
gx_window_constructor (GType type,
		       guint n_construct_params,
		       GObjectConstructParam *construct_params);
static void gx_window_finalize (GObject * self);


/* Variables */
static GXDrawableClass *parent_class = NULL;
/* static guint gx_window_signals[LAST_SIGNAL] = { 0 }; */

GHashTable *xid_to_windows_map = NULL;

GType
gx_window_get_type (void)	/* Typechecking */
{
  static GType self_type = 0;

  if (!self_type)
    {
      static const GTypeInfo object_info = {
	  sizeof (GXWindowClass),	/* class structure size */
	  NULL,			/* base class initializer */
	  NULL,			/* base class finalizer */
	  (GClassInitFunc) gx_window_class_init,	/* class initializer */
	  NULL,			/* class finalizer */
	  NULL,			/* class data */
	  sizeof (GXWindow),	/* instance structure size */
	  0,			/* preallocated instances */
	  (GInstanceInitFunc) gx_window_init,	/* instance initializer */
	  NULL			/* function table */
      };

      /* add the type of your parent class here */
      self_type = g_type_register_static (GX_TYPE_DRAWABLE, /* parent GType */
					  "GXWindow",	/* type name */
					  &object_info,	/* type info */
					  0	/* flags */
      );
#if 0
      /* add interfaces here */
      static const GInterfaceInfo mydoable_info = {
	  (GInterfaceInitFunc) gx_window_mydoable_interface_init,
	  (GInterfaceFinalizeFunc) NULL,
	  NULL			/* interface data */
      };

      if (self_type != G_TYPE_INVALID)
	{
	  g_type_add_interface_static (self_type,
				       MY_Type_MYDOABLE, &mydoable_info);
	}
#endif
    }

  return self_type;
}

static void
gx_window_class_init (GXWindowClass * klass)	/* Class Initialization */
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *new_param;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gx_window_finalize;

  gobject_class->get_property = gx_window_get_property;
  gobject_class->set_property = gx_window_set_property;

  gobject_class->constructor = gx_window_constructor;

  /* FIXME - mutex */
  if (!xid_to_windows_map)
    {
      xid_to_windows_map = g_hash_table_new (g_int_hash, g_int_equal);
    }

  /* set up properties */
#if 0
  //new_param = g_param_spec_int("name", /* name */
  //new_param = g_param_spec_uint("name", /* name */
  //new_param = g_param_spec_boolean("name", /* name */
  //new_param = g_param_spec_object("name", /* name */
  new_param = g_param_spec_pointer ("name",	/* name */
				    "Name",	/* nick name */
				    "Name",	/* description */
#if INT/UINT/Char/LONG/FLOAT...
				    10,	/* minimum */
				    100,	/* maximum */
				    0,	/* default */
#elif BOOLEAN
				    FALSE,	/* default */
#elif STRING
				    NULL,	/* default */
#elif OBJECT
				    GX_TYPE_PARAM_OBJ,	/* GType */
#elif PointER
				    /* nothing extra */
#endif
				    G_PARAM_READABLE	/* flags */
				    G_PARAM_WRITABLE	/* flags */
				    | G_PARAM_CONSTRUCT
				    | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_NAME, new_param);
#endif


  new_param = g_param_spec_boolean("wrap", /* name */
				   "Wrap",	/* nick name */
				   "Simply wrap an already existing xid",
				   FALSE, /* default */
				   G_PARAM_WRITABLE
				   | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_WRAP_XID, new_param);

  new_param = g_param_spec_object("parent", /* name */
				  "Parent",	/* nick name */
				  "Parent window",	/* description */
				  GX_TYPE_WINDOW,	/* GType */
				  G_PARAM_WRITABLE	/* flags */
				  | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_PARENT, new_param);

  new_param = g_param_spec_int("x", /* name */
			       "X",	/* nick name */
			       "X Coordinate",	/* description */
			       G_MININT16,	/* minimum */
			       G_MAXINT16,	/* maximum */
			       0,	/* default */
			       G_PARAM_WRITABLE
			       | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_X, new_param);

  new_param = g_param_spec_int("y", /* name */
			       "Y",	/* nick name */
			       "Y Coordinate",	/* description */
			       G_MININT16,	/* minimum */
			       G_MAXINT16,	/* maximum */
			       0,		/* default */
			       G_PARAM_WRITABLE
			       | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_Y, new_param);

  new_param = g_param_spec_uint("width", /* name */
			       "Width",	/* nick name */
			       "Window width",	/* description */
			       1,	/* minimum */
			       G_MAXUINT16,	/* maximum */
			       1,	/* default */
			       G_PARAM_WRITABLE
			       | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_WIDTH, new_param);

  new_param = g_param_spec_uint("height", /* name */
			       "Height",	/* nick name */
			       "Window height",	/* description */
			       1,	/* minimum */
			       G_MAXUINT16,	/* maximum */
			       1,	/* default */
			       G_PARAM_WRITABLE
			       | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_HEIGHT, new_param);


  /* set up signals */
#if 0				/* template code */
  klass->signal_member = signal_default_handler;
  gx_window_signals[SIGNAL_NAME] =
    g_signal_new ("signal_name",	/* name */
		  G_TYPE_FROM_CLASS (klass),	/* interface GType */
		  G_SIGNAL_RUN_LAST,	/* signal flags */
		  G_STRUCT_OFFSET (GXWindowClass, signal_member), NULL,	/* accumulator */
		  NULL,	/* accumulator data */
		  g_cclosure_marshal_VOID__VOID,	/* c marshaller */
		  G_TYPE_NONE,	/* return type */
		  0	/* number of parameters */
		  /* vararg, list of param types */
    );
#endif

  g_type_class_add_private (klass, sizeof (GXWindowPrivate));
}

static void
gx_window_get_property (GObject * object,
			guint id, GValue * value, GParamSpec * pspec)
{
  GXWindow* self = GX_WINDOW(object);

  switch (id)
    {
#if 0				/* template code */
    case PROP_NAME:
      g_value_set_int (value, self->priv->property);
      g_value_set_uint (value, self->priv->property);
      g_value_set_boolean (value, self->priv->property);
      /* don't forget that this will dup the string for you: */
      g_value_set_string (value, self->priv->property);
      g_value_set_object (value, self->priv->property);
      g_value_set_pointer (value, self->priv->property);
      break;
#endif
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, id, pspec);
      break;
    }
}

static void
gx_window_set_property (GObject * object,
			guint property_id,
			const GValue * value, GParamSpec * pspec)
{
  GXWindow* self = GX_WINDOW(object);

  switch (property_id)
    {
    case PROP_WRAP_XID:
      self->priv->wrap_construct = g_value_get_boolean (value);
      break;
    case PROP_PARENT:
      self->priv->parent_construct = g_value_get_object (value);
      break;
    case PROP_X:
      self->priv->x_construct = g_value_get_int (value);
      break;
    case PROP_Y:
      self->priv->y_construct = g_value_get_int (value);
      break;
    case PROP_WIDTH:
      self->priv->width_construct = g_value_get_uint (value);
      break;
    case PROP_HEIGHT:
      self->priv->height_construct = g_value_get_uint (value);
      break;
    default:
      g_warning ("gx_window_set_property on unknown property");
      return;
    }
}

/* Initialize interfaces here */

#if 0
static void
gx_window_mydoable_interface_init (gpointer interface, gpointer data)
{
  MyDoableIface *mydoable = interface;
  g_assert (G_TYPE_FROM_INTERFACE (mydoable) == MY_Type_MYDOABLE);

  mydoable->method1 = gx_window_method1;
  mydoable->method2 = gx_window_method2;
}
#endif

/* Instance Construction */
static void
gx_window_init (GXWindow * self)
{
  self->priv = GX_WINDOW_GET_PRIVATE (self);
  /* populate your object here */
}

static GObject *
gx_window_constructor (GType type,
		       guint n_construct_params,
		       GObjectConstructParam *construct_params)
{
  GObject *object;
  GXWindow *self;
  GXDrawable *drawable;
  GXConnection *connection;
  xcb_connection_t *xcb_connection;
  guint32 xid;

  object = G_OBJECT_CLASS (parent_class)->constructor (type,
						       n_construct_params,
						       construct_params);
  self = GX_WINDOW (object);
  drawable = GX_DRAWABLE (self);

  connection = gx_drawable_get_connection (drawable);
  g_assert (connection);
  xcb_connection = gx_connection_get_xcb_connection (connection);

  xid = drawable->xid;
  if (!drawable->xid)
    {
      g_assert (!self->priv->wrap_construct);
      drawable->xid = xcb_generate_id (xcb_connection);
    }

  if (!self->priv->wrap_construct)
    {

      xcb_create_window (
			 xcb_connection,
			 0, /* depth */
			 drawable->xid,
			 gx_drawable_get_xid (self->priv->parent_construct),
			 self->priv->x_construct,
			 self->priv->y_construct,
			 self->priv->width_construct,
			 self->priv->height_construct,
			 0, /* border width */
			 0, /* class: copy from parent */
			 0, /* visual: copy from parent */
			 0, /* value mask */
			 NULL /* value list */
      );
    }

  return self;
}

/* Instantiation wrapper */
GXWindow *
gx_window_new (GXConnection *connection,
	       GXWindow *parent,
	       guint16 x,
	       guint16 y,
	       guint16 width,
	       guint16 height)
{
  return GX_WINDOW (g_object_new (gx_window_get_type (),
				  "connection", connection,
				  "parent", parent,
				  "x", x,
				  "y", y,
				  "width", width,
				  "height", height,
				  NULL));
#if 0
  return GX_WINDOW (g_object_new (gx_window_get_type (),
				  "connection", connection,
				  "depth", 0, /* Copy from parent */
				  "parent", parent,
				  "x", x,
				  "y", y,
				  "width", width,
				  "height", height,
				  "border_width", 0,
				  "class", 0, /* Copy from parent */
				  "visual", 0, /* Copy from parent */
				  "visual_mask", 0,
				  "value_list", NULL,
				  NULL));
#endif
}

/* Users could instead just use g_object_new, with full control over
 * the properties passed and thus full control over the windows
 * construction. */
#if 0
GXWindow *
gx_window_new_raw (GXConnection *connection,
		   guint8 depth,
		   GXWindow *parent,
		   guint16 x,
		   guint16 y,
		   guint16 width,
		   guint16 height,
		   guint16 border_width,
		   guint16 _class,
		   GXVisualID visual,
		   guint32 visual_mask,
		   const guint32 value_list)
{
  /* FIXME */
  g_assert (0);
}
#endif

/* Instance Destruction */
void
gx_window_finalize (GObject * object)
{
  /* GXWindow *self = GX_WINDOW(object); */

  /* destruct your object here */
  G_OBJECT_CLASS (parent_class)->finalize (object);
}



/* add new methods here */

/**
 * function_name:
 * @par1:  description of parameter 1. These can extend over more than
 * one line.
 * @par2:  description of parameter 2
 *
 * The function description goes here.
 *
 * Returns: an integer.
 */
#if 0
For more gtk - doc notes, see:http:	//developer.gnome.org/arch/doc/authors.html
#endif
#if 0				/* getter/setter templates */
/**
 * gx_window_get_PropERTY:
 * @self:  A GXWindow.
 *
 * Fetches the PropERTY of the GXWindow. FIXME, add more info!
 *
 * Returns: The value of PropERTY. FIXME, add more info!
 */
PropType
gx_window_get_PropERTY (GXWindow * self)
{
  g_return_val_if_fail (GX_IS_WINDOW (self), /* FIXME */ );

  return self->priv->PropERTY;
  return g_strdup (self->priv->PropERTY);
  return g_object_ref (self->priv->PropERTY);
}

/**
 * gx_window_set_PropERTY:
 * @self:  A GXWindow.
 * @property:  The value to set. FIXME, add more info!
 *
 * Sets this properties value.
 *
 * This will also clear the properties previous value.
 */
void
gx_window_set_PropERTY (GXWindow * self, PropType PropERTY)
{
  g_return_if_fail (GX_IS_WINDOW (self));

  if (self->priv->PropERTY == PropERTY)
    if (self->priv->PropERTY == NULL
	|| strcmp (self->priv->PropERTY, PropERTY) != 0)
      {
	self->priv->PropERTY = PropERTY;
	g_free (self->priv->PropERTY);
	self->priv->PropERTY = g_strdup (PropERTY);
	g_object_unref (self->priv->PropERTY);
	self->priv->PropERTY = g_object_ref (PropERTY);
	g_object_notify (G_OBJECT (self), "PropERTY");
      }
}
#endif

#if 0
xcb_window_t
gx_window_get_xcb_window (GXWindow * self)
{
  return self->priv->xcb_window;
}
#endif

GXConnection *
gx_window_get_connection (GXWindow *self)
{
  return gx_drawable_get_connection (GX_DRAWABLE (self));
}

/* Currently only for internal use, this does a lookup for an existing
 * GXWindow that corresponds to the passed xid. */
GXWindow *
_gx_window_find_from_xid (guint32 xid)
{
  /* FIXME - mutex */
  GXWindow *window = g_hash_table_lookup (xid_to_windows_map, &xid);
  return window ? g_object_ref (window) : NULL;
}

#include "gx-window-gen.c"

