/*
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

#include "gx-connection.h"
#include "gx-drawable.h"
#include "gx-window.h"
#include "gx-types.h"

#include <glib.h>

#include <xcb/xcb.h>
#include <xcb/xcbext.h>
#include <stdlib.h>
#include <string.h>

/* Macros and defines */
#define GX_CONNECTION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_CONNECTION, GXConnectionPrivate))

/* Enums/Typedefs */
/* add your signals here */
enum {
    EVENT_SIGNAL,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_DISPLAY,
};

struct _GXConnectionPrivate
{
  xcb_connection_t  *xcb_connection;
  gchar		    *display;
  int		     screen_num;
  xcb_screen_t	    *screen;
  GXWindow	    *root;

  /* NB: This mechanism wouldn't be needed if only xcb had
   * a way to poll for replies without needing to pass a
   * sequence number. */
  GQueue	    *pending_reply_sequences;
};


/* Function definitions */
static void gx_connection_class_init(GXConnectionClass *klass);
static void gx_connection_get_property(GObject *object,
				       guint id,
				       GValue *value,
				       GParamSpec *pspec);
static void gx_connection_set_property(GObject *object,
				       guint property_id,
				       const GValue *value,
				       GParamSpec *pspec);
//static void gx_connection_mydoable_interface_init(gpointer interface,
//                                             gpointer data);
static void gx_connection_init(GXConnection *self);
static gboolean connect_to_display (GXConnection *self, const char *display);
static void gx_connection_finalize(GObject *self);


/* Variables */
static GObjectClass *parent_class = NULL;
static guint gx_connection_signals[LAST_SIGNAL] = { 0 };

