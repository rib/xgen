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

/* Macros and defines */
#define GX_DRAWABLE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_DRAWABLE, GXDrawablePrivate))

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
static void gx_drawable_init (GXDrawable * self);
static void gx_drawable_finalize (GObject * self);


/* Variables */
static GObjectClass *parent_class = NULL;
/* static guint gx_drawable_signals[LAST_SIGNAL] = { 0 }; */

GType
gx_drawable_get_type (void)	/* Typechecking */
{
  static GType self_type = 0;

  if (!self_type)
    {
      static const GTypeInfo object_info = {
	sizeof (GXDrawableClass),	/* class structure size */
	NULL,			/* base class initializer */
	NULL,			/* base class finalizer */
	(GClassInitFunc) gx_drawable_class_init,	/* class initializer */
	NULL,			/* class finalizer */
	NULL,			/* class data */
	sizeof (GXDrawable),	/* instance structure size */
	0,			/* preallocated instances */
	(GInstanceInitFunc) gx_drawable_init,	/* instance initializer */
	NULL			/* function table */
      };

      /* add the type of your parent class here */
      self_type = g_type_register_static (G_TYPE_OBJECT,	/* parent GType */
					  "GXDrawable",	/* type name */
					  &object_info,	/* type info */
					  0	/* flags */
	);
#if 0
      /* add interfaces here */
      static const GInterfaceInfo mydoable_info = {
	(GInterfaceInitFunc) gx_drawable_mydoable_interface_init,
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
gx_drawable_class_init (GXDrawableClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *new_param;

  parent_class = g_type_class_peek_parent (klass);

  gobject_class->finalize = gx_drawable_finalize;

  gobject_class->get_property = gx_drawable_get_property;
  gobject_class->set_property = gx_drawable_set_property;

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
				    MY_Type_PARAM_OBJ,	/* GType */
#elif PointER
				    /* nothing extra */
#endif
				    MY_PARAM_READABLE	/* flags */
				    MY_PARAM_WRITEABLE	/* flags */
				    MY_PARAM_READWRITE	/* flags */
				    | G_PARAM_CONSTRUCT
				    | G_PARAM_CONSTRUCT_ONLY);
  g_object_class_install_property (gobject_class, PROP_NAME, new_param);
#endif

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
#if 0				/* template code */
  klass->signal_member = signal_default_handler;
  gx_drawable_signals[SIGNAL_NAME] =
    g_signal_new ("signal_name",	/* name */
		  G_TYPE_FROM_CLASS (klass),	/* interface GType */
		  G_SIGNAL_RUN_LAST,	/* signal flags */
		  G_STRUCT_OFFSET (GXDrawableClass, signal_member), NULL, /* accumulator */
		  NULL,	/* accumulator data */
		  g_cclosure_marshal_VOID__VOID,	/* c marshaller */
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
      gx_drawable_set_property (self, g_value_get_uint (value));
      gx_drawable_set_property (self, g_value_get_boolean (value));
      gx_drawable_set_property (self, g_value_get_string (value));
      gx_drawable_set_property (self, g_value_get_object (value));
      gx_drawable_set_property (self, g_value_get_pointer (value));
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

/* Initialize interfaces here */

#if 0
static void
gx_drawable_mydoable_interface_init (gpointer interface, gpointer data)
{
  MyDoableIface *mydoable = interface;
  g_assert (G_TYPE_FROM_INTERFACE (mydoable) == MY_Type_MYDOABLE);

  mydoable->method1 = gx_drawable_method1;
  mydoable->method2 = gx_drawable_method2;
}
#endif

/* Instance Construction */
static void
gx_drawable_init (GXDrawable * self)
{
  self->priv = GX_DRAWABLE_GET_PRIVATE (self);
  /* populate your object here */
}

/* Instantiation wrapper */
GXDrawable *
gx_drawable_new (void)
{
  return GX_DRAWABLE (g_object_new (gx_drawable_get_type (), NULL));
}

/* Instance Destruction */
void
gx_drawable_finalize (GObject * object)
{
  /* GXDrawable *self = GX_DRAWABLE(object); */

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
For more gtk - doc notes, see:http://developer.gnome.org/arch/doc/authors.html
#endif
#if 0				/* getter/setter templates */
/**
 * gx_drawable_get_PROPERTY:
 * @self:  A GXDrawable.
 *
 * Fetches the PROPERTY of the GXDrawable. FIXME, add more info!
 *
 * Returns: The value of PROPERTY. FIXME, add more info!
 */
PROPType
gx_drawable_get_PROPERTY (GXDrawable * self)
{
  g_return_val_if_fail (GX_IS_DRAWABLE (self), /* FIXME */ );

  return self->priv->PROPERTY;
  return g_strdup (self->priv->PROPERTY);
  return g_object_ref (self->priv->PROPERTY);
}

/**
 * gx_drawable_set_PROPERTY:
 * @self:  A GXDrawable.
 * @property:  The value to set. FIXME, add more info!
 *
 * Sets this properties value.
 *
 * This will also clear the properties previous value.
 */
void
gx_drawable_set_PROPERTY (GXDrawable * self, PROPType PROPERTY)
{
  g_return_if_fail (GX_IS_DRAWABLE (self));

  if (self->priv->PROPERTY == PROPERTY)
    if (self->priv->PROPERTY == NULL
	|| strcmp (self->priv->PROPERTY, PROPERTY) != 0)
      {
	self->priv->PROPERTY = PROPERTY;
	g_free (self->priv->PROPERTY);
	self->priv->PROPERTY = g_strdup (PROPERTY);
	g_object_unref (self->priv->PROPERTY);
	self->priv->PROPERTY = g_object_ref (PROPERTY);
	g_object_notify (G_OBJECT (self), "PROPERTY");
      }
}
#endif

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

/* TODO - split this into seperate files */
#include "gx-drawable-gen.c"
