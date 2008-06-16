

/* Many X requests take value-mask and value-list data, that is
 * awkward to prepare.
 *
 * "These are used to allow the client to specify a subset of a
 * heterogeneous collection of optional arguments. The value-mask
 * specifies which arguments are to be provided; each such argument
 * is assigned a unique bit position. The representation of the
 * BITMASK will typically contain more bits than there are defined
 * arguments. The unused bits in the value-mask must be zero (or the
 * server generates a .PN Value error). The value-list contains one
 * value for each bit set to 1 in the mask, from least significant
 * to most significant bit in the mask. Each value is represented
 * with four bytes, but the actual value occupies only the least
 * significant bytes as required. The values of the unused bytes do
 * not matter."
 * -- Copied from the XProtocol spec v 1.4
 *
 * XCB provides no API sugar to aid the setup of such lists, which
 * can be error prone given that you need to lookup the value
 * of each mask bit you use so as to be sure that you order your
 * data values appropriately.
 *
 * This API provides a simple GXMaskValueItem structure which pairs
 * together a mask value and a data value. Use these for aggregating
 * mask-value pairs in an un-ordered fashion.
 *
 * Where the XCB exposes separate value-mask and value-list
 * arguments, the GX API instead takes a NULL terminated array
 * of GXMaskValueItems. Internally we then use this API to generate
 * the final mask value and ordered value-data.
 *
 */

#include <gx-mask-value-item.h>

#include <glib.h>
#include <string.h>

/**
 * gx_mask_value_items_get_count:
 * @items: A NULL terminated array of mask-value pairs
 *
 * Counts the number of mask-value pairs.
 */
guint
gx_mask_value_items_get_count (GXMaskValueItem *items)
{
  guint count;
  for (count = 0; items[count].mask; count++)
    ; /* empty */

  return count;
}


/**
 * gx_mask_value_items_get_list:
 * @items: A NULL terminated array of mask-value pairs
 * @mask: Returns the OR'd together masks of each item
 * @buf: The destination buffer for storing the ordered values
 *
 * This copies the values from the array of mask-value pairs
 * into buf in order according to the masks. Note: we don't
 * allocate buf ourselves because it is expected to be short lived
 * and the caller will ideally allocate that buffer on the stack.
 */
void
gx_mask_value_items_get_list (GXMaskValueItem *items,
			      guint32 *mask,
			      guint32 *buf)
{
  GXMaskValueItem *ordered_items[32] = {0};
  guint i, j;

  *mask = 0;

  for (i = 0; items[i].mask; i++)
    {
      for (j = 0; j < 31; j++)
	if (items[i].mask >> j == 1)
	  break;
      ordered_items[j] = &items[i];
      *mask |= items[i].mask;
    }

  for (i = 0; i < 31; i++)
    {
      guint buf_index = 0;
      if (ordered_items[i])
	buf[buf_index++] = ordered_items[i]->value;
    }
}

