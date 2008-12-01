
#include <gx.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test-gx-common.h"

void
test_gerrors (TestGXSimpleFixture *fixture,
	      gconstpointer data)
{
  GXConnection *connection;
  GXWindow *bad_window;
  GError *error = NULL;

  connection = gx_connection_new (NULL);

  /* gobject initialisation can't fail by returning a NULL pointer, which
   * would otherwise been a neater mechanism for detecting an error.
   *
   * Instead we copy the XCB style:
   */
  if (gx_connection_has_error (connection))
    {
      g_printerr ("Error establishing connection to X server");
      exit (1);
    }

  /* NB: The "xid" + "wrap" properties can be used to create GXWindow
   * objects that wrap an existing window XID, but in this case we
   * are a wrapping an invalid xid: */
  bad_window = GX_WINDOW (g_object_new (GX_TYPE_WINDOW,
				        "connection", connection,
					"xid", 123456,
					"wrap", TRUE,
					NULL));

  if (gx_window_map_window (bad_window, &error))
    {
      g_print ("No error reported when mapping bad window!\n");
      exit (1);
    }
  else
    {
      g_assert (error);
      g_print ("Correctly failed to map bad window.\n");
      g_print ("Error message = %s\n", error->message);
    }

  g_print ("OK\n");

  g_object_unref (connection);

  return;
}

