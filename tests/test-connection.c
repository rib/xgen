
#include <gx.h>

#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  GXConnection *connection;
  GXWindow *root;
  GXWindow *window;

  g_type_init();

  connection = gx_connection_new (NULL);

  /* I don't think it's standard practice to allow gobject initialisation
   * to fail by returning a NULL pointer, which I think I would have
   * otherwise prefered as my the mechanism for detecting error.
   *
   * Instead we copy the XCB approach.
   */
  if (gx_connection_has_error (connection))
    {
      g_printerr ("Error establishing connection to X server");
      return 1;
    }

  root = gx_connection_get_root_window (connection);

  /* Note window creation via the gx_window_new function has a very
   * simplified interface, that can hopefully be used 99% of the
   * time.
   *
   * If for example you need to set the window border you can either
   * use gx_window_change_window_attributes(), or to gain access to
   * _all_ create window parameters you can use:
   *
   *  g_object_new (GX_TYPE_WINDOW,...)
   *		    "connection", connection,
   *		    "parent", parent,
   *		    ...,
   *		    NULL);
   *
   * which has a full range of construct only properties
   */
  window = gx_window_new (connection,
			  root, /* parent */
			  0, /* x */
			  0, /* y */
			  300, /* width */
			  300, /* height */
			  0 /* event mask */
			  );

  gx_window_map_window (window);

  gx_connection_flush (connection, FALSE);

  g_print ("Hello World\n");
  g_print ("Sleeping for 5 seconds...\n");
  sleep (5);

  g_object_unref (root);
  g_object_unref (connection);

  return 0;
}

