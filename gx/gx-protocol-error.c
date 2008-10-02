
#include "gx-protocol-error.h"

#include <xcb/xcb.h>
#include <glib.h>


typedef struct {
    int		protocol_error_code;
    const char *description;
} GXProtocolErrorDetails;

static GXProtocolErrorDetails
gx_protocol_errors[] =
{
#include "gx-protocol-error-details-gen.h"
    {0, NULL}
};

GQuark
gx_protocol_error_quark (void)
{
  return g_quark_from_static_string ("gx-protocol-error-quark");
}

const char *
gx_protocol_error_get_description (GXProtocolError code)
{
  int i;

  if (code < GX_PROTOCOL_ERROR_LAST)
    return gx_protocol_errors[i].description;
  else
    return "Unknown error";
}

GXProtocolError
gx_protocol_error_from_xcb_generic_error (xcb_generic_error_t *error)
{
  int i;

  for (i = 0; i < GX_PROTOCOL_ERROR_LAST; i++)
    {
      if (gx_protocol_errors[i].protocol_error_code == error->error_code)
	return i;
    }
  g_warning ("Can't translate unknown error code (%d) into a GXProtocolError",
	     error->error_code);

  /* FIXME - we should probably choose a specific default when the error code
   * is unknown */
  return 0;
}

