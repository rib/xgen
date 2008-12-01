
#include <gx.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test-gx-common.h"

static void
print_screen_info (GXScreen *screen)
{
  GXWindow *root;
  const char *backing_stores;

  g_print ("Screen #%d:\n", gx_screen_get_number (screen));
  g_print ("dimensions:    %dx%d pixels (%dx%d millimeters)\n",
	   gx_screen_get_width (screen),
	   gx_screen_get_height (screen),
	   gx_screen_get_width_in_millimeters (screen),
	   gx_screen_get_height_in_millimeters (screen));
  /* TODO resolution: */
  /* TODO depths: */
  root = gx_screen_get_root (screen);
  g_print ("root window id:    0x%x\n",
	   gx_drawable_get_xid (GX_DRAWABLE (root)));
  g_print ("depth of root window:    %d planes\n",
	   gx_screen_get_root_depth (screen));
  g_print ("preallocated pixels:    black %ld, white %ld\n",
	   (unsigned long)gx_screen_get_black_pixel (screen),
	   (unsigned long)gx_screen_get_white_pixel (screen));

  /* TODO default visual id:
   * gx_screen_get_root_visual_id (screen);
   */

  switch (gx_screen_get_backing_stores (screen))
    {
    case GX_BACKING_STORE_NOT_USEFUL:
      backing_stores = "NO";
      break;
    case GX_BACKING_STORE_WHEN_MAPPED:
      backing_stores = "WHEN MAPPED";
      break;
    case GX_BACKING_STORE_ALWAYS:
      backing_stores = "YES";
      break;
    default:
      backing_stores = "Unknown";
    }
  g_print ("options:    backing-store %s, save-unders %s\n",
	   backing_stores,
	   gx_screen_get_save_unders (screen) ? "YES" : "NO");
}
void
test_screen_info (TestGXSimpleFixture *fixture,
		  gconstpointer data)
{
  GXConnection *connection;
  GXScreen     *default_screen;
  GList	       *screens;
  GList	       *tmp;

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

  default_screen = gx_connection_get_default_screen (connection);
  g_print ("default screen number: %d\n",
	   gx_screen_get_number (default_screen));

  screens = gx_connection_get_screens (connection);
  g_print ("number of screens: %d\n", g_list_length (screens));

  g_print ("\n");

  for (tmp = screens; tmp != NULL; tmp = tmp->next)
    {
      GXScreen *screen = tmp->data;

      print_screen_info (screen);

      g_object_unref (screen);
    }
  g_list_free (screens);

  g_print ("OK\n");

  g_object_unref (connection);

  return;
}

