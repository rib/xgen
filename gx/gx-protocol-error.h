#ifndef _GX_PROCOCOL_ERROR_H_
#define _GX_PROCOCOL_ERROR_H_

#include <glib.h>

#include <xcb/xcb.h>

#define GX_PROTOCOL_ERROR gx_protocol_error_quark ()

GQuark gx_protocol_error_quark (void);

typedef enum {

#include "gx-protocol-error-codes-gen.h"

    GX_PROTOCOL_ERROR_LAST
} GXProtocolError;

const char *
gx_protocol_error_get_description (GXProtocolError code);

GXProtocolError
gx_protocol_error_from_xcb_generic_error (xcb_generic_error_t *error);

#endif /* _GX_PROCOCOL_ERROR_H_ */

