
#include <gx.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test-gx-common.h"

void
test_events (TestGXSimpleFixture *fixture,
             gconstpointer data)
{
  GXConnection *connection;
  GXWindow *root;
  GXWindow *window;
  GXWindowQueryTreeReply *query_tree;
  GArray *array;
  GXWindow **children;
  GXWindow *child;
  int i;

  /* FIXME - this api sugar still doesn't go far enough to
   * validate the mask bits are appropriate for a particular
   * request. */
  GXMaskValueItem change_window_attribute_values[] = {
    { GX_CW_EVENT_MASK, GX_EVENT_MASK_EXPOSURE },
    {NULL}
  };

  g_type_init ();

  connection = gx_connection_new (NULL);
  if (gx_connection_has_error (connection))
    {
      g_printerr ("Error establishing connection to X server");
      exit (1);
    }

  root = gx_connection_get_root_window (connection);

  window = gx_window_new (connection,
			  root,
			  0,
			  0,
			  300,
			  300,
			  GX_EVENT_MASK_EXPOSURE);

  gx_window_change_window_attributes (window,
				      change_window_attribute_values,
				      NULL);

  gx_window_map_window (window, NULL);

  query_tree = gx_window_query_tree (root, NULL);

  array = gx_window_query_tree_get_children (query_tree);
  children = (GXWindow **)array->data;
  for (i = 0; i < array->len; i++)
    {
      child = children[i];
      if (child == window)
	{
	  g_print ("Fooo\n");
	  gx_window_get_window_attributes (window, NULL);
	}
      g_print ("child\n");
    }
  gx_window_query_tree_free_children (array);

  gx_window_query_tree_reply_free (query_tree);

  gx_connection_flush (connection, FALSE);

  printf ("Hello World\n");

  gx_main();

  g_object_unref (root);
  g_object_unref (connection);

  return 0;
}

