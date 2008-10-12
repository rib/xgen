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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 * </license>
 *
 */

#include <gx/gx-connection.h>
#include <gx/gx-cookie.h>
#include <gx/gx-drawable.h>
#include <gx/gx-window.h>
#include <gx/gx-types.h>
#include <gx/gx-mask-value-item.h>
#include <gx/gx-protocol-error.h>

#include <glib.h>

#include <xcb/xcbext.h>
#include <stdlib.h>
#include <string.h>

/* Macros and defines */
#define GX_CONNECTION_GET_PRIVATE(object) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((object), GX_TYPE_CONNECTION, GXConnectionPrivate))

enum {
    EVENT_SIGNAL,
    REPLY_SIGNAL,
    ERROR_SIGNAL,
    LAST_SIGNAL
};

enum {
    PROP_0,
    PROP_DISPLAY,
};

typedef struct
{
  GSource	       source;
  GPollFD	       xcb_poll_fd;
  GXConnection	      *connection;
#if 0
  xcb_generic_event_t *event;
  void		      *reply;
  xcb_generic_error_t *error;
#endif
}GXXCBFDSource;

typedef enum _XCBResponseType
{
  _GX_COOKIE_RESPONSE_TYPE_REPLY,
  _GX_COOKIE_RESPONSE_TYPE_ERROR
} XCBResponseType;

typedef struct _XCBResponseData
{
  XCBResponseType  type;
  unsigned int	   sequence;
  GXCookie	  *cookie;
  void		  *data;
} XCBResponseData;

struct _GXConnectionPrivate
{
  xcb_connection_t  *xcb_connection;

  gchar		*display;
  int		 screen_num;
  xcb_screen_t	*screen;
  GXWindow	*root;

  guint		 xcb_source_id;
  GXXCBFDSource *xcb_source;

  /* While cookies are registered with a connection they are in
   * one of the the two following lists. */

  /** The list of cookies for which no reply/error has yet been
   * recieved
   */
  GQueue	*pending_reply_cookies;

  /** The list of cookies for which a reply has been recieved
   * but they are still registered and owned by the connection */
  GQueue	*zombie_reply_cookies;

  /* Events, replies and errors are queued up when retrieving them
   * from XCB so they may be dispatched one at a time from a custom
   * GSource to ensure the mainloop remains interactive. */
  GQueue	*events_queue;
  GQueue	*response_queue;
#if 0
  GQueue	*replies_queue;
  GQueue	*errors_queue;
#endif

  /** Indicates that connecting to the X server failed. */
  gboolean	 has_error;
};


/* Function definitions */
static void gx_connection_get_property (GObject *object,
					guint id,
					GValue *value,
					GParamSpec *pspec);
static void gx_connection_set_property (GObject *object,
					guint property_id,
					const GValue *value,
					GParamSpec *pspec);
/*
static void gx_connection_mydoable_interface_init (gpointer interface,
						   gpointer data);
*/
static void connect_to_display (GXConnection *self, const char *display);
void gx_connection_dispose (GObject *object);
static void gx_connection_finalize (GObject *self);
static void disconnect_from_display (GXConnection *self);

static guint gx_connection_signals[LAST_SIGNAL] = { 0 };


static GQuark cookie_pending_quark;

/* NB: This declares gx_connection_parent_class */
G_DEFINE_TYPE (GXConnection, gx_connection, G_TYPE_OBJECT);


