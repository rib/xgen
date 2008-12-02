
#include <glib.h>
#include <stdlib.h>

#include <xgen.h>
#include "test-xgen-common.h"


/* This is a bit of sugar for adding new xgen tests:
 *
 * - It adds an extern function definition just to save maintaining a header
 *   that lists test entry points.
 * - It sets up callbacks for a fixture, which lets us share initialization
 *   *code* between tests. (see test-xgen-common.c)
 * - It passes in a shared *data* pointer that is initialised once in main(),
 *   that gets passed to the fixture setup and test functions. (See the
 *   definition in test-xgen-common.h)
 */
#define TEST_XGEN_SIMPLE(NAMESPACE, FUNC) \
  extern void FUNC (TestXGENSimpleFixture *fixture, gconstpointer data); \
  g_test_add ("/xgen" NAMESPACE "/" #FUNC, \
	      TestXGENSimpleFixture, \
	      shared_state, /* data argument for test */ \
	      test_xgen_simple_fixture_setup, \
	      FUNC, \
	      test_xgen_simple_fixture_teardown);


int
main (int argc, char **argv)
{
  TestXGENSharedState *shared_state = g_new0 (TestXGENSharedState, 1);

  g_test_init (&argc, &argv, NULL);

  /* TODO */
  /* g_test_bug_base ("http://bugzilla.sixbynine.org/show_bug.cgi?id=%s"); */

  /* Initialise the state you need to share with everything.
   */
  shared_state->argc_addr = &argc;
  shared_state->argv_addr = &argv;

  /* TEST_XGEN_SIMPLE ("", test_blah); */

  g_test_run ();
  return EXIT_SUCCESS;
}

