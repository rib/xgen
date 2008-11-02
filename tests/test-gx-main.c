
#include <glib.h>
#include <stdlib.h>

#include <gx.h>
#include "test-gx-common.h"


/* This is a bit of sugar for adding new gx tests:
 *
 * - It adds an extern function definition just to save maintaining a header
 *   that lists test entry points.
 * - It sets up callbacks for a fixture, which lets us share initialization
 *   *code* between tests. (see test-gx-common.c)
 * - It passes in a shared *data* pointer that is initialised once in main(),
 *   that gets passed to the fixture setup and test functions. (See the
 *   definition in test-gx-common.h)
 */
#define TEST_GX_SIMPLE(NAMESPACE, FUNC) \
  extern void FUNC (TestGXSimpleFixture *fixture, gconstpointer data); \
  g_test_add ("/gx" NAMESPACE "/" #FUNC, \
	      TestGXSimpleFixture, \
	      shared_state, /* data argument for test */ \
	      test_gx_simple_fixture_setup, \
	      FUNC, \
	      test_gx_simple_fixture_teardown);


int
main (int argc, char **argv)
{
  TestGXSharedState *shared_state = g_new0 (TestGXSharedState, 1);

  g_test_init (&argc, &argv, NULL);

  /* TODO */
  /* g_test_bug_base ("http://bugzilla.sixbynine.org/show_bug.cgi?id=%s"); */

  /* Initialise the state you need to share with everything.
   */
  shared_state->argc_addr = &argc;
  shared_state->argv_addr = &argv;

  gx_init (&argc, &argv);

  TEST_GX_SIMPLE ("", test_connection);
  TEST_GX_SIMPLE ("", test_reply);
  TEST_GX_SIMPLE ("", test_async_reply);
  TEST_GX_SIMPLE ("", test_cookie_life_cycle);
  TEST_GX_SIMPLE ("", test_gerrors);
  TEST_GX_SIMPLE ("", test_screen_info);

  g_test_run ();
  return EXIT_SUCCESS;
}

