
#include <gx.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test-gx-common.h"

void
test_reply (TestGXSimpleFixture *fixture,
            gconstpointer data)
{
  GXConnection *connection;
  GXWindow *root;
  GXWindowQueryTreeReply *query_tree;
  GArray *array;
  GXWindow **children;
  GXWindow *child;
  int i;

  g_type_init ();

  connection = gx_connection_new (NULL);
  if (gx_connection_has_error (connection))
    {
      g_printerr ("Error establishing connection to X server");
      exit (1);
    }

  root = gx_connection_get_root_window (connection);

  query_tree = gx_window_query_tree (root, NULL);

  array = gx_window_query_tree_get_children (query_tree);
  children = (GXWindow **)array->data;
  for (i = 0; i < array->len; i++)
    {
      child = children[i];
      g_print ("child of root (0x%08x)\n",
	       gx_drawable_get_xid (GX_DRAWABLE (child)));
    }
  gx_window_query_tree_free_children (array);

  gx_window_query_tree_reply_free (query_tree);

  /* FIXME - this test currently just tests things dont crash when issuing a
   * request and waiting for a reply. This test should actually verify the
   * value of some reply is correct */

  g_object_unref (root);
  g_object_unref (connection);

  return 0;
}