static void
gx_connection_class_init (GXConnectionClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GParamSpec *new_param;

  cookie_pending_quark = g_quark_from_static_string ("pending");

  gobject_class->finalize = gx_connection_finalize;
  gobject_class->dispose = gx_connection_dispose;

  gobject_class->get_property = gx_connection_get_property;
  gobject_class->set_property = gx_connection_set_property;

  new_param = g_param_spec_string ("display", /* name */
				   "Display", /* nick name */
				   "X Server Display", /* description */
				   NULL, /* default */
				   G_PARAM_READABLE
				   | G_PARAM_WRITABLE
				   | G_PARAM_CONSTRUCT_ONLY
  );
  g_object_class_install_property (gobject_class,
				   PROP_DISPLAY,
				   new_param);

  klass->event = NULL;
  gx_connection_signals[EVENT_SIGNAL] =
    g_signal_new ("event", /* name */
		  G_TYPE_FROM_CLASS (klass), /* interface GType */
		  G_SIGNAL_RUN_LAST, /* signal flags */
		  G_STRUCT_OFFSET (GXConnectionClass, event),
		  NULL, /* accumulator */
		  NULL,	/* accumulator data */
		  g_cclosure_marshal_VOID__POINTER, /* c marshaller */
		  G_TYPE_NONE,	/* return type */
		  0 /* number of parameters */
		  /* vararg, list of param types */
    );
#if 0
  klass->reply = NULL;
  gx_connection_signals[REPLY_SIGNAL] =
    g_signal_new ("reply", /* name */
		  G_TYPE_FROM_CLASS (klass), /* interface GType */
		  G_SIGNAL_RUN_LAST, /* signal flags */
		  G_STRUCT_OFFSET (GXConnectionClass, reply),
		  NULL, /* accumulator */
		  NULL,	/* accumulator data */
		  g_cclosure_marshal_VOID__OBJECT, /* c marshaller */
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
		  g_cclosure_marshal_VOID__OBJECT, /* c marshaller */
		  G_TYPE_NONE,	/* return type */
		  0 /* number of parameters */
		  /* vararg, list of param types */
    );
#endif

  g_type_class_add_private (klass, sizeof(GXConnectionPrivate));
}

static void
gx_connection_get_property (GObject *object,
			    guint id,
			    GValue *value,
			    GParamSpec *pspec)
{
  /* GXConnection* self = GX_CONNECTION (object); */

  switch (id) {
#if 0 /* template code */
    case PROP_NAME:
      g_value_set_int(value, self->priv->property);
      break;
#endif
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, id, pspec);
      break;
  }
}

static void
gx_connection_set_property (GObject *object,
			    guint property_id,
			    const GValue *value,
			    GParamSpec *pspec)
{
  GXConnection* self = GX_CONNECTION (object);

  switch (property_id)
    {
#if 0 /* template code */
    case PROP_NAME:
      gx_connection_set_property(self, g_value_get_int(value));
      break;
#endif
    case PROP_DISPLAY:
      self->priv->display = g_value_dup_string (value);
      connect_to_display (self, self->priv->display);
      break;

    default:
      g_warning ("gx_connection_set_property on unknown property");
      return;
    }
}

static void
gx_connection_init (GXConnection *self)
{
  self->priv = GX_CONNECTION_GET_PRIVATE (self);
  /* populate your object here */

  self->priv->events_queue = g_queue_new ();
  self->priv->response_queue = g_queue_new ();
  self->priv->pending_reply_cookies = g_queue_new ();
  self->priv->zombie_reply_cookies = g_queue_new ();
}

GXConnection *
gx_connection_new (const char *display)
{
  GXConnection *self =
    GX_CONNECTION (g_object_new (gx_connection_get_type (),
				 "display", display, NULL));

  return self;
}

void
gx_connection_finalize (GObject *object)
{
  GXConnection *self = GX_CONNECTION (object);

  if (self->priv->xcb_connection)
    disconnect_from_display (self);

  g_queue_free (self->priv->events_queue);
  g_queue_free (self->priv->response_queue);
  g_queue_free (self->priv->pending_reply_cookies);
  g_queue_free (self->priv->zombie_reply_cookies);

  G_OBJECT_CLASS (gx_connection_parent_class)->finalize (object);
}

static void
cookie_pending_finalized_notify (gpointer data,
				 GObject *old_cookie)
{
  GXConnection *self = data;

  g_queue_remove (self->priv->pending_reply_cookies, old_cookie);
}

static void
response_queue_remove_cookie_references (GXConnection *self,
					 const GXCookie *cookie)
{
  GList *tmp;

  for (tmp = self->priv->response_queue->head;
       tmp != NULL;
       tmp = tmp->next)
    {
      XCBResponseData *response = tmp->data;
      if (response->cookie == cookie)
	{
	  g_queue_delete_link (self->priv->response_queue, tmp);
	  break;
	}
    }
}

static void
cookie_zombie_finalized_notify (gpointer data,
				GObject *old_cookie)
{
  GXConnection *self = data;

  response_queue_remove_cookie_references (self, GX_COOKIE (old_cookie));

  g_queue_remove (self->priv->zombie_reply_cookies, old_cookie);
}

