/*
 * vim: tabstop=8 shiftwidth=2 noexpandtab softtabstop=2 cinoptions=>2,{2,:0,t0,(0,W4
 *
 * Copyright (C) 2008  Robert Bragg
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

#include <glib.h>

#include <gx/gx-cookie.h>

#define GX_COOKIE_GET_PRIVATE(object) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_COOKIE, GXCookiePrivate))

#if 0
enum {
    SIGNAL_NAME,
    LAST_SIGNAL
};
#endif

enum {
  PROP_0,
  PROP_CONNECTION,
  PROP_TYPE,
  PROP_SEQUENCE,
  PROP_REPLY,
  PROP_ERROR
};

struct _GXCookiePrivate
{
  GXCookieType	 type;
  GXConnection	*connection;
  unsigned int	 sequence;

  xcb_generic_reply_t *reply;
  xcb_generic_error_t *error;
};


static void gx_cookie_class_init (GXCookieClass *klass);
static void gx_cookie_get_property (GObject *object,
				    guint id,
				    GValue *value,
				    GParamSpec *pspec);
static void gx_cookie_set_property (GObject *object,
				    guint property_id,
				    const GValue *value,
				    GParamSpec *pspec);
/* static void gx_cookie_mydoable_interface_init(gpointer interface,
   gpointer data); */
static void gx_cookie_init (GXCookie *self);
static void gx_cookie_finalize (GObject *self);

#if 0
/* static guint gx_cookie_signals[LAST_SIGNAL] = { 0 }; */
#endif

/* NB: This declares gx_cookie_parent_class */
G_DEFINE_TYPE (GXCookie, gx_cookie, G_TYPE_INITIALLY_UNOWNED);

static void
gx_cookie_class_init (GXCookieClass *klass) /* Class Initialization */
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GParamSpec *new_param;

  gobject_class->finalize = gx_cookie_finalize;

  gobject_class->get_property = gx_cookie_get_property;
  gobject_class->set_property = gx_cookie_set_property;

  new_param = g_param_spec_int ("type", /* name */
				"Cookie Type", /* nick name */
				"The type of cookie ", /* description */
				0, /* minimum */
				G_MAXINT, /* maximum */
				0, /* default */
				G_PARAM_CONSTRUCT_ONLY
				| G_PARAM_WRITABLE
				| G_PARAM_READABLE
				);
  g_object_class_install_property (gobject_class,
				   PROP_TYPE,
				   new_param);

  new_param = g_param_spec_uint ("sequence", /* name */
				 "Sequence", /* nick name */
				 "The sequence number of the corresponding "
				 "request", /* description */
				 0, /* minimum */
				 G_MAXUINT, /* maximum */
				 0, /* default */
				 G_PARAM_CONSTRUCT_ONLY
				 | G_PARAM_WRITABLE
				 | G_PARAM_READABLE
				 );
  g_object_class_install_property (gobject_class,
				   PROP_SEQUENCE,
				   new_param);

  new_param = g_param_spec_object ("connection", /* name */
				   "Connection", /* nick name */
				   "The connection this cookie is associated "
				   "with", /* description */
				   GX_TYPE_CONNECTION,
				   G_PARAM_CONSTRUCT_ONLY
				   | G_PARAM_WRITABLE
				   | G_PARAM_READABLE
				   );
  g_object_class_install_property (gobject_class,
				   PROP_CONNECTION,
				   new_param);

  new_param = g_param_spec_pointer ("reply", /* name */
				    "Reply", /* nick name */
				    "The xcb_*_rely_t for this cookie's request",
				    G_PARAM_WRITABLE
				    | G_PARAM_READABLE
				    );
  g_object_class_install_property (gobject_class,
				   PROP_REPLY,
				   new_param);

  new_param = g_param_spec_pointer ("error", /* name */
				    "Error", /* nick name */
				    "Any xcb_*_error_t for this cookie's request",
				    G_PARAM_WRITABLE
				    | G_PARAM_READABLE
				    );
  g_object_class_install_property (gobject_class,
				   PROP_ERROR,
				   new_param);

  /* Signals */
#if 0
  klass->reply = NULL;
  gx_connection_signals[REPLY_SIGNAL] =
    g_signal_new ("reply", /* name */
		  G_TYPE_FROM_CLASS (klass), /* interface GType */
		  G_SIGNAL_RUN_LAST, /* signal flags */
		  G_STRUCT_OFFSET (GXConnectionClass, reply),
		  NULL, /* accumulator */
		  NULL,	/* accumulator data */
		  g_cclosure_marshal_VOID__VOID, /* c marshaller */
		  G_TYPE_NONE,	/* return type */
		  0 /* number of parameters */
		  /* vararg, list of param types */
    );
  klass->error = NULL;
  gx_connection_signals[ERROR_SIGNAL] =
    g_signal_new ("error", /* name */
		  G_TYPE_FROM_CLASS (klass), /* interface GType */
		  G_SIGNAL_RUN_LAST, /* signal flags */
		  G_STRUCT_OFFSET (GXConnectionClass, error),
		  NULL, /* accumulator */
		  NULL,	/* accumulator data */
		  g_cclosure_marshal_VOID__VOID, /* c marshaller */
		  G_TYPE_NONE,	/* return type */
		  0 /* number of parameters */
		  /* vararg, list of param types */
    );
