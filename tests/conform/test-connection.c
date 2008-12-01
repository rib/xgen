
#include <gx.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test-gx-common.h"

void
test_connection (TestGXSimpleFixture *fixture,
		 gconstpointer data)
{
  GXConnection *connection;

  g_type_init();

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
  g_object_unref (connection);

  /* Now make sure the error paths work: */
  connection = gx_connection_new (":1000");

  if (!gx_connection_has_error (connection))
    {
      g_printerr ("Failed to detect error "
		  "when connecting to non existant server");
      exit (1);
    }
  g_object_unref (connection);

  g_print ("OK\n");

  return;
}

