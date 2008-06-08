
#include "gx.h"

#include <stdio.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
  GXConnection *connection;
  GXWindow *root;
  GXWindow *window;
  GXQueryTreeReply *query_tree;
  GArray *array;
  GXWindow **children;
  GXWindow *child;
  int i;

  g_type_init();

  connection = gx_connection_new(NULL);
  if (!connection)
    {
      printf("Moo\n");
      return 1;
    }
  root = gx_connection_get_root_window (connection);

  window = gx_window_new(connection,
			 root,
			 0,
			 0,
			 300,
			 300);

  gx_window_map_window (window);

  query_tree = gx_window_query_tree (root);

  array = gx_window_query_tree_get_children (query_tree);
  children = (GXWindow **)array->data;
  for (i = 0; i < array->len; i++)
    {
      child = children[i];
      g_print ("child\n");
    }
  gx_window_query_tree_free_children (array);

  gx_window_query_tree_reply_free (query_tree);

  gx_connection_flush (connection, FALSE);

  sleep (10);

  g_object_unref (root);
  g_object_unref (connection);
  printf("Hello World\n");
  return 0;
}