GType
gx_connection_get_type(void) /* Typechecking */
{
  static GType self_type = 0;

  if (!self_type)
    {
      static const GTypeInfo object_info =
	{
	  sizeof(GXConnectionClass), /* class structure size */
	  NULL, /* base class initializer */
	  NULL, /* base class finalizer */
	  (GClassInitFunc)gx_connection_class_init, /* class initializer */
	  NULL, /* class finalizer */
	  NULL, /* class data */
	  sizeof(GXConnection), /* instance structure size */
	  0, /* preallocated instances */
	  (GInstanceInitFunc)gx_connection_init, /* instance initializer */
	  NULL /* function table */
	};

      /* add the type of your parent class here */
      self_type = g_type_register_static(G_TYPE_OBJECT, /* parent GType */
					 "GXConnection", /* type name */
					 &object_info, /* type info */
					 0 /* flags */
      );
#if 0
      /* add interfaces here */
      static const GInterfaceInfo mydoable_info =
	{
	  (GInterfaceInitFunc)
	    gx_connection_mydoable_interface_init,
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
gx_connection_class_init(GXConnectionClass *klass) /* Class Initialization */
{
  GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
  GParamSpec *new_param;

  parent_class = g_type_class_peek_parent(klass);

  gobject_class->finalize = gx_connection_finalize;

  gobject_class->get_property = gx_connection_get_property;
  gobject_class->set_property = gx_connection_set_property;

  /* set up properties */
  new_param = g_param_spec_string("display", /* name */
				  "Display", /* nick name */
				  "X Server Display", /* description */
				   NULL, /* default */
                                   G_PARAM_READABLE
                                   | G_PARAM_WRITABLE
				   | G_PARAM_CONSTRUCT_ONLY
				   );
  g_object_class_install_property(gobject_class,
				  PROP_DISPLAY,
				  new_param);

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
  gx_connection_signals[SIGNAL_NAME] =
    g_signal_new("signal_name", /* name */
		 G_TYPE_FROM_CLASS(klass), /* interface GType */
		 G_SIGNAL_RUN_LAST, /* signal flags */
		 G_STRUCT_OFFSET(GXConnectionClass, signal_member),
		 NULL, /* accumulator */
		 NULL, /* accumulator data */
		 g_cclosure_marshal_VOID__VOID, /* c marshaller */
		 G_TYPE_NONE, /* return type */
		 0 /* number of parameters */
		 /* vararg, list of param types */
    );
#endif
  klass->event = NULL;
  gx_connection_signals[EVENT_SIGNAL] =
    g_signal_new ("event",	/* name */
		  G_TYPE_FROM_CLASS (klass),	/* interface GType */
		  G_SIGNAL_RUN_LAST,	/* signal flags */
		  G_STRUCT_OFFSET (GXConnectionClass, event), NULL,	/* accumulator */
		  NULL,	/* accumulator data */
		  g_cclosure_marshal_VOID__VOID,	/* c marshaller */
		  G_TYPE_NONE,	/* return type */
		  0	/* number of parameters */
		  /* vararg, list of param types */
    );

  g_type_class_add_private(klass, sizeof(GXConnectionPrivate));
}

static void
gx_connection_get_property(GObject *object,
			   guint id,
			   GValue *value,
			   GParamSpec *pspec)
{
  /* GXConnection* self = GX_CONNECTION(object); */

  switch(id) {
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
gx_connection_set_property(GObject *object,
			   guint property_id,
			   const GValue *value,
			   GParamSpec *pspec)
{
  GXConnection* self = GX_CONNECTION(object);

  switch(property_id)
    {
#if 0 /* template code */
    case PROP_NAME:
      gx_connection_set_property(self, g_value_get_int(value));
      gx_connection_set_property(self, g_value_get_uint(value));
      gx_connection_set_property(self, g_value_get_boolean(value));
      gx_connection_set_property(self, g_value_get_string(value));
      gx_connection_set_property(self, g_value_get_object(value));
      gx_connection_set_property(self, g_value_get_pointer(value));
      break;
#endif
    case PROP_DISPLAY:
      self->priv->display = g_value_dup_string(value);
      connect_to_display(self,self->priv->display);
      break;

    default:
      g_warning("gx_connection_set_property on unknown property");
      return;
    }
}

/* Initialize interfaces here */

#if 0
static void
gx_connection_mydoable_interface_init(gpointer interface,
				      gpointer data)
{
  MyDoableIface *mydoable = interface;
  g_assert(G_TYPE_FROM_INTERFACE(mydoable) == MY_TYPE_MYDOABLE);

  mydoable->method1 = gx_connection_method1;
  mydoable->method2 = gx_connection_method2;
}
#endif

/* Instance Construction */
static void
gx_connection_init (GXConnection *self)
{
  self->priv = GX_CONNECTION_GET_PRIVATE (self);
  /* populate your object here */

  self->priv->pending_reply_sequences = g_queue_new ();
}

/* Instantiation wrapper */
GXConnection *
gx_connection_new (const char *display)
{
  GXConnection *self =
    GX_CONNECTION (g_object_new (gx_connection_get_type (),
                   "display", display, NULL));

  return self;
}

/* Instance Destruction */
void
gx_connection_finalize (GObject *object)
{
  GXConnection *self = GX_CONNECTION (object);

  g_queue_free (self->priv->pending_reply_sequences);

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
For more gtk-doc notes, see:
http://developer.gnome.org/arch/doc/authors.html
#endif


#if 0 // getter/setter templates
/**
 * gx_connection_get_PROPERTY:
 * @self:  A GXConnection.
 *
 * Fetches the PROPERTY of the GXConnection. FIXME, add more info!
 *
 * Returns: The value of PROPERTY. FIXME, add more info!
 */
PropType
gx_connection_get_PROPERTY(GXConnection *self)
{
  g_return_val_if_fail(GX_IS_CONNECTION(self), /* FIXME */);

  //return self->priv->PROPERTY;
  //return g_strdup(self->priv->PROPERTY);
  //return g_object_ref(self->priv->PROPERTY);
}

/**
 * gx_connection_set_PROPERTY:
 * @self:  A GXConnection.
 * @property:  The value to set. FIXME, add more info!
 *
 * Sets this properties value.
 *
 * This will also clear the properties previous value.
 */
void
gx_connection_set_PROPERTY(GXConnection *self, PropType PROPERTY)
{
  g_return_if_fail(GX_IS_CONNECTION(self));

  //if(self->priv->PROPERTY == PROPERTY)
  //if(self->priv->PROPERTY == NULL
  //   || strcmp(self->priv->PROPERTY, PROPERTY) != 0)
    {
      //    self->priv->PROPERTY = PROPERTY;
      //    g_free(self->priv->PROPERTY);
      //    self->priv->PROPERTY = g_strdup(PROPERTY);
      //    g_object_unref(self->priv->PROPERTY);
      //    self->priv->PROPERTY = g_object_ref(PROPERTY);
      //    g_object_notify(G_OBJECT(self), "PROPERTY");
    }
}
#endif

xcb_connection_t *
gx_connection_get_xcb_connection (GXConnection *self)
{
    return self->priv->xcb_connection;
}

static xcb_screen_t *
screen_of_connection (xcb_connection_t *c,
		      int               screen)
{
  xcb_screen_iterator_t iter;

  iter = xcb_setup_roots_iterator (xcb_get_setup (c));
  for (; iter.rem; --screen, xcb_screen_next (&iter))
    if (screen == 0)
      return iter.data;

  return NULL;
}

static void
check_for_any_reply_or_error (GXConnection *connection,
			      void **reply,
			      xcb_generic_error_t **error)
{
  xcb_connection_t *xcb_connection =
    gx_connection_get_xcb_connection (connection);
  GList *tmp;

  g_return_if_fail (*reply == NULL);
  g_return_if_fail (*error == NULL);

  for (tmp = connection->priv->pending_reply_sequences->head;
       tmp != NULL;
       tmp = tmp->next)
  {
    unsigned int request = GPOINTER_TO_INT (tmp->data);
    xcb_poll_for_reply (xcb_connection, request, reply, error);
    if (reply || error)
      g_queue_remove (connection->priv->pending_reply_sequences, tmp->data);
  }
}

typedef struct
{
  GSource	       source;

  GPollFD	       xcb_poll_fd;

  GXConnection	      *connection;

  xcb_generic_event_t *event;
  void		      *reply;
  xcb_generic_error_t *error;

}GXXCBFDSource;

static gboolean
xcb_event_prepare (GSource *source,
                   gint    *timeout)
{
  GXXCBFDSource *xcb_source = (GXXCBFDSource *)source;
  xcb_connection_t *xcb_connection =
    gx_connection_get_xcb_connection (xcb_source->connection);

  g_assert (!xcb_source->event);
  xcb_source->event = xcb_poll_for_event (xcb_connection);
  if (xcb_source->event)
    return TRUE;

  /* TODO: This doesn't seem to be a very nice way to have to deal with
   * replies, but xcb does not currently have a xcb_poll_for_any_reply()
   * function and so you have to pass xcb a specific sequence number.
   */
  g_assert (!xcb_source->reply);
  check_for_any_reply_or_error (xcb_source->connection,
				&xcb_source->reply,
				&xcb_source->error);
  if (xcb_source->reply || xcb_source->error)
    return TRUE;

  /* We don't mind how long poll() will block */
  *timeout = -1;

  return FALSE;
}

static gboolean
xcb_event_check (GSource *source)
{
  GXXCBFDSource *xcb_source = (GXXCBFDSource *)source;

  if (xcb_source->xcb_poll_fd.revents & G_IO_IN)
    return TRUE;

  return FALSE;
}

static void
signal_event (GXConnection *connection, xcb_generic_event_t *event)
{
  static guint event_signal = 0;

  g_print ("event\n");

  if (!event_signal)
    event_signal = g_signal_lookup ("event", GX_TYPE_CONNECTION);

  /* FIXME - we should be able to determine the event window of an
   * event and send the signal from the corresponding GXWindow. */

  /* TODO: we should use the name of the signal as the detail */
  g_signal_emit (connection, event_signal, 0, event);

}

static void
signal_reply (GXConnection *connection, xcb_generic_reply_t *reply)
{
  g_print ("reply\n");
}

static void
signal_error (GXConnection *connection, xcb_generic_error_t *error)
{
  g_print ("error\n");
}

static gboolean
xcb_event_dispatch (GSource *source, GSourceFunc callback, gpointer data)
{
  GXXCBFDSource	      *xcb_source = (GXXCBFDSource *)source;
  xcb_connection_t    *xcb_connection =
    gx_connection_get_xcb_connection (xcb_source->connection);
  xcb_generic_event_t *event;
  void		      *reply;
  xcb_generic_error_t *error;


  xcb_source->event = NULL;
  event = xcb_source->event;

  xcb_source->reply = NULL;
  reply = xcb_source->reply;

  xcb_source->error = NULL;
  error = xcb_source->error;

  do
    {
      if (event)
	signal_event (xcb_source->connection, event);
      event = xcb_poll_for_event (xcb_connection);

      if (reply)
	signal_reply (xcb_source->connection, reply);
      else if (error)
	signal_error (xcb_source->connection, error);
      check_for_any_reply_or_error (xcb_source->connection,
				    &reply,
				    &error);

    } while (event || reply || error);

  return TRUE;
}

static GSourceFuncs xcb_source_funcs = {
  .prepare = xcb_event_prepare,
  .check = xcb_event_check,
  .dispatch = xcb_event_dispatch,
  .finalize = NULL,
};

static void
add_xcb_event_source(GXConnection *self)
{
  GSource *source;
  GXXCBFDSource *xcb_source;
  int fd;

  fd = xcb_get_file_descriptor (self->priv->xcb_connection);

  source = g_source_new (&xcb_source_funcs, sizeof (GXXCBFDSource));
  /* g_source_set_priority (source, ); */

  xcb_source = (GXXCBFDSource *)source;
  xcb_source->connection = self;
  xcb_source->event = NULL;
  xcb_source->reply = NULL;
  xcb_source->error = NULL;
  xcb_source->xcb_poll_fd.fd = fd;
  xcb_source->xcb_poll_fd.events = G_IO_IN;

  g_source_add_poll (source, &xcb_source->xcb_poll_fd);
  g_source_set_can_recurse (source, TRUE);
  g_source_attach (source, NULL);
}

static gboolean
connect_to_display (GXConnection *self, const char *display)
{
  if (!display)
    display = getenv("DISPLAY");

  g_print("DISPLAY=%s\n", display);

  self->priv->xcb_connection =
    xcb_connect (display, &self->priv->screen_num);

  self->priv->screen =
    screen_of_connection (self->priv->xcb_connection,
			  self->priv->screen_num);

  self->priv->root =
    GX_WINDOW (g_object_new (GX_TYPE_WINDOW,
			     "connection", self,
			     "xid", self->priv->screen->root,
			     "wrap", TRUE,
			     NULL));

  add_xcb_event_source (self);

  return TRUE;
}

GXWindow *
gx_connection_get_root_window (GXConnection *connection)
{
  return g_object_ref (connection->priv->root);
}

void
gx_connection_flush (GXConnection *connection, gboolean flush_server)
{
  //if (flush_server)
  //  xcb_aux_flush (connection->priv->xcb_connection);
  //else
    xcb_flush(connection->priv->xcb_connection);
}

void
gx_main(void)
{

}

#include "gx-connection-gen.c"

