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

#include "gx-gcontext.h"

#include <xcb/xcb.h>
#include <string.h>

/* Macros and defines */
#define GX_GCONTEXT_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_GCONTEXT, GXGContextPrivate))

/* Enums/Typedefs */
/* add your signals here */
#if 0
enum {
    SIGNAL_NAME,
    LAST_SIGNAL
};
#endif

#if 0
enum {
    PROP_0,
    PROP_NAME,
};
#endif

struct _GXGContextPrivate
{
    xcb_gcontext_t  xcb_gcontext;
};


/* Function definitions */
static void gx_gcontext_class_init(GXGContextClass *klass);
static void gx_gcontext_get_property(GObject *object,
				   guint id,
				   GValue *value,
				   GParamSpec *pspec);
static void gx_gcontext_set_property(GObject *object,
				   guint property_id,
				   const GValue *value,
				   GParamSpec *pspec);
/* static void gx_gcontext_mydoable_interface_init(gpointer interface,
   gpointer data); */
static void gx_gcontext_init(GXGContext *self);
static void gx_gcontext_finalize(GObject *self);


/* Variables */
static GObjectClass *parent_class = NULL;
/* static guint gx_gcontext_signals[LAST_SIGNAL] = { 0 }; */

GType
gx_gcontext_get_type(void) /* Typechecking */
{
  static GType self_type = 0;

  if (!self_type)
    {
      static const GTypeInfo object_info =
	{
	  sizeof(GXGContextClass), /* class structure size */
	  NULL, /* base class initializer */
	  NULL, /* base class finalizer */
	  (GClassInitFunc)gx_gcontext_class_init, /* class initializer */
	  NULL, /* class finalizer */
	  NULL, /* class data */
	  sizeof(GXGContext), /* instance structure size */
	  0, /* preallocated instances */
	  (GInstanceInitFunc)gx_gcontext_init, /* instance initializer */
	  NULL /* function table */
	};

      /* add the type of your parent class here */
      self_type = g_type_register_static(G_TYPE_OBJECT, /* parent GType */
					 "GXGContext", /* type name */
					 &object_info, /* type info */
					 0 /* flags */
      );
#if 0
      /* add interfaces here */
      static const GInterfaceInfo mydoable_info =
	{
	  (GInterfaceInitFunc)
	    gx_gcontext_mydoable_interface_init,
	  (GInterfaceFinalizeFunc)NULL,
	  NULL /* interface data */
	};

      if(self_type != G_TYPE_INVALID) {
	  g_type_add_interface_static(self_type,
				      MY_TYPE_MYDOABLE,
				      &mydoable_info);
      }
#endif
    }

  return self_type;
}

static void
gx_gcontext_class_init(GXGContextClass *klass) /* Class Initialization */
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  /* GParamSpec *new_param;*/

  parent_class = g_type_class_peek_parent(klass);

  gobject_class->finalize = gx_gcontext_finalize;

  gobject_class->get_property = gx_gcontext_get_property;
  gobject_class->set_property = gx_gcontext_set_property;

  /* set up properties */
#if 0
  //new_param = g_param_spec_int("name", /* name */
  //new_param = g_param_spec_uint("name", /* name */
  //new_param = g_param_spec_boolean("name", /* name */
  //new_param = g_param_spec_object("name", /* name */
  new_param = g_param_spec_pointer("name", /* name */
				   "Name", /* nick name */
				   "Name", /* description */
#if INT/UINT/CHAR/LONG/FLOAT...
				   10, /* minimum */
				   100, /* maximum */
				   0, /* default */
#elif BOOLEAN
				   FALSE, /* default */
#elif STRING
				   NULL, /* default */
#elif OBJECT
				   MY_TYPE_PARAM_OBJ, /* GType */
#elif POINTER
				   /* nothing extra */
#endif
				   MY_PARAM_READABLE /* flags */
				   MY_PARAM_WRITEABLE /* flags */
				   MY_PARAM_READWRITE /* flags */
				   | G_PARAM_CONSTRUCT
				   | G_PARAM_CONSTRUCT_ONLY
				   );
  g_object_class_install_property(gobject_class,
				  PROP_NAME,
				  new_param);
#endif

  /* set up signals */
#if 0 /* template code */
  klass->signal_member = signal_default_handler;
  gx_gcontext_signals[SIGNAL_NAME] =
    g_signal_new("signal_name", /* name */
		 G_TYPE_FROM_CLASS(klass), /* interface GType */
		 G_SIGNAL_RUN_LAST, /* signal flags */
		 G_STRUCT_OFFSET(GXGContextClass, signal_member),
		 NULL, /* accumulator */
		 NULL, /* accumulator data */
		 g_cclosure_marshal_VOID__VOID, /* c marshaller */
		 G_TYPE_NONE, /* return type */
		 0 /* number of parameters */
		 /* vararg, list of param types */
    );