void
gx_connection_dispose (GObject *object)
{
  GXConnection *self = GX_CONNECTION (object);
  GList *copy_list;
  GList *tmp;

  /* NB: There is a circular dependency between connection and
   * cookie objects.
   */

  /* We copy the list so we can use gx_connection_unregister_cookie
   * (which modifies the pending/zombie_reply_cookies lists).
   *
   * We use gx_connection_unregister_cookie so we only have one
   * place to deal with unregistering/unrefing cookies from the
   * connection */
  copy_list = g_list_copy (self->priv->pending_reply_cookies->head);
  for (tmp = copy_list; tmp != NULL; tmp = tmp->next)
    gx_connection_unregister_cookie (self, tmp->data);
  g_list_free (copy_list);

  copy_list = g_list_copy (self->priv->zombie_reply_cookies->head);
  for (tmp = copy_list; tmp != NULL; tmp = tmp->next)
    gx_connection_unregister_cookie (self, tmp->data);
  g_list_free (copy_list);

  G_OBJECT_CLASS (gx_connection_parent_class)->dispose (object);
}

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

static GXCookie *
check_for_any_reply_or_error (GXConnection *self,
			      void **reply,
			      xcb_generic_error_t **error)
{
  xcb_connection_t *xcb_connection =
    gx_connection_get_xcb_connection (self);
  GList *tmp;

  g_return_val_if_fail (reply && *reply == NULL, NULL);
  g_return_val_if_fail (error && *error == NULL, NULL);

  for (tmp = self->priv->pending_reply_cookies->head;
       tmp != NULL;
       tmp = tmp->next)
    {
      GXCookie *cookie = tmp->data;
      unsigned int request = gx_cookie_get_sequence (cookie);
      xcb_poll_for_reply (xcb_connection, request, reply, error);
#if 0
#error "FIXME: check xcb_poll_for_reply semantics. Does obtaining a reply/error pointer"
#error "this way mean we can't then call xcb_blah_reply(xcb_cookie) to get the same pointer?"
#error "I guess not. Can we perhaps push the reply/error back in to xcb if not?"
#endif
      if (*reply || *error)
	{
	  g_queue_remove (self->priv->pending_reply_cookies,
			  cookie);
	  g_object_weak_unref (G_OBJECT (cookie),
			       cookie_pending_finalized_notify,
			       self);

	  g_queue_push_tail (self->priv->zombie_reply_cookies,
			     cookie);
	  g_object_weak_ref (G_OBJECT (cookie),
			     cookie_zombie_finalized_notify,
			     self);
	  g_object_set_qdata (G_OBJECT (cookie), cookie_pending_quark, NULL);
	  return cookie;
	}
    }

  return NULL;
}

/**
 * reads a single event, reply or error from XCB and queues it
 * up for dispatch by our custom GSource.
 *
 * this function returns FALSE if nothing was read from XCB and
 * queued.
 */
static gboolean
queue_xcb_next (GXConnection *self)
{
  xcb_connection_t *xcb_connection =
    gx_connection_get_xcb_connection (self);
  xcb_generic_event_t *event;
  void *reply = NULL;
  xcb_generic_error_t *error = NULL;
  GXCookie *cookie = NULL;

  g_printerr ("queue_xcb_next0\n");

  event = xcb_poll_for_event (xcb_connection);
  if (event)
    {
      g_queue_push_tail (self->priv->events_queue, event);
      g_printerr ("queue_xcb_next EVENT\n");
      return TRUE;
    }


  /* FIXME: This doesn't seem to be a very nice way to have to deal with
   * replies, but xcb does not currently have a xcb_poll_for_any_reply()
   * function and so you have to pass xcb a specific sequence number.
   */
  cookie = check_for_any_reply_or_error (self,
					 &reply,
					 &error);
  if (cookie)
    {
      XCBResponseData *response_data =
	g_slice_alloc (sizeof (XCBResponseData));
      response_data->cookie = cookie;

      if (reply)
	{
	  g_printerr ("queue_xcb_next REPLY\n");
	  response_data->type = _GX_COOKIE_RESPONSE_TYPE_REPLY;
	  response_data->data = reply;
	}
      else
	{
	  g_printerr ("queue_xcb_next ERROR\n");
	  response_data->type = _GX_COOKIE_RESPONSE_TYPE_ERROR;
	  response_data->data = error;
	}

      g_queue_push_tail (self->priv->response_queue, response_data);

      return TRUE;
    }

  return FALSE;
}

