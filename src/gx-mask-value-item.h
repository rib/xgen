#ifndef _GX_VALUE_MASK_ITEM_H_
#define _GX_VALUE_MASK_ITEM_H_

#include <glib.h>

typedef struct {
  guint32 mask;
  guint32 value;
} GXMaskValueItem;

guint
gx_mask_value_items_get_count (GXMaskValueItem *items);


void
gx_mask_value_items_get_list (GXMaskValueItem *items,
			      guint32 *mask,
			      guint32 *buf);

#endif /* _GX_VALUE_MASK_ITEM_H_ */