#endif

  g_type_class_add_private (klass, sizeof (GXCookiePrivate));
}

static void
gx_cookie_get_property (GObject *object,
		        guint id,
		        GValue *value,
		        GParamSpec *pspec)
{
  GXCookie* self = GX_COOKIE (object);

  switch (id)
    {
#if 0 /* template code */
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
    case PROP_TYPE:
      g_value_set_int (value, self->priv->type);
      break;
    case PROP_SEQUENCE:
      g_value_set_uint (value, self->priv->sequence);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, id, pspec);
      break;
    }
}

#if 0
static void
connection_finalize_notify (gpointer data,
                            GObject *where_the_object_was)
{
  GXCookie *self = GX_COOKIE (data);

  self->priv->connection = NULL;
}
#endif

static void
gx_cookie_set_property (GObject *object,
		        guint property_id,
		        const GValue *value,
		        GParamSpec *pspec)
{
  GXCookie* self = GX_COOKIE (object);

  switch (property_id)
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
    case PROP_CONNECTION:
      self->priv->connection = g_value_get_object(value);
      g_object_add_weak_pointer (G_OBJECT (self->priv->connection),
				 (gpointer *)&self->priv->connection);
#if 0
      g_object_weak_ref (self->priv->connection,
			 connection_finalized_notify,
			 self);
#endif
      break;
    case PROP_TYPE:
      self->priv->type = g_value_get_int (value);
      break;
    case PROP_SEQUENCE:
      self->priv->sequence = g_value_get_uint (value);
      break;
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
gx_cookie_init (GXCookie *self)
{
  self->priv = GX_COOKIE_GET_PRIVATE (self);
  /* populate your object here */
}

/* Instantiation wrapper */
GXCookie *
gx_cookie_new (GXConnection *connection,
	       GXCookieType type,
	       unsigned int sequence)
{
  return GX_COOKIE (g_object_new (gx_cookie_get_type (),
				  "connection", connection,
				  "type", type,
				  "sequence", sequence,
				  NULL));
}

/* Instance Destruction */
void
gx_cookie_finalize (GObject *object)
{
  /* GXCookie *self = GX_COOKIE(object); */

  /* destruct your object here */
  G_OBJECT_CLASS (gx_cookie_parent_class)->finalize (object);
}



/* add new methods here */


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

GXCookieType
gx_cookie_get_cookie_type (GXCookie *self)
{
  return self->priv->type;
}

unsigned int
gx_cookie_get_sequence (GXCookie *self)
{
  return self->priv->sequence;
}

/**
 * gx_cookie_get_connection:
 * @self: a cookie
 *
 * This function returns a pointer to the connection object that the
 * passed cookie is associated with.
 *
 * Note: that the cookie itself doesn't own a reference on the connection
 * object it just maintains a weak pointer. This function doesn't ref the
 * connection object before returning it to you, so it is your responsability
 * to ref it, or setup another weak pointer.
 */
GXConnection *
gx_cookie_get_connection (GXCookie *self)
{
  return self->priv->connection;
}

/**
 * gx_cookie_set_reply:
 * @self: a cookie
 *
 * This function manually sets a xcb_*_reply_t pointer that will be used
 * when requesting the reply for the cookie. Normally you wouldn't use this
 * directly, since the connection object can asynchronously set the reply
 * data as soon as the data is available from the server.
 */
void
gx_cookie_set_reply (GXCookie *self, xcb_generic_reply_t *reply)
{
  self->priv->reply = reply;
  g_object_notify (G_OBJECT (self), "reply");
}

/**
 * gx_cookie_get_reply:
 * @self: a cookie
 *
 * If a reply has been recieved for this cookie then a pointer to the
 * corresponding xcb_*reply_t is returned, else NULL.
 */
GXGenericReply *
gx_cookie_get_reply (GXCookie *self)
{
  return self->priv->reply;
}

/**
 * gx_cookie_set_error:
 * @self: a cookie
 *
 * This function manually sets a xcb_*_reply_t pointer that will be used
 * when requesting the reply for the cookie. Normally you wouldn't use this
 * directly, since the connection object would asynchronously set the error
 * data if retrieved from the server.
 */
void
gx_cookie_set_error (GXCookie *self, xcb_generic_error_t *error)
{
  self->priv->error = error;
  g_object_notify (G_OBJECT (self), "error");
}

/**
 * gx_cookie_get_reply:
 * @self: a cookie
 *
 * If an error has been recieved for this cookie then a pointer to the
 * corresponding xcb_*reply_t is returned, else NULL.
 */
GXGenericError *
gx_cookie_get_error (GXCookie *self)
{
  return self->priv->error;
}

