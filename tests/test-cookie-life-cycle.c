
#include <gx.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "test-gx-common.h"

/* This is just to try and catch a silly mistake with the internal ref counting
 * of cookies, since it's expected that a lot of cookies will be created and
 * destroyed over the lifetime of a program.
 *
 * This test does two asynchronous query window requests, and for one cookie it
 * immediatly demands the reply by calling gx_window_query_reply, and for the
 * other a signal handler is installed for the reply. Once both replies have
 * been recieved and we expect that those cookie should have internally been
 * unrefed we quite the main loop and check that they have been finalized.
 *
 * Finally just before we unref the connection we issue another request and
 * after unrefing the connection that last cookie should also have been
 * finalized.
 */

gboolean cookie0_finalized = FALSE;
gboolean cookie1_finalized = FALSE;

gboolean cookie2_finalized = FALSE;

static int
check_cookies_0_and_1_status (void)
{
  int error = 0;

  if (!cookie0_finalized)
    {
      g_print ("cookie0 was not finalized!\n");
      error = 1;
    }
  if (!cookie1_finalized)
    {
      g_print ("cookie1 was not finalized!\n");
      error = 1;
    }
  return error;
}

static int
check_cookies_2_status (void)
{
  if (!cookie2_finalized)
    {
      g_print ("cookie2 was not finalized!\n");
      return 1;
    }
  return 0;
}

static void
cookie_finalize_notify (gpointer data,
                        GObject *where_the_object_was)
{
  gboolean *cookie_finalized_status = data;
  *cookie_finalized_status = TRUE;
}

static void
query_tree_reply_handler (GXCookie *self,
			  const GParamSpec *pspec,
			  gpointer user_data)
{
  GXWindowQueryTreeReply *query_tree;

  query_tree = gx_window_query_tree_reply (self, NULL);
  /* SNIP processing the reply */
  gx_window_query_tree_reply_free (query_tree);

  /*
   * At this point we expect that both cookies should have been finalized
   */
  gx_main_quit ();
}

void
test_cookie_life_cycle (TestGXSimpleFixture *fixture,
			gconstpointer data)
{
  GXConnection *connection;
  GXScreen *screen;
  GXWindow *root;
  GXCookie *cookie0;
  GXCookie *cookie1;
  GXCookie *cookie2;
  GXWindowQueryTreeReply *query_tree;
  int error = 0;

  connection = gx_connection_new (NULL);
  if (gx_connection_has_error (connection))
    {
      g_printerr ("Error establishing connection to X server");
      exit (1);
    }

  screen = gx_connection_get_default_screen (connection);
  root = gx_screen_get_root_window (screen);

  cookie0 = gx_window_query_tree_async (root);
  g_object_weak_ref (G_OBJECT (cookie0),
		     cookie_finalize_notify,
		     &cookie0_finalized);
  /* You could do work here to hide the request latency. */
  query_tree = gx_window_query_tree_reply (cookie0, NULL);
  /* SNIP processing the reply */
  gx_window_query_tree_reply_free (query_tree);


  cookie1 = gx_window_query_tree_async (root);
  g_object_weak_ref (G_OBJECT (cookie1),
		     cookie_finalize_notify,
		     &cookie1_finalized);
  g_signal_connect (cookie1,
		    "notify::reply",
		    G_CALLBACK (query_tree_reply_handler),
		    NULL);

  gx_connection_flush (connection, FALSE);

  gx_main();

  error |= check_cookies_0_and_1_status ();

  /* Check that when the connection is unrefed then all internal references
   * to cookies are unrefed */
  cookie2 = gx_window_query_tree_async (root);
  g_object_weak_ref (G_OBJECT (cookie2),
		     cookie_finalize_notify,
		     &cookie2_finalized);

  g_object_unref (root);
  g_object_unref (screen);
  g_object_unref (connection);

  error |= check_cookies_2_status ();

  if (error)
    exit (1);
}

