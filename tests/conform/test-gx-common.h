#include <glib.h>

/* Stuff you put in here is setup once in main() and gets passed around to
 * all test functions and fixture setup/teardown functions in the data
 * argument */
typedef struct _TestGXSharedState
{
  int	 *argc_addr;
  char ***argv_addr;
} TestGXSharedState;


/* This fixture structure is allocated by glib, and before running each test
 * the test_gx_simple_fixture_setup func (see below) is called to
 * initialise it, and test_gx_simple_fixture_teardown is called when
 * the test is finished. */
typedef struct _TestGXSimpleFixture
{
  /**/
} TestGXSimpleFixture;

void test_gx_simple_fixture_setup (TestGXSimpleFixture *fixture,
				   gconstpointer data);
void test_gx_simple_fixture_teardown (TestGXSimpleFixture *fixture,
				      gconstpointer data);