#endif

  g_type_class_add_private(klass, sizeof(GXGContextPrivate));
}

static void
gx_gcontext_get_property(GObject *object,
		       guint id,
		       GValue *value,
		       GParamSpec *pspec)
{
  /* GXGContext* self = GX_GCONTEXT(object); */

  switch(id)
    {
#if 0 /* template code */
    case PROP_NAME:
      g_value_set_int(value, self->priv->property);
      g_value_set_uint(value, self->priv->property);
      g_value_set_boolean(value, self->priv->property);
      /* don't forget that this will dup the string for you: */
      g_value_set_string(value, self->priv->property);
      g_value_set_object(value, self->priv->property);
      g_value_set_pointer(value, self->priv->property);
      break;
#endif
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, id, pspec);
      break;
    }
}

static void
gx_gcontext_set_property(GObject *object,
		       guint property_id,
		       const GValue *value,
		       GParamSpec *pspec)
{
  /* GXGContext* self = GX_GCONTEXT(object); */

  switch(property_id)
    {
#if 0 /* template code */
    case PROP_NAME:
      gx_gcontext_set_property(self, g_value_get_int(value));
      gx_gcontext_set_property(self, g_value_get_uint(value));
      gx_gcontext_set_property(self, g_value_get_boolean(value));
      gx_gcontext_set_property(self, g_value_get_string(value));
      gx_gcontext_set_property(self, g_value_get_object(value));
      gx_gcontext_set_property(self, g_value_get_pointer(value));
      break;
#endif
    default:
      g_warning("gx_gcontext_set_property on unknown property");
      return;
    }
}

/* Initialize interfaces here */

#if 0
static void
gx_gcontext_mydoable_interface_init(gpointer interface,
				  gpointer data)
{
  MyDoableIface *mydoable = interface;
  g_assert(G_TYPE_FROM_INTERFACE(mydoable) == MY_TYPE_MYDOABLE);

  mydoable->method1 = gx_gcontext_method1;
  mydoable->method2 = gx_gcontext_method2;
}
#endif

/* Instance Construction */
static void
gx_gcontext_init(GXGContext *self)
{
  self->priv = GX_GCONTEXT_GET_PRIVATE(self);
  /* populate your object here */
}

/* Instantiation wrapper */
GXGContext*
gx_gcontext_new(void)
{
  return GX_GCONTEXT(g_object_new(gx_gcontext_get_type(), NULL));
}

/* Instance Destruction */
void
gx_gcontext_finalize(GObject *object)
{
  /* GXGContext *self = GX_GCONTEXT(object); */

  /* destruct your object here */
  G_OBJECT_CLASS(parent_class)->finalize(object);
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
For more gtk-doc notes, see:
http://developer.gnome.org/arch/doc/authors.html
#endif


#if 0 /* getter/setter templates */
/**
 * gx_gcontext_get_PROPERTY:
 * @self:  A GXGContext.
 *
 * Fetches the PROPERTY of the GXGContext. FIXME, add more info!
 *
 * Returns: The value of PROPERTY. FIXME, add more info!
 */
PropType
gx_gcontext_get_PROPERTY(GXGContext *self)
{
  g_return_val_if_fail(GX_IS_GCONTEXT(self), /* FIXME */);

  return self->priv->PROPERTY;
  return g_strdup(self->priv->PROPERTY);
  return g_object_ref(self->priv->PROPERTY);
}

/**
 * gx_gcontext_set_PROPERTY:
 * @self:  A GXGContext.
 * @property:  The value to set. FIXME, add more info!
 *
 * Sets this properties value.
 *
 * This will also clear the properties previous value.
 */
void
gx_gcontext_set_PROPERTY(GXGContext *self, PropType PROPERTY)
{
  g_return_if_fail(GX_IS_GCONTEXT(self));

  if(self->priv->PROPERTY == PROPERTY)
    if(self->priv->PROPERTY == NULL
       || strcmp(self->priv->PROPERTY, PROPERTY) != 0)
      {
	self->priv->PROPERTY = PROPERTY;
	g_free(self->priv->PROPERTY);
	self->priv->PROPERTY = g_strdup(PROPERTY);
	g_object_unref(self->priv->PROPERTY);
	self->priv->PROPERTY = g_object_ref(PROPERTY);
	g_object_notify(G_OBJECT(self), "PROPERTY");
      }
}
#endif

xcb_gcontext_t
gx_gcontext_get_xcb_gcontext (GXGContext *self)
{
    return self->priv->xcb_gcontext;
}