static gboolean
is_anything_pending (GXConnection *self)
{
  if (g_queue_peek_head (self->priv->events_queue) != NULL)
    return TRUE;

  if (g_queue_peek_head (self->priv->response_queue) != NULL)
    return TRUE;

  return FALSE;
}

static gboolean
xcb_event_prepare (GSource *source,
		   gint    *timeout)
{
  GXXCBFDSource *xcb_source = (GXXCBFDSource *)source;

  /* We don't mind how long poll() will block */
  *timeout = -1;

  return is_anything_pending (xcb_source->connection)
	 || (queue_xcb_next (xcb_source->connection)
	     && is_anything_pending (xcb_source->connection));
}

static gboolean
xcb_event_check (GSource *source)
{
  GXXCBFDSource *xcb_source = (GXXCBFDSource *)source;

  if (xcb_source->xcb_poll_fd.revents & G_IO_IN)
    return is_anything_pending (xcb_source->connection)
	   || (queue_xcb_next (xcb_source->connection)
	       && is_anything_pending (xcb_source->connection));

  return FALSE;
}

static void
signal_event (GXConnection *connection, xcb_generic_event_t *event)
{
  g_print ("event\n");

  /* FIXME - we should be able to determine the event window of an
   * event and also send a signal from the corresponding GXWindow. */

  /* TODO: we should use the name of the signal as the detail */
  g_signal_emit (connection, gx_connection_signals[EVENT_SIGNAL], 0, event);

}

static void
signal_reply (GXConnection *connection, GXCookie *cookie)
{
  static guint reply_signal = 0;

  g_print ("reply\n");

  if (!reply_signal)
    reply_signal = g_signal_lookup ("reply", GX_TYPE_COOKIE);

  g_signal_emit (cookie, reply_signal, 0);
  g_signal_emit (connection, REPLY_SIGNAL, 0);
}

static void
signal_error (GXConnection *connection, GXCookie *cookie)
{
  static guint error_signal = 0;

  g_print ("error\n");

  if (!error_signal)
    error_signal = g_signal_lookup ("error", GX_TYPE_COOKIE);

  g_signal_emit (cookie, error_signal, 0);
  g_signal_emit (connection, ERROR_SIGNAL, 0);
}

static gboolean
xcb_event_dispatch (GSource *source, GSourceFunc callback, gpointer data)
{
  GXXCBFDSource	*xcb_source = (GXXCBFDSource *)source;
  GXConnection	*connection = xcb_source->connection;

  g_printerr ("xcb_event_dispatch\n");

  /* Queue up all data recieved from XCB. */
  while (queue_xcb_next (connection))
    ; /*  */

  if (g_queue_peek_head (connection->priv->events_queue))
    {
      xcb_generic_event_t *event =
	g_queue_pop_head (connection->priv->events_queue);
      signal_event (xcb_source->connection, event);
      return TRUE;
    }

  if (g_queue_peek_head (connection->priv->response_queue))
    {
      XCBResponseData *response =
	g_queue_pop_head (connection->priv->response_queue);
      if (response->type == _GX_COOKIE_RESPONSE_TYPE_REPLY)
	gx_cookie_set_reply (response->cookie, response->data);
      else
	{
	  /* FIXME - DEBUG */
	  g_print ("calling gx_cookie_set_error\n");
	  gx_cookie_set_error (response->cookie, response->data);
	}
      g_slice_free (XCBResponseData, response);
      return TRUE;
    }

  return FALSE;
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
#if 0
  xcb_source->event = NULL;
  xcb_source->reply = NULL;
  xcb_source->error = NULL;
#endif
  xcb_source->xcb_poll_fd.fd = fd;
  xcb_source->xcb_poll_fd.events = G_IO_IN;

  g_source_add_poll (source, &xcb_source->xcb_poll_fd);
  g_source_set_can_recurse (source, TRUE);
  self->priv->xcb_source_id = g_source_attach (source, NULL);
  self->priv->xcb_source = xcb_source;
}

static void
remove_xcb_event_source (GXConnection *self)
{
  g_source_remove (self->priv->xcb_source_id);
  g_source_unref ((GSource *)self->priv->xcb_source);
}

