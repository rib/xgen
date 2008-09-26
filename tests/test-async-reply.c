
#include <gx.h>

#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  GXConnection *connection;
  GXWindow *root;
  GXCookie *cookie;
  GXQueryTreeReply *query_tree;
  GArray *array;
  GXWindow **children;
  GXWindow *child;
  int i;

  g_type_init ();

  connection = gx_connection_new (NULL);
  if (gx_connection_has_error (connection))
    {
      g_printerr ("Error establishing connection to X server");
      return 1;
    }

  root = gx_connection_get_root_window (connection);

  cookie = gx_window_query_tree_async (root);

  /* Now you have an opertunity to do work here to hide the
   * request latency. */

  query_tree = gx_window_query_tree_reply (cookie, NULL);

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

  g_object_unref (root);
  g_object_unref (connection);

  return 0;
}

