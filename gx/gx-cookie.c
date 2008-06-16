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

#include <gx/gx-cookie.h>

/* Macros and defines */
#define GX_COOKIE_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_COOKIE, GXCookiePrivate))

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

struct _GXCookiePrivate
{
#error template_fixme
};


/* Function definitions */
static void gx_cookie_class_init(GXCookieClass *klass);
static void gx_cookie_get_property(GObject *object,
				   guint id,
				   GValue *value,
				   GParamSpec *pspec);
static void gx_cookie_set_property(GObject *object,
				   guint property_id,
				   const GValue *value,
				   GParamSpec *pspec);
/* static void gx_cookie_mydoable_interface_init(gpointer interface,
   gpointer data); */
static void gx_cookie_init(GXCookie *self);
static void gx_cookie_finalize(GObject *self);


/* Variables */
static GObjectClass *parent_class = NULL;
#error template_fixme
/* static guint gx_cookie_signals[LAST_SIGNAL] = { 0 }; */

GType
gx_cookie_get_type(void) /* Typechecking */
{
  static GType self_type = 0;

  if (!self_type)
    {
      static const GTypeInfo object_info =
	{
	  sizeof(GXCookieClass), /* class structure size */
	  NULL, /* base class initializer */
	  NULL, /* base class finalizer */
	  (GClassInitFunc)gx_cookie_class_init, /* class initializer */
	  NULL, /* class finalizer */
	  NULL, /* class data */
	  sizeof(GXCookie), /* instance structure size */
	  0, /* preallocated instances */
	  (GInstanceInitFunc)gx_cookie_init, /* instance initializer */
	  NULL /* function table */
	};

      /* add the type of your parent class here */
#error template_fixme
      self_type = g_type_register_static(G_TYPE_OBJECT, /* parent GType */
					 "GXCookie", /* type name */
					 &object_info, /* type info */
					 0 /* flags */
      );
#if 0
      /* add interfaces here */
      static const GInterfaceInfo mydoable_info =
	{
	  (GInterfaceInitFunc)
	    gx_cookie_mydoable_interface_init,
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
gx_cookie_class_init(GXCookieClass *klass) /* Class Initialization */
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  /* GParamSpec *new_param;*/

  parent_class = g_type_class_peek_parent(klass);

  gobject_class->finalize = gx_cookie_finalize;

  gobject_class->get_property = gx_cookie_get_property;
  gobject_class->set_property = gx_cookie_set_property;

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
  gx_cookie_signals[SIGNAL_NAME] =
    g_signal_new("signal_name", /* name */
		 G_TYPE_FROM_CLASS(klass), /* interface GType */
		 G_SIGNAL_RUN_LAST, /* signal flags */
		 G_STRUCT_OFFSET(GXCookieClass, signal_member),
		 NULL, /* accumulator */
		 NULL, /* accumulator data */
		 g_cclosure_marshal_VOID__VOID, /* c marshaller */
		 G_TYPE_NONE, /* return type */
		 0 /* number of parameters */
		 /* vararg, list of param types */
    );
#endif

  g_type_class_add_private(klass, sizeof(GXCookiePrivate));
}

static void
gx_cookie_get_property(GObject *object,
		       guint id,
		       GValue *value,
		       GParamSpec *pspec)
{
  /* GXCookie* self = GX_COOKIE(object); */

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
gx_cookie_set_property(GObject *object,
		       guint property_id,
		       const GValue *value,
		       GParamSpec *pspec)
{
  /* GXCookie* self = GX_COOKIE(object); */

  switch(property_id)
    {
#if 0 /* template code */
    case PROP_NAME:
      gx_cookie_set_property(self, g_value_get_int(value));
      gx_cookie_set_property(self, g_value_get_uint(value));
      gx_cookie_set_property(self, g_value_get_boolean(value));
      gx_cookie_set_property(self, g_value_get_string(value));
      gx_cookie_set_property(self, g_value_get_object(value));
      gx_cookie_set_property(self, g_value_get_pointer(value));
      break;
#endif
    default:
      g_warning("gx_cookie_set_property on unknown property");
      return;
    }
}

/* Initialize interfaces here */

#if 0
static void
gx_cookie_mydoable_interface_init(gpointer interface,
				  gpointer data)
{
  MyDoableIface *mydoable = interface;
  g_assert(G_TYPE_FROM_INTERFACE(mydoable) == MY_TYPE_MYDOABLE);

  mydoable->method1 = gx_cookie_method1;
  mydoable->method2 = gx_cookie_method2;
}
#endif

/* Instance Construction */
static void
gx_cookie_init(GXCookie *self)
{
  self->priv = GX_COOKIE_GET_PRIVATE(self);
  /* populate your object here */
}

/* Instantiation wrapper */
GXCookie*
gx_cookie_new(void)
{
  return GX_COOKIE(g_object_new(gx_cookie_get_type(), NULL));
}

/* Instance Destruction */
void
gx_cookie_finalize(GObject *object)
{
  /* GXCookie *self = GX_COOKIE(object); */

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
 * gx_cookie_get_PROPERTY:
 * @self:  A GXCookie.
 *
 * Fetches the PROPERTY of the GXCookie. FIXME, add more info!
 *
 * Returns: The value of PROPERTY. FIXME, add more info!
 */
PropType
gx_cookie_get_PROPERTY(GXCookie *self)
{
  g_return_val_if_fail(GX_IS_COOKIE(self), /* FIXME */);

  return self->priv->PROPERTY;
  return g_strdup(self->priv->PROPERTY);
  return g_object_ref(self->priv->PROPERTY);
}

/**
 * gx_cookie_set_PROPERTY:
 * @self:  A GXCookie.
 * @property:  The value to set. FIXME, add more info!
 *
 * Sets this properties value.
 *
 * This will also clear the properties previous value.
 */
void
gx_cookie_set_PROPERTY(GXCookie *self, PropType PROPERTY)
{
  g_return_if_fail(GX_IS_COOKIE(self));

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