static void
connect_to_display (GXConnection *self, const char *display)
{
  if (!display)
    display = getenv("DISPLAY");

  self->priv->xcb_connection =
    xcb_connect (display, &self->priv->screen_num);

  if (xcb_connection_has_error (self->priv->xcb_connection))
    {
      self->priv->has_error = TRUE;
      /* NB: In this error condition, the xcb_connection_t returned
       * by xcb is actually a cast static int, so we dont need to
       * free anything. */
      self->priv->xcb_connection = NULL;
    }
  else
    {
      self->priv->has_error = FALSE;

      self->priv->screen =
	screen_of_connection (self->priv->xcb_connection,
			      self->priv->screen_num);

      /* The "xid" + "wrap" properties can be used to create GXWindow
       * objects that wrap and existing window XID */
      self->priv->root =
	GX_WINDOW (g_object_new (GX_TYPE_WINDOW,
				 "connection", self,
				 "xid", self->priv->screen->root,
				 "wrap", TRUE,
				 NULL));

      add_xcb_event_source (self);
    }
}

static void
disconnect_from_display (GXConnection *self)
{
  g_return_if_fail (self->priv->xcb_connection != NULL);

  remove_xcb_event_source (self);

  xcb_disconnect (self->priv->xcb_connection);
}

GXWindow *
gx_connection_get_root_window (GXConnection *connection)
{
  return g_object_ref (connection->priv->root);
}

void
gx_connection_flush (GXConnection *connection, gboolean flush_server)
{
  /*
     if (flush_server)
     xcb_aux_flush (connection->priv->xcb_connection);
     else
     */
  xcb_flush (connection->priv->xcb_connection);
}

gboolean
gx_connection_has_error (GXConnection *self)
{
  return self->priv->has_error;
}

/**
 * gx_connection_register_cookie:
 * @self: a GX Connection
 * @cookie: a new GX Cookie
 *
 * This registers a new cookie with a connection so that a reply corresponding
 * to that cookie can trigger a signal to the user.
 *
 * Typically you wouldn't have to do this yourself since any cookies you get
 * back from auto generated GX APIs will already be registered with your
 * connection.
 *
 * This function takes a reference on the cookie, which means that developers
 * shouldn't normally need to worry about managing the ref count of cookies.
 *
 * The cookie will be unref'd when it is unregistered from the connection,
 * which automatically happens via the gx_*_reply() functions.
 */
void
gx_connection_register_cookie (GXConnection *self, GXCookie *cookie)
{
  g_object_ref_sink (cookie);

  /* If developers manually unref a cookie, instead of calling
   * gx_connection_unregister_cookie, then we will be notified, and all will
   * be well.
   *
   * NB: We depend on the behaviour within gx_*_reply() functions.
   * It's slightly slimpler than calling gx_connection_unregister_cookie
   * because the weak pointer callbacks know which priv queue the
   * cookie is currently in. */

  g_object_weak_ref (G_OBJECT (cookie),
		     cookie_pending_finalized_notify,
		     self);
  g_queue_push_tail (self->priv->pending_reply_cookies,
		     cookie);
  g_object_set_qdata (G_OBJECT (cookie), cookie_pending_quark, "1");
}

/**
 * gx_connection_unregister_cookie:
 * @self: a GX Connection
 * @cookie: a new GX Cookie
 *
 * After calling this function the GXConnection will no longer be able to
 * cause a signal based notification of a reply for the coresponding cookie.
 *
 * As with gx_connection_register_cookie, developers shouldn't normally need
 * to use this directly since cookies are automatically unregistered via the
 * gx_*_reply() functions.
 */
void
gx_connection_unregister_cookie (GXConnection *self, GXCookie *cookie)
{
  g_queue_remove (self->priv->pending_reply_cookies, cookie);
  g_queue_remove (self->priv->zombie_reply_cookies, cookie);

  response_queue_remove_cookie_references (self, cookie);

  if (g_object_get_qdata (G_OBJECT (cookie), cookie_pending_quark))
    g_object_weak_unref (G_OBJECT (cookie),
			 cookie_pending_finalized_notify,
			 self);
  else
    g_object_weak_unref (G_OBJECT (cookie),
			 cookie_zombie_finalized_notify,
			 self);
  g_object_unref (cookie);
}

#include "gx-connection-gen.c"

