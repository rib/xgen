/* GXGen - Glib/GObject style XCB binding generator
 *
 * Copyright (C) 2004-2005 Josh Triplett
 * Copyright (C) 2008 Robert Bragg
 *
 * This package is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This package is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
 * License for more details.
 */

#include <libxml/parser.h>

#include <glib.h>
#include <glib/gprintf.h>

#include <stdarg.h>
#include <string.h>


typedef enum
{
  XGEN_VOID,
  XGEN_BOOLEAN,
  XGEN_CHAR,
  XGEN_SIGNED,
  XGEN_UNSIGNED,
  XGEN_XID,
  XGEN_FLOAT,
  XGEN_DOUBLE,
  XGEN_STRUCT,
  XGEN_UNION,
  XGEN_ENUM,
  XGEN_XIDUNION,
  XGEN_TYPEDEF,
  XGEN_REQUEST,
  XGEN_EVENT,
  XGEN_VALUEPARAM,
  XGEN_ERROR,
} XGenType;

#if 0
typedef enum
{
  XGEN_REQUEST,
  XGEN_RESPONSE
} XGenDirection;
#endif

typedef struct _XGenDefinition XGenDefinition;
struct _XGenDefinition
{
  char *name;
  XGenType type;

  /* FIXME: Put the following into seperate typedefs
   * and create a union: */

  /* base types */
  unsigned int size;

  /* struct, union, enum */
  union {
    GList *fields;
    GList *items;
  };

  /* typedef / valueparam */
  XGenDefinition *reference;

  /* requests */
  GList *reply_fields;

  /* valueparams */
  gchar *mask_name;
  gchar *list_name;
};

typedef enum
{
  XGEN_FIELDREF,
  XGEN_VALUE,
  XGEN_OP
} XGenExpressionType;

typedef enum
{
  XGEN_ADD,
  XGEN_SUBTRACT,
  XGEN_MULTIPLY,
  XGEN_DIVIDE,
  XGEN_LEFT_SHIFT,
  XGEN_BITWISE_AND
} XGenOp;

typedef struct _XGenExpression XGenExpression;
struct _XGenExpression
{
  XGenExpressionType type;
  union
  {
    char *field;		/* Field name for XGEN_FIELDREF */
    unsigned long value;	/* Value for XGEN_VALUE */
    struct			/* Operator and operands for XGEN_OP */
    {
      XGenOp op;
      XGenExpression *left;
      XGenExpression *right;
    };
  };
};

typedef struct
{
  char *name;
  XGenDefinition *definition;
  XGenExpression *length;      /* List length; NULL for non-list */
} XGenFieldDefinition;

typedef struct
{
  XGenFieldDefinition *field;
  unsigned int offset;
  union
  {
    unsigned char bool_value;
    char	  char_value;
    signed long	  signed_value;
    unsigned long unsigned_value;
  };
} XGenFieldValue;

typedef enum
{
  XGEN_ITEM_AS_VALUE = 1,
  XGEN_ITEM_AS_BIT,
}XGenItemType;

typedef struct
{
  XGenItemType	 type;
  char		*name;
  char		*value;
  guint		 bit;
} XGenItemDefinition;

typedef struct
{
  char	*name;
  char  *header;
  GList *definitions;
  GList *requests;
  GList *events;
  GList *errors;
} XGenExtension;

/* Concrete definitions for opaque and private structure types. */
typedef struct
{
  XGenExtension	  *extension;
  unsigned char	   opcode;
  XGenDefinition  *definition;
} XGenRequest;

typedef struct
{
  XGenExtension	  *extension;
  unsigned char	   number;
  XGenDefinition  *definition;
} XGenEvent;

typedef struct
{
  XGenExtension	  *extension;
  unsigned char	   number;
  XGenDefinition  *definition;
} XGenError;

typedef struct
{
  gboolean   host_is_little_endian;
  GList	    *definitions;
  GList	    *extensions;
} XGenState;

static const XGenDefinition core_type_definitions[] = {
  {
    .name = "void",
    .type = XGEN_VOID,
    .size = 0},
  {
    .name = "char",
    .type = XGEN_CHAR,
    .size = 1},
  {
    .name = "float",
    .type = XGEN_FLOAT,
    .size = 4},
  {
    .name = "double",
    .type = XGEN_DOUBLE,
    .size = 8},
  {
    .name = "BOOL",
    .type = XGEN_BOOLEAN,
    .size = 1},
  {
    .name = "BYTE",
    .type = XGEN_UNSIGNED,
    .size = 1},
  {
    .name = "CARD8",
    .type = XGEN_UNSIGNED,
    .size = 1},
  {
    .name = "CARD16",
    .type = XGEN_UNSIGNED,
    .size = 2},
  {
    .name = "CARD32",
    .type = XGEN_UNSIGNED,
    .size = 4},
  {
    .name= "INT8",
    .type = XGEN_SIGNED,
    .size = 1},
  {
    .name = "INT16",
    .type = XGEN_SIGNED,
    .size = 2},
  {
    .name = "INT32",
    .type = XGEN_SIGNED,
    .size = 4}
};

struct _TypeMapping
{
  char *from;
  char *to;
};

static struct _TypeMapping core_type_mappings[] = {
  {"void", "void"},
  {"char", "gchar"},
  {"float", "gfloat"},
  {"double", "gdouble"},
  {"BOOL", "guint8"},
  {"BYTE", "guint8"},
  {"CARD8", "guint8"},
  {"CARD16", "guint16"},
  {"CARD32", "guint32"},
  {"INT8", "gint8"},
  {"INT16", "gint16"},
  {"INT32", "gint32"},

  {NULL}
};

struct _CamelCaseMapping
{
  char *uppercase;
  char *camelcase;
};

/* We need to use a simple dictionary to convert various uppercase
 * X protocol types into CamelCase.
 *
 * NB: replacements are done in order from top to bottom so put
 * shorter words at the bottom.
 *
 * NB: replacements must have the same length.
 */
static struct _CamelCaseMapping camelcase_dictionary[] = {
  {"RECTANGLE","Rectangle"},
  {"TIMESTAMP","Timestamp"},
  {"COLORMAP","Colormap"},
  {"COLORITEM","Coloritem"},
  {"FONTABLE","Fontable"},
  /* {"GLYPHSET","GlyphSet"}, */
  {"CONTEXT","Context"},
  {"PICTURE","Picture"},
  {"SEGMENT","Segment"},
  {"BUTTON","Button"},
  {"CURSOR","Cursor"},
  {"FORMAT","Format"},
  {"REGION","Region"},
  {"SCREEN","Screen"},
  {"VISUAL","Visual"},
  {"DEPTH","Depth"},
  {"FIXED","Fixed"},
  {"GLYPH","Glyph"},
  {"POINT","Point"},
  {"ATOM","Atom"},
  {"CHAR","Char"},
  {"CODE","Code"},
  {"FONT","Font"},
  {"HOST","Host"},
  {"KIND","Kind"},
  {"INFO","Info"},
  {"PICT","Pict"},
  {"PROP","Prop"},
  {"TYPE","Type"},
  {"ARC","Arc"},
  {"FIX","Fix"},
  {"KEY","Key"},
  {"MAP","Map"},
  {"SET","Set"},
  {"SYM","Sym"},
  {NULL}
};


#define OUT(PART, ...) \
  out (output_context, PART, __VA_ARGS__)
#define OUT2(PART0, PART1, ...) \
  out2 (output_context, PART0, PART1, __VA_ARGS__)


typedef enum
{
  GXGEN_PART_CONNECTION_OBJ_H_INC,
  GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
  GXGEN_PART_CONNECTION_OBJ_H_MACROS,
  GXGEN_PART_CONNECTION_OBJ_H_PROTOS,

  GXGEN_PART_CONNECTION_OBJ_C_INC,
  GXGEN_PART_CONNECTION_OBJ_C_PROTOS,
  GXGEN_PART_CONNECTION_OBJ_C_FUNCS,

  GXGEN_PART_DRAWABLE_OBJ_H_INC,
  GXGEN_PART_DRAWABLE_OBJ_H_TYPEDEFS,
  GXGEN_PART_DRAWABLE_OBJ_H_MACROS,
  GXGEN_PART_DRAWABLE_OBJ_H_PROTOS,

  GXGEN_PART_DRAWABLE_OBJ_C_INC,
  GXGEN_PART_DRAWABLE_OBJ_C_PROTOS,
  GXGEN_PART_DRAWABLE_OBJ_C_FUNCS,

  GXGEN_PART_PIXMAP_OBJ_H_INC,
  GXGEN_PART_PIXMAP_OBJ_H_TYPEDEFS,
  GXGEN_PART_PIXMAP_OBJ_H_MACROS,
  GXGEN_PART_PIXMAP_OBJ_H_PROTOS,

  GXGEN_PART_PIXMAP_OBJ_C_INC,
  GXGEN_PART_PIXMAP_OBJ_C_PROTOS,
  GXGEN_PART_PIXMAP_OBJ_C_FUNCS,

  GXGEN_PART_WINDOW_OBJ_H_INC,
  GXGEN_PART_WINDOW_OBJ_H_TYPEDEFS,
  GXGEN_PART_WINDOW_OBJ_H_MACROS,
  GXGEN_PART_WINDOW_OBJ_H_PROTOS,

  GXGEN_PART_WINDOW_OBJ_C_INC,
  GXGEN_PART_WINDOW_OBJ_C_PROTOS,
  GXGEN_PART_WINDOW_OBJ_C_FUNCS,

  GXGEN_PART_GCONTEXT_OBJ_H_INC,
  GXGEN_PART_GCONTEXT_OBJ_H_TYPEDEFS,
  GXGEN_PART_GCONTEXT_OBJ_H_MACROS,
  GXGEN_PART_GCONTEXT_OBJ_H_PROTOS,

  GXGEN_PART_GCONTEXT_OBJ_C_INC,
  GXGEN_PART_GCONTEXT_OBJ_C_PROTOS,
  GXGEN_PART_GCONTEXT_OBJ_C_FUNCS,

  GXGEN_PART_COOKIE_OBJ_H_TYPEDEFS,

  GXGEN_PART_ERROR_CODES_H_ENUMS,
  GXGEN_PART_ERROR_DETAILS_C,

  GXGEN_PART_XCB_DEPENDENCIES_H,

  GXGEN_PART_TESTS_C,

  GXGEN_PART_COUNT
} GXGenPart;


typedef enum {
  GXGEN_IS_CONNECTION_OBJ,
  GXGEN_IS_DRAWABLE_OBJ,
  GXGEN_IS_PIXMAP_OBJ,
  GXGEN_IS_WINDOW_OBJ,
  GXGEN_IS_GCONTEXT_OBJ
} GXGenOutputObjectType;

typedef struct _GXGenOutputObject {
  GXGenOutputObjectType	 type;
  /* lc=lowercase
   * cc=camel case
   * uc=uppercase
   */
  const char		*name_cc;
  char			*name_uc;
  char			*name_lc;
  const char		*first_arg;
  XGenFieldDefinition	*first_object_field;
  GXGenPart		 h_typedefs, h_protos, c_funcs;
} GXGenOutputObject;

typedef struct _GXGenOutputRequest
{
  const XGenRequest *request;
  /* lc=lowercase
   * cc=camel case
   * uc=uppercase
   */
  const char	    *xcb_name_lc;
  const char	    *xcb_name_cc;
  const char	    *xcb_name_uc;
  const char	    *gx_name_lc;
  const char	    *gx_name_cc;
  const char	    *gx_name_uc;
} GXGenOutputRequest;

typedef struct _GXGenOutputNamespace
{
  /* lc=lowercase
   * cc=camel case
   * uc=uppercase
   */
  char *gx_lc;
  char *gx_uc;
  char *gx_cc;
  char *xcb_lc;
  char *xcb_cc;
  char *xcb_uc;
} GXGenOutputNamespace;

typedef struct _GXGenOutputContext
{
  const XGenState	       *state;
  GString		      **parts;
  const XGenExtension	       *extension;
  const GXGenOutputObject      *obj;
  const GXGenOutputRequest     *out_request;
  const GXGenOutputNamespace   *namespace;
} GXGenOutputContext;



/* Helper function to avoid casting. */
static char *
xgen_xml_get_prop (xmlNodePtr node, const char *name)
{
  return (char *) xmlGetProp (node, (xmlChar *) name);
}

/* Helper function to avoid casting. */
static char *
xgen_xml_get_node_name (xmlNodePtr node)
{
  return (char *) node->name;
}

/* Helper function to avoid casting. */
static char *
xgen_xml_get_node_content (xmlNodePtr node)
{
  return (char *) xmlNodeGetContent (node);
}

static xmlNode *
xgen_xml_next_elem (xmlNode * elem)
{
  while (elem && elem->type != XML_ELEMENT_NODE)
    elem = elem->next;
  return elem;
}

static XGenDefinition *
xgen_find_type_in_extension (XGenExtension *extension, char *type_name)
{
  GList *tmp;

  for (tmp = extension->definitions; tmp != NULL; tmp = tmp->next)
    {
      XGenDefinition *def = tmp->data;

      if (strcmp (def->name, type_name) == 0)
	  return def;
    }

  return NULL;
}

static XGenDefinition *
xgen_find_type (XGenState * state, char *name)
{
  GList *tmp;
  char **bits;
  char *extension_name = NULL;
  char *type_name;
  XGenExtension *current_extension;

  /* NB: The head of state->extensions will correspond to the
   * extension currently being parsed */
  current_extension = state->extensions->data;

  bits = g_strsplit (name, ":", 2);

  if (bits[1] != NULL)
    {
      extension_name = bits[0];
      type_name = bits[1];
    }
  else
    {
      extension_name = current_extension->name;
      type_name = bits[0];
    }

  /* First we try in looking in the extension being parsed
   * or the extension that was explicitly specified. */
  for (tmp = state->extensions;
       tmp != NULL;
       tmp = tmp->next)
    {
      XGenExtension *extension = tmp->data;
      if (strcmp (extension->name, extension_name) == 0)
	{
	  XGenDefinition *def =
	    xgen_find_type_in_extension (extension, type_name);
	  if (def)
	    return def;
	}
    }
#if 0
  for (i = 0; core_type_mappings[i].from != NULL; i++)
    {

    }
#endif

  /* If an extension was explicitly specified we have no where
   * else to look */
  if (bits[1] != NULL)
    {
      g_critical ("Failed to find type = %s\n", name);
      return NULL;
    }

  for (tmp = state->extensions;
       tmp != NULL;
       tmp = tmp->next)
    {
      XGenExtension *extension = tmp->data;
      XGenDefinition *def =
	xgen_find_type_in_extension (extension, type_name);
      if (def)
	return def;
    }

  g_critical ("Failed to find type = %s\n", name);

  g_strfreev(bits);
  return NULL;
}

static XGenExpression *
xgen_parse_expression (XGenState * state, xmlNode * elem)
{
  XGenExpression *e = g_new0 (XGenExpression, 1);
  elem = xgen_xml_next_elem (elem);
  if (strcmp (xgen_xml_get_node_name (elem), "op") == 0)
    {
      char *temp = xgen_xml_get_prop (elem, "op");
      e->type = XGEN_OP;
      if (strcmp (temp, "+") == 0)
	e->op = XGEN_ADD;
      else if (strcmp (temp, "-") == 0)
	e->op = XGEN_SUBTRACT;
      else if (strcmp (temp, "*") == 0)
	e->op = XGEN_MULTIPLY;
      else if (strcmp (temp, "/") == 0)
	e->op = XGEN_DIVIDE;
      else if (strcmp (temp, "<<") == 0)
	e->op = XGEN_LEFT_SHIFT;
      else if (strcmp (temp, "&") == 0)
	e->op = XGEN_BITWISE_AND;
      elem = xgen_xml_next_elem (elem->children);
      e->left = xgen_parse_expression (state, elem);
      elem = xgen_xml_next_elem (elem->next);
      e->right = xgen_parse_expression (state, elem);
    }
  else if (strcmp (xgen_xml_get_node_name (elem), "value") == 0)
    {
      e->type = XGEN_VALUE;
      e->value = strtol (xgen_xml_get_node_content (elem), NULL, 0);
    }
  else if (strcmp (xgen_xml_get_node_name (elem), "fieldref") == 0)
    {
      e->type = XGEN_FIELDREF;
      e->field = strdup (xgen_xml_get_node_content (elem));
    }
  return e;
}

static GList *
xgen_parse_field_elements (XGenState * state, xmlNode * elem)
{
  xmlNode *cur;
  GList *fields = NULL;

  for (cur = elem->children;
       cur != NULL;
       cur = xgen_xml_next_elem (cur->next))
    {
      XGenFieldDefinition *field;
      field = g_new0 (XGenFieldDefinition, 1);
      if (strcmp (xgen_xml_get_node_name (cur), "pad") == 0)
	{
	  field->name = "pad";
	  field->definition = xgen_find_type (state, "CARD8");
	  field->length = g_new0 (XGenExpression, 1);
	  field->length->type = XGEN_VALUE;
	  field->length->value = atoi (xgen_xml_get_prop (cur, "bytes"));
	}
      else if (strcmp (xgen_xml_get_node_name (cur), "field") == 0)
	{
	  field->name = strdup (xgen_xml_get_prop (cur, "name"));
	  field->definition = xgen_find_type (state,
						xgen_xml_get_prop (cur,
								     "type"));
	}
      else if (strcmp (xgen_xml_get_node_name (cur), "list") == 0)
	{
	  field->name = strdup (xgen_xml_get_prop (cur, "name"));
	  field->definition = xgen_find_type (state,
						xgen_xml_get_prop (cur,
								     "type"));
	  if (cur->children)
	    {
	      field->length = xgen_parse_expression (state, cur->children);
	    }
	  else
	    {
	      XGenFieldDefinition *len_field;
	      XGenExpression *exp;

	      len_field = g_new0 (XGenFieldDefinition, 1);
	      len_field->name = g_strdup_printf ("%s_len", field->name);
	      len_field->definition = xgen_find_type (state, "CARD32");

	      fields = g_list_prepend (fields, len_field);

	      exp = g_new0 (XGenExpression, 1);
	      exp->type = XGEN_FIELDREF;
	      exp->field = g_strdup (field->name);
	      field->length = exp;
	    }
	}
      else if (strcmp (xgen_xml_get_node_name (cur), "valueparam") == 0)
	{
	  XGenDefinition *definition;
	  field->name = g_strdup ("valueparam");
	  definition = g_new0 (XGenDefinition, 1);
	  definition->name = g_strdup ("valueparam");
	  definition->type = XGEN_VALUEPARAM;
	  definition->reference =
	    xgen_find_type (state,
			      xgen_xml_get_prop (cur, "value-mask-type"));
	  definition->mask_name =
	    strdup (xgen_xml_get_prop (cur, "value-mask-name"));
	  definition->list_name =
	    strdup (xgen_xml_get_prop (cur, "value-list-name"));
	  field->definition = definition;
	}
      else
	continue;
      fields = g_list_prepend (fields, field);
    }

  fields = g_list_reverse (fields);
  return fields;
}

static GList *
xgen_parse_item_elements (XGenState * state, xmlNode * elem)
{
  xmlNode *cur, *cur2;
  GList *items = NULL;
  long last_value = -1;

  for (cur = elem->children;
       cur != NULL;
       cur = xgen_xml_next_elem (cur->next))
    {
      XGenItemDefinition *item = g_new0 (XGenItemDefinition, 1);

      item->name = xgen_xml_get_prop (cur, "name");

      for (cur2 = cur->children;
	   cur2 != NULL;
	   cur2 = xgen_xml_next_elem (cur2->next))
      {
	if (strcmp (xgen_xml_get_node_name (cur2), "value") == 0)
	  {
	    char *endptr;
	    item->type = XGEN_ITEM_AS_VALUE;
	    item->value = xgen_xml_get_node_content (cur2);
	    last_value = strtol (item->value, &endptr, 0);
	    g_assert (item->value[0] != '\0' && endptr[0] == '\0');
	    break;
	  }
	else if (strcmp (xgen_xml_get_node_name (cur2), "bit") == 0)
	  {
	    char *bit = xgen_xml_get_node_content (cur2);
	    item->type = XGEN_ITEM_AS_BIT;
	    item->bit = atoi (bit);
	    xmlFree (bit);
	    last_value = (1 << item->bit);
	    break;
	  }
      }
      if (!item->type)
	{
	  item->type = XGEN_ITEM_AS_VALUE;
	  item->value = g_strdup_printf("%ld", ++last_value);
	}

      items = g_list_prepend (items, item);
    }

  items = g_list_reverse (items);

  return items;
}

static GList *
xgen_parse_reply_fields (XGenState * state, xmlNode * elem)
{
  xmlNode *cur;
  xmlNode *reply = NULL;

  for (cur = elem->children;
       cur != NULL; cur = xgen_xml_next_elem (cur->next))
    {
      if (strcmp (xgen_xml_get_node_name (cur), "reply") == 0)
	{
	  reply = cur;
	  break;
	}
    }

  if (!reply)
    return NULL;

  return xgen_parse_field_elements (state, reply);
}


static void
xgen_parse_xmlxcb_file (XGenState * state, char *filename)
{
  xmlDoc *doc;
  xmlNode *root, *elem;
  char *extension_name;
  char *extension_header;
  XGenExtension *extension = NULL;

  /* Ignore text nodes consisting entirely of whitespace. */
  xmlKeepBlanksDefault (0);

  doc = xmlParseFile (filename);
  if (!doc)
    return;

  root = xmlDocGetRootElement (doc);
  if (!root)
    return;

  extension_name = xgen_xml_get_prop (root, "extension-name");
  if (!extension_name)
    extension_name = g_strdup ("Core");
  extension_header = xgen_xml_get_prop (root, "header");
  g_assert (extension_header);

  printf ("Extension: %s\n", extension_name);

  extension = g_new0 (XGenExtension, 1);
  extension->name = g_strdup (extension_name);
  extension->header = g_strdup (extension_header);
  state->extensions = g_list_prepend (state->extensions, extension);

  xmlFree (extension_name);

  if (strcmp (extension->header, "xproto") == 0)
    {
      int i;

      /* Add definitions of core types. */
      for (i = 0;
	   i < sizeof (core_type_definitions) / sizeof (XGenDefinition);
	   i++)
	{
	  XGenDefinition *temp = g_new0 (XGenDefinition, 1);

	  *temp = core_type_definitions[i];
	  temp->name = g_strdup (core_type_definitions[i].name);

	  state->definitions = g_list_prepend (state->definitions, temp);
	  extension->definitions =
	    g_list_prepend (extension->definitions, temp);
	}
    }

  for (elem = root->children;
       elem != NULL; elem = xgen_xml_next_elem (elem->next))
    {

      if (strcmp (xgen_xml_get_node_name (elem), "request") == 0)
	{
	  XGenRequest *request;
	  XGenDefinition *def;
	  XGenFieldDefinition *field;
	  XGenFieldDefinition *first_byte_field;
	  GList *fields;
	  int opcode = atoi (xgen_xml_get_prop (elem, "opcode"));

	  def = g_new0 (XGenDefinition, 1);
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_REQUEST;

	  fields = xgen_parse_field_elements (state, elem);
	  if (!fields)
	    {
	      field = g_new0 (XGenFieldDefinition, 1);
	      field->name = g_strdup ("pad");
	      field->definition = xgen_find_type (state, "CARD8");
	      fields = g_list_prepend (fields, field);
	    }

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("length");
	  field->definition = xgen_find_type (state, "CARD16");
	  fields = g_list_prepend (fields, field);

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("opcode");
	  field->definition = xgen_find_type (state, "BYTE");
	  fields = g_list_prepend (fields, field);

	  def->fields = fields;

	  fields = xgen_parse_reply_fields (state, elem);
	  if (fields)
	    {
	      /* FIXME: assert that sizeof(first_byte_field)==1 */
	      first_byte_field = fields->data;
	      fields = g_list_remove (fields, first_byte_field);

	      field = g_new0 (XGenFieldDefinition, 1);
	      field->name = g_strdup ("length");
	      field->definition = xgen_find_type (state, "CARD32");
	      fields = g_list_prepend (fields, field);

	      field = g_new0 (XGenFieldDefinition, 1);
	      field->name = g_strdup ("sequence");
	      field->definition = xgen_find_type (state, "CARD16");
	      fields = g_list_prepend (fields, field);

	      fields = g_list_prepend (fields, first_byte_field);

	      field = g_new0 (XGenFieldDefinition, 1);
	      field->name = g_strdup ("response_type");
	      field->definition = xgen_find_type (state, "BYTE");
	      fields = g_list_prepend (fields, field);
	    }

	  def->reply_fields = fields;

	  state->definitions = g_list_prepend (state->definitions, def);

	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	  request = g_new0 (XGenRequest, 1);
	  request->extension = extension;
	  request->opcode = opcode;
	  request->definition = def;
	  extension->requests =
	    g_list_prepend (extension->requests, request);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "event") == 0)
	{
	  XGenEvent *event;
	  char *no_sequence_number;
	  XGenDefinition *def;
	  XGenFieldDefinition *field;
	  XGenFieldDefinition *first_byte_field;
	  GList *fields;
	  int number = atoi (xgen_xml_get_prop (elem, "number"));

	  def = g_new0 (XGenDefinition, 1);
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_EVENT;

	  fields = xgen_parse_field_elements (state, elem);

	  first_byte_field = fields->data;
	  fields = g_list_remove (fields, first_byte_field);

	  no_sequence_number =
	    xgen_xml_get_prop (elem, "no-sequence-number");
	  if (!no_sequence_number)
	    {
	      field = g_new0 (XGenFieldDefinition, 1);
	      field->name = g_strdup ("sequence");
	      field->definition = xgen_find_type (state, "CARD16");
	      fields = g_list_prepend (fields, field);
	    }

	  fields = g_list_prepend (fields, first_byte_field);

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("response_type");
	  field->definition = xgen_find_type (state, "BYTE");
	  fields = g_list_prepend (fields, field);

	  def->fields = fields;

	  state->definitions = g_list_prepend (state->definitions, def);

	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	  event = g_new0 (XGenEvent, 1);
	  event->extension = extension;
	  event->number = number;
	  event->definition = def;
	  extension->events = g_list_prepend (extension->events, event);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "eventcopy") == 0)
	{
	  XGenEvent *event;
	  XGenDefinition *def;
	  int number = atoi (xgen_xml_get_prop (elem, "number"));
	  XGenDefinition *copy_of;

	  def = g_new0 (XGenDefinition, 1);
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_EVENT;
	  /* NB: This isn't a very nice thing to be doing if we
	   * are ever going to care about freeing resources!
	   * We should deep copy the fields instead. */
	  copy_of = xgen_find_type (state,
				       xgen_xml_get_prop (elem, "ref"));
	  def->fields = copy_of->fields;

	  state->definitions = g_list_prepend (state->definitions, def);

	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	  event = g_new0 (XGenEvent, 1);
	  event->extension = extension;
	  event->number = number;
	  event->definition = def;
	  extension->events = g_list_prepend (extension->events, event);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "error") == 0)
	{
	  XGenDefinition *def = g_new0 (XGenDefinition, 1);
	  int number = atoi (xgen_xml_get_prop (elem, "number"));
	  GList *fields;
	  XGenFieldDefinition *field;
	  XGenError *error;

	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_ERROR;

	  fields = xgen_parse_field_elements (state, elem);

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("response");
	  field->definition = xgen_find_type (state, "BYTE");
	  fields = g_list_prepend (fields, field);

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("error_code");
	  field->definition = xgen_find_type (state, "BYTE");
	  fields = g_list_prepend (fields, field);

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("sequence");
	  field->definition = xgen_find_type (state, "CARD16");
	  fields = g_list_prepend (fields, field);

	  def->fields = fields;

	  state->definitions = g_list_prepend (state->definitions, def);

	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	  error = g_new0 (XGenError, 1);
	  error->extension = extension;
	  error->number = number;
	  error->definition = def;
	  extension->errors = g_list_prepend (extension->errors, error);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "errorcopy") == 0)
	{
	  XGenError *error;
	  XGenDefinition *def;
	  int number = atoi (xgen_xml_get_prop (elem, "number"));
	  XGenDefinition *copy_of;

	  def = g_new0 (XGenDefinition, 1);
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_ERROR;
	  /* NB: This isn't a very nice thing to be doing if we
	   * are ever going to care about freeing resources!
	   * We should deep copy the fields instead. */
	  copy_of = xgen_find_type (state,
				     xgen_xml_get_prop (elem, "ref"));
	  def->fields = copy_of->fields;

	  state->definitions = g_list_prepend (state->definitions, def);

	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	  error = g_new0 (XGenError, 1);
	  error->extension = extension;
	  error->number = number;
	  error->definition = def;
	  extension->errors = g_list_prepend (extension->errors, error);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "struct") == 0)
	{
	  XGenDefinition *def = g_new0 (XGenDefinition, 1);

	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_STRUCT;
	  def->fields = xgen_parse_field_elements (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "xidunion") == 0)
	{
	  XGenDefinition *def = g_new0 (XGenDefinition, 1);

	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_XIDUNION;
	  def->fields = xgen_parse_field_elements (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "union") == 0)
	{
	  XGenDefinition *def = g_new0 (XGenDefinition, 1);

	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_UNION;
	  def->fields = xgen_parse_field_elements (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "xidtype") == 0)
	{
	  XGenDefinition *def = g_new0 (XGenDefinition, 1);

	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_XID;
	  def->size = 4;
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "enum") == 0)
	{
	  XGenDefinition *def = g_new0 (XGenDefinition, 1);
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_ENUM;

	  def->items = xgen_parse_item_elements (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "typedef") == 0)
	{
	  XGenDefinition *def = g_new0 (XGenDefinition, 1);

	  def->name = g_strdup (xgen_xml_get_prop (elem, "newname"));
	  def->type = XGEN_TYPEDEF;
	  def->reference = xgen_find_type (state,
				       xgen_xml_get_prop (elem, "oldname"));
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "import") == 0)
	{
	}
    }

  extension->definitions = g_list_reverse (extension->definitions);
}

static long
xgen_evaluate_expression (XGenExpression * expression,
			  GList *field_values)
{
  switch (expression->type)
    {
    case XGEN_VALUE:
      return expression->value;

    case XGEN_FIELDREF:
      {
	GList *tmp;
	for (tmp = field_values; tmp != NULL; tmp = tmp->next)
	  {
	    XGenFieldValue *field_value = tmp->data;
	    if (strcmp (field_value->field->name, expression->field) == 0)
	      switch (field_value->field->definition->type)
		{
		case XGEN_BOOLEAN:
		  return field_value->bool_value;
		case XGEN_CHAR:
		  return field_value->char_value;
		case XGEN_SIGNED:
		  return field_value->signed_value;
		case XGEN_UNSIGNED:
		case XGEN_XID:
		  return field_value->unsigned_value;
		default:
		  g_error ("un-expected fieldref type");
		}
	  }
	g_error ("Failed to find expresion's fieldref=\"%s\"\n",
		expression->field);
      }

    case XGEN_OP:
      {
	long left =
	  xgen_evaluate_expression (expression->left, field_values);
	long right =
	  xgen_evaluate_expression (expression->right, field_values);
	switch (expression->op)
	  {
	  case XGEN_ADD:
	    return left + right;
	  case XGEN_SUBTRACT:
	    return left - right;
	  case XGEN_MULTIPLY:
	    return left * right;
	    /* FIXME: divide by zero */
	  case XGEN_DIVIDE:
	    return left / right;
	  case XGEN_LEFT_SHIFT:
	    return left << right;
	  case XGEN_BITWISE_AND:
	    return left & right;
	  }
      }
    default:
      g_error ("un-handled operator type!");
    }

  g_error ("Failed to parse expression!");
  return 0;
}

/**
 * xgen_parse_xcb_proto_files:
 * @files: A list of xcb xml protocol descriptions
 *
 * This function takes a list of protocol description files and parses
 * them in to structures that can be accessed via the returned XGenState
 * pointer. Note: currently the files are parsed in the order listed and
 * its your responsability to list in order of inter-dependencies, with
 * xproto.xml first.
 */
static XGenState *
xgen_parse_xcb_proto_files (GList *files)
{
  XGenState *state = g_new0 (XGenState, 1);
  unsigned long l = 1;
  GList *tmp;

  state->host_is_little_endian = *(unsigned char *)&l ? TRUE : FALSE;

  for (tmp = files; tmp != NULL; tmp = tmp->next)
    xgen_parse_xmlxcb_file (state, tmp->data);

  state->definitions = g_list_reverse (state->definitions);

  /* Make sure that xproto is listed first */
  state->extensions = g_list_reverse (state->extensions);

  return state;
}

static char *
gxgen_get_lowercase_name (const char *name)
{
  gint pos;
  GString *new_name;
  gchar *tmp, *ret;

  g_return_val_if_fail (name && name[0], NULL);
  new_name = g_string_new (name);

  for (pos = 0; new_name->str[pos + 1]; pos++)
    {
      if (!new_name->str[pos] || !new_name->str[pos + 1])
	break;

      if ((g_ascii_islower (new_name->str[pos])
	   && g_ascii_isupper (new_name->str[pos + 1]))
	  || (g_ascii_isalpha (new_name->str[pos])
	      && g_ascii_isdigit (new_name->str[pos + 1])))
	{
	  new_name = g_string_insert_c (new_name, pos + 1, '_');
	  pos++;
	}
    }

  tmp = g_string_free (new_name, FALSE);
  ret = g_ascii_strdown (tmp, -1);
  g_free (tmp);

  return ret;
}

static char *
gxgen_get_uppercase_name (const char *name)
{
  char *name_lc = gxgen_get_lowercase_name (name);
  char *name_uc = g_ascii_strup (name_lc, -1);
  g_free (name_lc);
  return name_uc;
}

static char *
gxgen_get_camelcase_name (const char *name)
{
  char *name_cc = g_strdup (name);
  int i;
  char *str;

  g_return_val_if_fail (name && name[0], NULL);

  for (i = 0; camelcase_dictionary[i].uppercase; i++)
    while ((str = strstr (name_cc, camelcase_dictionary[i].uppercase)))
      memcpy (str, camelcase_dictionary[i].camelcase,
	      strlen(camelcase_dictionary[i].uppercase));

  /* At the very least; if a lower case name is passed then we
   * uppercase the first letter: */
  /* FIXME - this should also use a dictionary */
  if (g_ascii_islower (name_cc[0]))
    name_cc[0] = g_ascii_toupper (name_cc[0]);

  return name_cc;
}

static char *
gxgen_definition_to_gx_type (XGenDefinition * definition,
			     gboolean object_types)
{
  struct _TypeMapping *mapping;
  char *name_cc;
  char *ret;

  for (mapping = core_type_mappings; mapping->from != NULL; mapping++)
    if (strcmp (mapping->from, definition->name) == 0)
      return g_strdup (mapping->to);

  if (strcmp (definition->name, "WINDOW") == 0)
    return g_strdup (object_types ? "GXWindow *" : "guint32");
  else if (strcmp (definition->name, "DRAWABLE") == 0)
    return g_strdup (object_types ? "GXDrawable *" : "guint32");
  else if (strcmp (definition->name, "PIXMAP") == 0)
    return g_strdup (object_types ? "GXPixmap *" : "guint32");
  else if (strcmp (definition->name, "GCONTEXT") == 0)
    return g_strdup (object_types ? "GXGContext *" : "guint32");

  name_cc = gxgen_get_camelcase_name (definition->name);
  ret = g_strdup_printf ("GX%s", name_cc);
  g_free (name_cc);

  return ret;
}

static void
out (GXGenOutputContext *output_context,
     GXGenPart part,
     const char *format, ...)
{
  va_list ap;

  va_start (ap, format);
  g_string_append_vprintf (output_context->parts[part], format, ap);
  va_end (ap);
}

/**
 * out2:
 *
 * This is a convenience function that can output to two gstring parts. This
 * can be handy for function prototypes that need to be emitted to a header
 * file and a C file.
 */
static void
out2 (GXGenOutputContext *output_context,
      GXGenPart part0,
      GXGenPart part1,
      const char *format, ...)
{
  va_list ap;

  va_start (ap, format);
  g_string_append_vprintf (output_context->parts[part0], format, ap);
  g_string_append_vprintf (output_context->parts[part1], format, ap);
  va_end (ap);
}

static void
output_pad_field (GXGenOutputContext *output_context,
		  GXGenPart part,
		  XGenFieldDefinition *field,
		  int index)
{
  char *type_name_cc;
  char *pad_name;

  if (field->length->value == 1)
    pad_name = g_strdup_printf ("pad%u", index);
  else
    pad_name = g_strdup_printf ("pad%u[%lu]", index, field->length->value);

  type_name_cc = gxgen_definition_to_gx_type (field->definition, FALSE);
  OUT (part, "\t%s %s;\n",
       type_name_cc, pad_name);
  g_free (type_name_cc);
  g_free (pad_name);
}

static void
output_field_xcb_reference (GXGenOutputContext *output_context,
			    GXGenPart part,
			    XGenFieldDefinition * field)
{
  if (strcmp (field->definition->name, "DRAWABLE") == 0)
    {
      OUT (part, "gx_drawable_get_xid (%s)", field->name);
    }
  else if (strcmp (field->definition->name, "PIXMAP") == 0
	   || strcmp (field->definition->name, "WINDOW") == 0)
    {
      OUT (part,
	   "gx_drawable_get_xid (\n"
	   "\t\t\t\tGX_DRAWABLE(%s))", field->name);
    }
  else if (strcmp (field->definition->name, "GCONTEXT") == 0)
    {
      OUT (part, "gx_gcontext_get_xcb_gcontext (%s)", field->name);
    }
  else
    OUT (part, "%s", field->name);
}

static gboolean
is_special_xid_definition (XGenDefinition *definition)
{
  if (strcmp (definition->name, "DRAWABLE") == 0
      || strcmp (definition->name, "PIXMAP") == 0
      || strcmp (definition->name, "WINDOW") == 0
      || strcmp (definition->name, "GCONTEXT") == 0)
    return TRUE;
  else
    return FALSE;
}

static void
output_typedefs (GXGenOutputContext *output_context)
{
  const XGenExtension *extension = output_context->extension;
  GList *tmp;
  char *typedef_type_cc;
  char *typedef_name_cc;

  for (tmp = extension->definitions; tmp != NULL; tmp = tmp->next)
    {
      XGenDefinition *def = tmp->data;

      if (def->type == XGEN_BOOLEAN
	  || def->type == XGEN_CHAR
	  || def->type == XGEN_SIGNED || def->type == XGEN_UNSIGNED)
	{

	  if (strcmp (def->name, "char") == 0)
	    continue;

	  typedef_type_cc = gxgen_definition_to_gx_type (def, FALSE);
	  typedef_name_cc = gxgen_get_camelcase_name (def->name);

	  OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef %s %s;\n", typedef_type_cc, typedef_name_cc);

	  g_free (typedef_type_cc);
	  g_free (typedef_name_cc);
	}
      else if (def->type == XGEN_XID || def->type == XGEN_XIDUNION)
	{
	  if (is_special_xid_definition (def))
	    continue;

	  typedef_name_cc = gxgen_definition_to_gx_type (def, FALSE);

	  OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef guint32 %s;\n", typedef_name_cc);

	  g_free (typedef_name_cc);
	}
    }
  OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "\n");

  for (tmp = extension->definitions; tmp != NULL; tmp = tmp->next)
    {
      XGenDefinition *def = tmp->data;

      if (def->type == XGEN_TYPEDEF)
	{
	  typedef_type_cc = gxgen_get_camelcase_name (def->reference->name);
	  typedef_name_cc = gxgen_definition_to_gx_type (def, FALSE);

	  OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef %s %s;\n", typedef_type_cc, typedef_name_cc);

	  g_free (typedef_type_cc);
	  g_free (typedef_name_cc);
	}
    }
  OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "\n\n\n");


}

static void
output_structs_and_unions (GXGenOutputContext *output_context)
{
  const XGenExtension *extension = output_context->extension;
  GList *tmp;

  for (tmp = extension->definitions; tmp != NULL; tmp = tmp->next)
    {
      XGenDefinition *def = tmp->data;
      guint pad = 0;

      if (def->type == XGEN_STRUCT || def->type == XGEN_UNION)
	{
	  GList *tmp2;

	  /* Some types are special cased if they are represented as
	   * objects */
	  if (strcmp (def->name, "SCREEN") == 0)
	    continue;

	  OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef %s {\n",
	       def->type == XGEN_STRUCT ? "struct" : "union");

	  for (tmp2 = def->fields; tmp2 != NULL; tmp2 = tmp2->next)
	    {
	      XGenFieldDefinition *field = tmp2->data;
	      if (strcmp (field->name, "pad") == 0)
		{
		  output_pad_field (output_context,
				    GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
				    field,
				    pad++);
		}
	      else
		{
		  /* Dont print trailing list fields */
		  if (!(tmp2->next == NULL && field->length != NULL))
		    {
		      OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
			   "\t%s %s;\n",
			   gxgen_definition_to_gx_type (field->definition, FALSE),
			   field->name);
		    }
		}
	    }

	  OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "");
	  OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "} %s;\n\n", gxgen_definition_to_gx_type (def, FALSE));
	}
    }
  OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "\n");

}

static void
output_enums (GXGenOutputContext *output_context)
{
  const XGenExtension *extension = output_context->extension;
  GList *tmp;

  for (tmp = extension->definitions; tmp != NULL; tmp = tmp->next)
    {
      XGenDefinition *def = tmp->data;
      GList *tmp2;

      if (def->type != XGEN_ENUM)
	continue;

      OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	   "typedef enum\n{\n");

      for (tmp2 = def->items; tmp2 != NULL; tmp2 = tmp2->next)
	{
	  XGenItemDefinition *item = tmp2->data;
	  char *enum_stem_uc = gxgen_get_uppercase_name (item->name);
	  char *enum_prefix_uc = gxgen_get_uppercase_name (def->name);

	  if (item->type == XGEN_ITEM_AS_VALUE)
	    {
	      OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
		   " GX_%s_%s = %s,\n",
		   enum_prefix_uc,
		   enum_stem_uc,
		   item->value);
	    }
	  else
	    {
	      OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
		   " GX_%s_%s = (1 << %u),\n",
		   enum_prefix_uc,
		   enum_stem_uc,
		   item->bit);
	    }

	  g_free (enum_stem_uc);
	  g_free (enum_prefix_uc);
	}
      OUT (GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	   "} GX%s;\n\n", def->name);
    }
}

static GXGenOutputObject *
setup_output_object (XGenRequest *request)
{
  GXGenOutputObject *obj = g_new0(GXGenOutputObject, 1);
  GList *tmp;

  /* FIXME - this could be implemented using static
   * descriptions of the objects. */

  /* Look at the first field (after those that are ignored)
   * to identify what object this request belongs too.
   *
   * The default object for requests to become member of is
   * GXConnection.
   */
  if (g_list_length (request->definition->fields) >= 2)
    {
      for (tmp = request->definition->fields;
	   tmp != NULL; tmp = tmp->next)
	{
	  XGenFieldDefinition *field = tmp->data;

	  if (strcmp (field->name, "opcode") == 0
	      || strcmp (field->name, "pad") == 0
	      || strcmp (field->name, "length") == 0)
	    continue;

	  if (strcmp (field->name, "window") == 0)
	    {
	      obj->type = GXGEN_IS_WINDOW_OBJ;
	      obj->name_cc = "Window";
	      obj->first_arg = "GXWindow *window";
	      obj->first_object_field = field;
	      obj->h_typedefs = GXGEN_PART_WINDOW_OBJ_H_TYPEDEFS;
	      obj->h_protos = GXGEN_PART_WINDOW_OBJ_H_PROTOS;
	      obj->c_funcs = GXGEN_PART_WINDOW_OBJ_C_FUNCS;
	      break;
	    }
	  else if (strcmp (field->name, "pixmap") == 0)
	    {
	      obj->type = GXGEN_IS_PIXMAP_OBJ;
	      obj->name_cc = "Pixmap";
	      obj->first_arg = "GXPixmap *pixmap";
	      obj->first_object_field = field;
	      obj->h_typedefs = GXGEN_PART_PIXMAP_OBJ_H_TYPEDEFS;
	      obj->h_protos = GXGEN_PART_PIXMAP_OBJ_H_PROTOS;
	      obj->c_funcs = GXGEN_PART_PIXMAP_OBJ_C_FUNCS;
	      break;
	    }
	  else if (strcmp (field->name, "drawable") == 0)
	    {
	      obj->type = GXGEN_IS_DRAWABLE_OBJ;
	      obj->name_cc = "Drawable";
	      obj->first_arg = "GXDrawable *drawable";
	      obj->first_object_field = field;
	      obj->h_typedefs = GXGEN_PART_DRAWABLE_OBJ_H_TYPEDEFS;
	      obj->h_protos = GXGEN_PART_DRAWABLE_OBJ_H_PROTOS;
	      obj->c_funcs = GXGEN_PART_DRAWABLE_OBJ_C_FUNCS;
	      break;
	    }
	  else if (strcmp (field->name, "gc") == 0)
	    {
	      obj->type = GXGEN_IS_GCONTEXT_OBJ;
	      obj->name_cc = "GContext";
	      obj->first_arg = "GXGContext *gc";
	      obj->first_object_field = field;
	      obj->h_typedefs = GXGEN_PART_GCONTEXT_OBJ_H_TYPEDEFS;
	      obj->h_protos = GXGEN_PART_GCONTEXT_OBJ_H_PROTOS;
	      obj->c_funcs = GXGEN_PART_GCONTEXT_OBJ_C_FUNCS;
	      break;
	    }
	  //else
	  //  break;
	}
    }

  /* By default requests become members of the GXConnection
   * object */
  if (!obj->name_cc)
    {
      obj->type = GXGEN_IS_CONNECTION_OBJ;
      obj->name_cc = "Connection";
      obj->first_arg = "GXConnection *connection";
      obj->h_typedefs = GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS;
      obj->h_protos = GXGEN_PART_CONNECTION_OBJ_H_PROTOS;
      obj->c_funcs = GXGEN_PART_CONNECTION_OBJ_C_FUNCS;
    }

  obj->name_lc = gxgen_get_lowercase_name (obj->name_cc);
  obj->name_uc = gxgen_get_uppercase_name (obj->name_cc);

  return obj;
}

void
free_output_object (GXGenOutputObject *obj)
{
  g_free (obj->name_uc);
  g_free (obj);
}

static void
output_reply_typedef (GXGenOutputContext *output_context)
{
  const GXGenOutputRequest *out_request = output_context->out_request;
  const XGenRequest *request = out_request->request;
  const GXGenOutputObject *obj = output_context->obj;
  GList *tmp;
  guint pad = 0;

  if (g_list_length (request->definition->reply_fields) <= 1)
    return;

  OUT (obj->h_typedefs, "typedef struct {\n");

  for (tmp = request->definition->reply_fields;
       tmp != NULL; tmp = tmp->next)
    {
      XGenFieldDefinition *field = tmp->data;
      if (strcmp (field->name, "pad") == 0)
	output_pad_field (output_context, obj->h_typedefs, field, pad++);
      else
	{
	  /* Dont print trailing list members */
	  if (!(tmp->next == NULL && field->length != NULL))
	    {
	      OUT (obj->h_typedefs,
		   "\t%s %s;\n",
		   gxgen_definition_to_gx_type (field->definition, FALSE),
		   field->name);
	    }
	}
    }
  OUT (obj->h_typedefs,
       "\n} GX%s%sX11Reply;\n\n",
       output_context->namespace->gx_cc,
       out_request->gx_name_cc);

  OUT (obj->h_typedefs, "typedef struct {\n");
  OUT (obj->h_typedefs, "\tGXConnection *connection;\n");
  OUT (obj->h_typedefs, "\tGX%s%sX11Reply *x11_reply;\n",
       output_context->namespace->gx_cc,
       out_request->gx_name_cc);
  OUT (obj->h_typedefs,
       "\n} GX%s%sReply;\n\n",
       output_context->namespace->gx_cc,
       out_request->gx_name_cc);
}

static void
output_reply_list_get (GXGenOutputContext *output_context)
{
  const GXGenOutputRequest *out_request = output_context->out_request;
  const XGenRequest *request = out_request->request;
  const GXGenOutputObject *obj = output_context->obj;
  XGenFieldDefinition *field;

  if (!request->definition->reply_fields)
    return;

  field = (g_list_last (request->definition->reply_fields))->data;
  if (field->length == NULL)
    return;

  /* FIXME - shouldn't be restricted to FIELDREF length types */
  if (field->length->type != XGEN_FIELDREF)
    return;

  OUT2 (obj->h_protos,
        obj->c_funcs,
        "GArray *\n"
        "gx_%s%s_get_%s (GX%s%sReply *%s_reply)",
        output_context->namespace->gx_lc,
        out_request->gx_name_lc,
        field->name,
	output_context->namespace->gx_cc,
        out_request->gx_name_cc,
        out_request->gx_name_lc);

  OUT (obj->h_protos, ";\n");
  OUT (obj->c_funcs,
       "\n{\n");

  OUT (obj->c_funcs,
       "  %s *p = (%s *)(%s_reply->x11_reply + 1);\n",
       gxgen_definition_to_gx_type (field->definition, FALSE),
       gxgen_definition_to_gx_type (field->definition, FALSE),
       out_request->gx_name_lc);

  OUT (obj->c_funcs,
       "  GArray *tmp;\n");

  OUT (obj->c_funcs,
       "\n");

  OUT (obj->c_funcs,
       "/* It's possible the connection has been closed since the reply\n"
       " * was recieved: (the reply struct contains weak pointer) */\n"
       "  if (!%s_reply->connection)\n"
       "    return NULL;\n",
       out_request->gx_name_lc);

  if (is_special_xid_definition (field->definition))
    {
      OUT (obj->c_funcs,
       "  tmp = g_array_new (TRUE, FALSE, sizeof(void *));\n");
      OUT (obj->c_funcs,
       "  int i;\n");
    }
  else
    OUT (obj->c_funcs,
       "  tmp = g_array_new (TRUE, FALSE, sizeof(%s));\n",
       gxgen_definition_to_gx_type (field->definition, FALSE));

  if (is_special_xid_definition (field->definition))
    {
      OUT (obj->c_funcs,
       "  for (i = 0; i< %s_reply->x11_reply->%s; i++)\n"
       "    {\n",
       out_request->gx_name_lc,
       field->length->field);

      OUT (obj->c_funcs,
       "      /* FIXME - mutex */\n");

      OUT (obj->c_funcs,
       "      %s item = _gx_%s_find_from_xid (p[i]);\n",
       gxgen_definition_to_gx_type (field->definition, TRUE),
       obj->name_lc);

      OUT (obj->c_funcs,
       "      if (!item)\n"
       "	item = g_object_new (gx_%s_get_type(),\n"
       "			     \"connection\", %s_reply->connection,\n"
       "			     \"xid\", p[i],\n"
       "			     \"wrap\", TRUE,\n"
       "			     NULL);\n",
       obj->name_lc, out_request->gx_name_lc);

      OUT (obj->c_funcs,
       "      tmp = g_array_append_val (tmp, item);\n"
       "    }\n");
    }
  else
    {
      OUT (obj->c_funcs,
       "  tmp = g_array_append_vals (tmp, p, %s_reply->x11_reply->%s);\n",
       out_request->gx_name_lc, field->length->field);
    }

  OUT (obj->c_funcs,
       "  return tmp;\n");

  OUT (obj->c_funcs,
       "}\n");

}

static void
output_reply_list_free (GXGenOutputContext *output_context)
{
  const GXGenOutputRequest *out_request = output_context->out_request;
  const XGenRequest *request = out_request->request;
  const GXGenOutputObject *obj = output_context->obj;
  XGenFieldDefinition *field;

  if (!request->definition->reply_fields)
    return;

  field = (g_list_last (request->definition->reply_fields))->data;
  if (field->length == NULL)
    return;

  /* FIXME - shouldn't be restricted to FIELDREF length types */
  if (field->length->type != XGEN_FIELDREF)
    return;

  OUT2 (obj->h_protos,
	obj->c_funcs,
        "\nvoid\n"
        "gx_%s%s_free_%s (GArray *%s)",
        output_context->namespace->gx_lc,
        out_request->gx_name_lc,
        field->name,
        field->name);

  OUT (obj->h_protos, ";\n");
  OUT (obj->c_funcs,
       "\n{\n");

  if (is_special_xid_definition (field->definition))
    {
      OUT (obj->c_funcs,
       "\t%s*p = %s->data;\n"
       "\tint i;\n"
       "\tfor (i = 0; i < %s->len; i++)\n"
       "\t\tg_object_unref (p[i]);\n",
       gxgen_definition_to_gx_type (field->definition, TRUE),
       field->name,
       field->name);
    }

  OUT (obj->c_funcs, "\tg_array_free (%s, TRUE);\n", field->name);

  OUT (obj->c_funcs,
       "}\n");
}

static void
output_reply_free (GXGenOutputContext *output_context)
{
  const GXGenOutputRequest *out_request = output_context->out_request;
  const XGenRequest *request = out_request->request;
  const GXGenOutputObject *obj = output_context->obj;

  if (!request->definition->reply_fields)
    return;

  OUT2 (obj->h_protos,
        obj->c_funcs,
        "void\n"
        "gx_%s%s_reply_free (GX%s%sReply *%s_reply)",
        output_context->namespace->gx_lc,
        out_request->gx_name_lc,
	output_context->namespace->gx_cc,
        out_request->gx_name_cc,
        out_request->gx_name_lc);

  OUT (obj->h_protos, ";\n");
  OUT (obj->c_funcs,
       "\n{\n");

  OUT (obj->c_funcs,
       "  free (%s_reply->x11_reply);\n",
       out_request->gx_name_lc);
  OUT (obj->c_funcs,
       "  g_slice_free (GX%s%sReply, %s_reply);\n",
       output_context->namespace->gx_cc,
       out_request->gx_name_cc,
       out_request->gx_name_lc);

  OUT (obj->c_funcs,
       "}\n");
}

static void
output_mask_value_variable_declarations (GXGenOutputContext *output_context,
					 GXGenPart part)
{
  OUT (part,
       "\tguint32 value_list_len = "
	  "gx_mask_value_items_get_count (mask_value_items);\n");
  OUT (part,
       "\tguint32 *value_list = "
	  "alloca (value_list_len * 4);\n");
  OUT (part,
	"\tguint32 value_mask;\n");
  OUT (part,
	"\n");

  OUT (part,
       "\tgx_mask_value_items_get_list (mask_value_items, "
	  "&value_mask, value_list);\n");
}

/**
 * output_reply_variable_definitions:
 * @part: The particular stream you to output too
 * @request: The request to which you will be replying
 *
 * This function outputs the variable declarations needed
 * for preparing a reply. This should be called in the
 * *_reply () funcs that take a cookie or the synchronous
 * request functions.
 */
static void
output_reply_variable_declarations (GXGenOutputContext *output_context)
{
  const GXGenOutputRequest *out_request = output_context->out_request;
  const XGenRequest *request = out_request->request;
  const GXGenOutputObject *obj = output_context->obj;

  OUT (obj->c_funcs,
       "\txcb_generic_error_t *xcb_error;\n");

  if (g_list_length (request->definition->reply_fields) > 1)
    {
      OUT (obj->c_funcs,
	   "\tGX%s%sReply *reply = g_slice_new (GX%s%sReply);\n",
	   output_context->namespace->gx_cc,
	   out_request->gx_name_cc,
	   output_context->namespace->gx_cc,
	   out_request->gx_name_cc);
    }
}

/**
 * output_async_request:
 *
 * This function outputs the code for all gx_*_async () functions
 */
void
output_async_request (GXGenOutputContext *output_context)
{
  const GXGenOutputRequest *out_request = output_context->out_request;
  const XGenRequest *request = out_request->request;
  const GXGenOutputObject *obj = output_context->obj;
  GList *tmp;
  char *cookie_type_uc;
  gboolean has_mask_value_items = FALSE;

  OUT2 (obj->h_protos,
	obj->c_funcs,
	"\nGXCookie *\ngx_%s%s_async (%s",
        output_context->namespace->gx_lc,
        out_request->gx_name_lc,
        obj->first_arg);

  for (tmp = request->definition->fields; tmp != NULL; tmp = tmp->next)
    {
      XGenFieldDefinition *field = tmp->data;
      const char *type;

      if (strcmp (field->name, "opcode") == 0
	  || strcmp (field->name, "pad") == 0
	  || strcmp (field->name, "length") == 0)
	continue;

      if (obj->first_object_field && field == obj->first_object_field)
	continue;

      if ((obj->type == GXGEN_IS_WINDOW_OBJ
	   && strcmp (field->name, "window") == 0)
	  || (obj->type == GXGEN_IS_DRAWABLE_OBJ
	      && strcmp (field->name, "drawable") == 0))
	continue;

      type = gxgen_definition_to_gx_type (field->definition, TRUE);

      if (field->length)
	OUT2 (obj->h_protos, obj->c_funcs,
	      ",\n\t\tconst %s *%s", type, field->name);
      else if (field->definition->type == XGEN_VALUEPARAM)
	{
	  OUT2 (obj->h_protos, obj->c_funcs,
	        ",\n\t\tGXMaskValueItem *mask_value_items");
	  has_mask_value_items = TRUE;
	}
      else
	OUT2 (obj->h_protos, obj->c_funcs,
	      ",\n\t\t%s %s", type, field->name);
    }
  OUT (obj->h_protos, ");\n\n");
  OUT (obj->c_funcs, ")\n{\n");

  /*
   * *_async() code
   */
  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    {
      OUT (obj->c_funcs,
	   "\tGXConnection *connection = gx_%s_get_connection (%s);\n",
	   obj->name_lc, obj->name_lc);
    }

  if (g_list_length (request->definition->reply_fields) <= 1)
    OUT (obj->c_funcs,
	 "\txcb_void_cookie_t xcb_cookie;\n");
  else
    OUT (obj->c_funcs,
	 "\txcb_%s%s_cookie_t xcb_cookie;\n",
	 output_context->namespace->xcb_lc,
	 out_request->xcb_name_lc);
  OUT (obj->c_funcs,
	 "\tGXCookie *cookie;\n\n");

  if (has_mask_value_items)
    output_mask_value_variable_declarations (output_context, obj->c_funcs);

  OUT (obj->c_funcs, "\n");

  if (g_list_length (request->definition->reply_fields) > 1)
    {
      OUT (obj->c_funcs,
	   "\txcb_cookie =\n"
	   "\t\txcb_%s%s(\n"
	   "\t\t\tgx_connection_get_xcb_connection (connection)",
	   output_context->namespace->xcb_lc,
	   out_request->xcb_name_lc);
    }
  else
    {
      OUT (obj->c_funcs,
	   "\txcb_cookie =\n"
	   "\t\txcb_%s%s_checked (\n"
	   "\t\t\tgx_connection_get_xcb_connection (connection)",
	   output_context->namespace->xcb_lc,
	   out_request->xcb_name_lc);
    }

  for (tmp = request->definition->fields; tmp != NULL;
       tmp = tmp->next)
    {
      XGenFieldDefinition *field = tmp->data;

      if (strcmp (field->name, "opcode") == 0
	  || strcmp (field->name, "pad") == 0
	  || strcmp (field->name, "length") == 0)
	continue;

      if (field->definition->type != XGEN_VALUEPARAM)
	{
	  /* Some special cased field types require a function call
	   * to lookup their counterpart xcb value */
	  OUT (obj->c_funcs, ",\n\t\t\t");
	  output_field_xcb_reference (output_context, obj->c_funcs, field);
	}
      else
	{
	  OUT (obj->c_funcs, ",\n\t\t\tvalue_mask");
	  OUT (obj->c_funcs, ",\n\t\t\tvalue_list");
	}
    }
  OUT (obj->c_funcs, ");\n\n");

  cookie_type_uc =
    g_strdup_printf ("GX_COOKIE_%s%s",
		     output_context->namespace->gx_uc,
		     out_request->gx_name_uc);

  OUT (GXGEN_PART_COOKIE_OBJ_H_TYPEDEFS,
       "\t%s,\n", cookie_type_uc);


  OUT (obj->c_funcs,
       "\tcookie = gx_cookie_new (connection, %s, xcb_cookie.sequence);\n",
       cookie_type_uc);

  g_free (cookie_type_uc);

  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    OUT (obj->c_funcs, "\tg_object_unref (connection);\n");

  OUT (obj->c_funcs,
       "\tgx_connection_register_cookie (connection, cookie);\n");

  OUT (obj->c_funcs, "\treturn cookie;\n");

  OUT (obj->c_funcs, "}\n");
}

void
output_reply (GXGenOutputContext *output_context)
{
  const GXGenOutputRequest *out_request = output_context->out_request;
  const XGenRequest *request = out_request->request;
  const GXGenOutputObject *obj = output_context->obj;

  if (g_list_length (request->definition->reply_fields) > 1)
    {
      OUT2 (obj->h_protos, obj->c_funcs,
	    "\nGX%s%sReply *\n",
	    output_context->namespace->gx_cc,
	    out_request->gx_name_cc);
    }
  else
    OUT2 (obj->h_protos, obj->c_funcs, "\ngboolean\n");

  OUT2 (obj->h_protos, obj->c_funcs,
        "gx_%s%s_reply (GXCookie *cookie, GError **error)\n",
        output_context->namespace->gx_lc, out_request->gx_name_lc);

  OUT (obj->h_protos, ";\n");
  OUT (obj->c_funcs,
       "\n{\n");

  OUT (obj->c_funcs,
       "\tGXConnection *connection = gx_cookie_get_connection (cookie);\n");

  if (g_list_length (request->definition->reply_fields) <= 1)
    OUT (obj->c_funcs, "\txcb_void_cookie_t xcb_cookie;\n");
  else
    {
      OUT (obj->c_funcs, "\txcb_%s%s_cookie_t xcb_cookie;\n",
	   output_context->namespace->xcb_lc, out_request->xcb_name_lc);
    }
  output_reply_variable_declarations (output_context);
  OUT (obj->c_funcs, "\n");

  OUT (obj->c_funcs,
       "\tg_return_val_if_fail (error == NULL || *error == NULL, %s);\n",
       g_list_length (request->definition->reply_fields)
	  <= 1 ? "FALSE" : "NULL");

  OUT (obj->c_funcs, "\n");

  if (g_list_length (request->definition->reply_fields) > 1)
    OUT (obj->c_funcs,
	 "\treply->connection = connection;\n\n");

  if (g_list_length (request->definition->reply_fields) > 1)
    {
      OUT (obj->c_funcs,
	   "\treply->x11_reply = (GX%s%sX11Reply *)\n"
	   "\t\tgx_cookie_get_reply (cookie);\n",
	   output_context->namespace->gx_cc,
	   out_request->gx_name_cc);

      OUT (obj->c_funcs,
	   "\tif (!reply->x11_reply)\n"
	   "\t  {\n");
    }

  /* If the cookie doesn't have an associated reply, then we see
   * first see if it has an associated error instead.
   */
  /* FIXME - we need a mechanism for translating X errors into a glib
   * error domain, code and message. */
  OUT (obj->c_funcs,
      "\txcb_error = gx_cookie_get_error (cookie);\n");
  /* FIXME create a func for outputing this... */
  OUT (obj->c_funcs,
       "\tif (xcb_error)\n"
       "\t  {\n"
       "\t\tg_set_error (error,\n"
       "\t\t\tGX_PROTOCOL_ERROR,\n"
       "\t\t\tgx_protocol_error_from_xcb_generic_error (xcb_error),\n"
       "\t\t\t\"Protocol Error\");\n"
       "\t\treturn %s;\n"
       "\t  }\n",
       g_list_length (request->definition->reply_fields)
       > 1 ? "NULL" : "FALSE");
  /* FIXME - free reply */
  /* FIXME - check we don't skip any other function cleanup */

  OUT (obj->c_funcs,
       "\txcb_cookie.sequence = gx_cookie_get_sequence (cookie);\n");

  /* If the cookie has no associated reply or error, then we ask
   * XCB for a reply/error
   */
  if (g_list_length (request->definition->reply_fields) > 1)
    {
      OUT (obj->c_funcs,
	   "\treply->x11_reply = (GX%s%sX11Reply *)\n"
	   "\t\txcb_%s%s_reply (\n"
	   "\t\t\tgx_connection_get_xcb_connection (connection),\n"
	   "\t\t\txcb_cookie,\n"
	   "\t\t\t&xcb_error);\n",
	   output_context->namespace->gx_cc,
	   out_request->gx_name_cc,
	   output_context->namespace->xcb_lc,
	   out_request->xcb_name_lc);
    }
  else
    {
      OUT (obj->c_funcs,
	   "\txcb_error = \n"
	   "\t\txcb_request_check (\n"
	   "\t\t\tgx_connection_get_xcb_connection (connection),\n"
	   "\t\t\txcb_cookie);\n");
    }

  OUT (obj->c_funcs,
       "\tif (xcb_error)\n"
       "\t  {\n"
       "\t\tg_set_error (error,\n"
       "\t\t\tGX_PROTOCOL_ERROR,\n"
       "\t\t\tgx_protocol_error_from_xcb_generic_error (xcb_error),\n"
       "\t\t\t\"Protocol Error\");\n"
       "\t\treturn %s;\n"
       "\t  }\n",
       g_list_length (request->definition->reply_fields)
       > 1 ? "NULL" : "FALSE");


  if (g_list_length (request->definition->reply_fields) > 1)
    OUT (obj->c_funcs,
	 "\n\t  }\n");

  OUT (obj->c_funcs,
       "\tgx_connection_unregister_cookie (connection, cookie);\n");

  if (g_list_length (request->definition->reply_fields) <= 1)
    OUT (obj->c_funcs, "\treturn TRUE;\n");
  else
    OUT (obj->c_funcs, "\treturn reply;\n");

  OUT (obj->c_funcs,
       "}\n");
}

void
output_sync_request (GXGenOutputContext *output_context)
{
  const GXGenOutputRequest *out_request = output_context->out_request;
  const XGenRequest *request = out_request->request;
  const GXGenOutputObject *obj = output_context->obj;
  gboolean has_mask_value_items = FALSE;
  GList *tmp;

  if (g_list_length (request->definition->reply_fields) <= 1)
    OUT2 (obj->h_protos, obj->c_funcs, "\ngboolean\n");
  else
    {
      OUT2 (obj->h_protos, obj->c_funcs,
	    "\nGX%s%sReply *\n",
	    output_context->namespace->gx_cc,
	    out_request->gx_name_cc);
    }

  OUT2 (obj->h_protos, obj->c_funcs,
        "gx_%s%s (%s",
        output_context->namespace->gx_lc, out_request->gx_name_lc, obj->first_arg);

  for (tmp = request->definition->fields;
       tmp != NULL; tmp = tmp->next)
    {
      XGenFieldDefinition *field = tmp->data;
      const char *type;

      if (strcmp (field->name, "opcode") == 0
	  || strcmp (field->name, "pad") == 0
	  || strcmp (field->name, "length") == 0)
	continue;

      if (obj->first_object_field && field == obj->first_object_field)
	continue;

      type = gxgen_definition_to_gx_type (field->definition, TRUE);

      if (field->length)
	{
	  OUT2 (obj->h_protos, obj->c_funcs,
	        ",\n\t\tconst %s *%s", type, field->name);
	}
      else if (field->definition->type == XGEN_VALUEPARAM)
	{
	  OUT2 (obj->h_protos, obj->c_funcs,
	        ",\n\t\tGXMaskValueItem *mask_value_items");
	  has_mask_value_items = TRUE;
	}
      else
	OUT2 (obj->h_protos, obj->c_funcs,
	      ",\n\t\t%s %s", type, field->name);
    }
  OUT2 (obj->h_protos, obj->c_funcs, ",\n\t\tGError **error)");

  OUT (obj->h_protos, ";\n\n");
  OUT (obj->c_funcs, "\n{\n");

  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    {
      OUT (obj->c_funcs,
	   "\tGXConnection *connection = gx_%s_get_connection (%s);\n",
	   obj->name_lc, obj->name_lc);
    }

  if (g_list_length (request->definition->reply_fields) <= 1)
    OUT (obj->c_funcs, "\txcb_void_cookie_t cookie;\n");
  else
    OUT (obj->c_funcs, "\txcb_%s%s_cookie_t cookie;\n",
	 output_context->namespace->xcb_lc, out_request->xcb_name_lc);
  output_reply_variable_declarations (output_context);

  /* If the request has a XGEN_VALUEPARAM field, then we will need
   * to translate an array of GXMaskValueItems from the user.
   */
  if (has_mask_value_items)
    output_mask_value_variable_declarations (output_context, obj->c_funcs);

  OUT (obj->c_funcs, "\n");
  if (g_list_length (request->definition->reply_fields) <= 1)
    OUT (obj->c_funcs,
	 "\tg_return_val_if_fail (error == NULL || *error == NULL, FALSE);\n");
  else
    OUT (obj->c_funcs,
	 "\tg_return_val_if_fail (error == NULL || *error == NULL, NULL);\n");

  if (g_list_length (request->definition->reply_fields) > 1)
    OUT (obj->c_funcs,
	 "\treply->connection = connection;\n\n");

  if (g_list_length (request->definition->reply_fields) > 1)
    {
      OUT (obj->c_funcs,
	   "\tcookie =\n"
	   "\t\txcb_%s%s (\n",
	   output_context->namespace->xcb_lc,
	   out_request->xcb_name_lc);
    }
  else
    {
      OUT (obj->c_funcs,
	   "\tcookie =\n"
	   "\t\txcb_%s%s_checked (\n",
	   output_context->namespace->xcb_lc,
	   out_request->xcb_name_lc);
    }
  OUT (obj->c_funcs,
       "\t\t\tgx_connection_get_xcb_connection (connection)");

  for (tmp = request->definition->fields; tmp != NULL;
       tmp = tmp->next)
    {
      XGenFieldDefinition *field = tmp->data;

      if (strcmp (field->name, "opcode") == 0
	  || strcmp (field->name, "pad") == 0
	  || strcmp (field->name, "length") == 0)
	continue;

      if (field->definition->type != XGEN_VALUEPARAM)
	{
	  /* Some special cased field types require a function call
	   * to lookup their counterpart xcb value */
	  OUT (obj->c_funcs, ",\n\t\t\t");
	  output_field_xcb_reference (output_context, obj->c_funcs, field);
	}
      else
	{
	  OUT (obj->c_funcs, ",\n\t\t\tvalue_mask");
	  OUT (obj->c_funcs, ",\n\t\t\tvalue_list");
	}
    }
  OUT (obj->c_funcs, ");\n\n");

  if (g_list_length (request->definition->reply_fields) > 1)
    {
      OUT (obj->c_funcs,
	   "\treply->x11_reply = (GX%s%sX11Reply *)\n"
	   "\t\txcb_%s%s_reply (\n"
	   "\t\t\tgx_connection_get_xcb_connection (connection),\n"
	   "\t\t\tcookie,\n"
	   "\t\t\t&xcb_error);\n",
	   output_context->namespace->gx_cc,
	   out_request->gx_name_cc,
	   output_context->namespace->xcb_lc,
	   out_request->xcb_name_lc);
    }
  else
    {
      OUT (obj->c_funcs,
	   "\txcb_error = \n"
	   "\t\txcb_request_check (\n"
	   "\t\t\tgx_connection_get_xcb_connection (connection),\n"
	   "\t\t\tcookie);\n");
    }

  /* FIXME create a func for outputing this... */
  OUT (obj->c_funcs,
       "\tif (xcb_error)\n"
       "\t  {\n"
       "\t\tg_set_error (error,\n"
       "\t\t\tGX_PROTOCOL_ERROR,\n"
       "\t\t\tgx_protocol_error_from_xcb_generic_error (xcb_error),\n"
       "\t\t\t\"Protocol Error\");\n"
       "\t\treturn %s;\n"
       "\t  }\n",
       g_list_length (request->definition->reply_fields)
       > 1 ? "NULL" : "FALSE");

  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    OUT (obj->c_funcs, "\tg_object_unref (connection);\n");

  if (g_list_length (request->definition->reply_fields) <= 1)
    OUT (obj->c_funcs, "\n\treturn TRUE;\n");
  else
    OUT (obj->c_funcs, "\n\treturn reply;\n");

  OUT (obj->c_funcs, "}\n\n");
}

GXGenOutputNamespace *
setup_request_namespace (GXGenOutputContext *output_context)
{
  const XGenExtension *ext = output_context->extension;
  const GXGenOutputObject *obj = output_context->obj;
  GXGenOutputNamespace *namespace = g_new0 (GXGenOutputNamespace, 1);

  if (strcmp (ext->header, "xproto") == 0)
    {
      namespace->gx_cc = g_strdup (obj->name_cc);
      namespace->gx_uc = g_strdup_printf ("%s_", obj->name_uc);
      namespace->gx_lc = g_strdup_printf ("%s_", obj->name_lc);
    }
  else
    {
      namespace->gx_cc =
	g_strdup_printf ("%s%s",
			 obj->name_cc,
			 gxgen_get_camelcase_name (ext->header));
      namespace->gx_uc =
	g_strdup_printf ("%s_%s_",
			 obj->name_uc,
			 gxgen_get_uppercase_name (ext->header));
      namespace->gx_lc =
	g_strdup_printf ("%s_%s_",
			 obj->name_lc,
			 gxgen_get_lowercase_name (ext->header));
    }

  if (strcmp (ext->header, "xproto") == 0)
    namespace->xcb_lc = g_strdup ("");
  else
    {
      namespace->xcb_lc =
	g_strdup_printf ("%s_",
			 gxgen_get_lowercase_name (ext->header));
    }

  return namespace;
}

static void
free_namespace (GXGenOutputNamespace *namespace)
{
  g_free (namespace->gx_cc);
  g_free (namespace->gx_uc);
  g_free (namespace->gx_lc);
  g_free (namespace->xcb_cc);
  g_free (namespace->xcb_uc);
  g_free (namespace->xcb_lc);
  g_free (namespace);
}

static void
output_requests (GXGenOutputContext *output_context)
{
  const XGenExtension *extension = output_context->extension;
  GList *tmp;

  for (tmp = extension->requests; tmp != NULL; tmp = tmp->next)
    {
      XGenRequest *request = tmp->data;
      GXGenOutputRequest *out_request;
      GXGenOutputObject *obj = NULL;
      GXGenOutputNamespace *namespace;
      gchar *gx_name;

      if (strcmp (request->definition->name, "RotateProperties") == 0)
	g_print ("DEBUG\n");

      /* Some constructor type requests are special cased as object
       * constructors */
      if (strcmp (request->definition->name, "CreateWindow") == 0
	  || strcmp (request->definition->name, "CreatePixmap") == 0
	  || strcmp (request->definition->name, "CreateGC") == 0)
	continue;

      out_request = g_new0 (GXGenOutputRequest, 1);
      out_request->request = request;

      obj = setup_output_object (request);
      output_context->obj = obj;

      namespace = setup_request_namespace (output_context);
      output_context->namespace = namespace;

      /* the get/set_property names clash with the gobject
       * property accessor functions */
      if (strcmp (request->definition->name, "GetProperty") == 0)
	gx_name = g_strdup ("GetXProperty");
      else if (strcmp (request->definition->name, "SetProperty") == 0)
	gx_name = g_strdup ("SetXProperty");
      else
	gx_name = g_strdup (request->definition->name);

      out_request->gx_name_cc = gx_name;
      out_request->gx_name_lc =
	gxgen_get_lowercase_name (out_request->gx_name_cc);
      out_request->gx_name_uc =
	gxgen_get_uppercase_name (out_request->gx_name_cc);

      out_request->xcb_name_cc = g_strdup (request->definition->name);
      out_request->xcb_name_lc =
	gxgen_get_lowercase_name (out_request->xcb_name_cc);
      out_request->xcb_name_uc =
	gxgen_get_uppercase_name (out_request->xcb_name_cc);

      output_context->out_request = out_request;

      /* If the request has a reply definition then we typedef
       * the reply struct.
       */
      output_reply_typedef (output_context);

      /* Some replys include a list of data. If this is such a request
       * then we output a getter function for trailing list fields */
      output_reply_list_get (output_context);
      output_reply_list_free (output_context);

      output_reply_free (output_context);

      output_async_request (output_context);
      output_reply (output_context);

      output_sync_request (output_context);


      free_namespace (namespace);
      free_output_object (obj);
      g_free ((char *)out_request->xcb_name_cc);
      g_free ((char *)out_request->xcb_name_lc);
      g_free ((char *)out_request->xcb_name_uc);
      g_free ((char *)out_request->gx_name_cc);
      g_free ((char *)out_request->gx_name_lc);
      g_free ((char *)out_request->gx_name_uc);
      g_free (out_request);
    }
}

static void
output_event_typedefs (GXGenOutputContext *output_context)
{
  const XGenExtension *extension = output_context->extension;
  GXGenPart typedefs = GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS;
  GList *tmp;
  guint pad = 0;

  for (tmp = extension->events; tmp != NULL; tmp = tmp->next)
    {
      XGenEvent *event = tmp->data;
      GList *tmp2;

      OUT (typedefs, "\ntypedef struct {\n");

      for (tmp2 = event->definition->fields; tmp2 != NULL; tmp2 = tmp2->next)
	{
	  XGenFieldDefinition *field = tmp2->data;
	  if (strcmp (field->name, "pad") == 0)
	    output_pad_field (output_context, typedefs, field, pad++);
	  else
	    {
	      /* Dont print trailing list fields */
	      if (!(tmp2->next == NULL && field->length != NULL))
		{
		  OUT (typedefs,
		       "\t%s %s;\n",
		       gxgen_definition_to_gx_type (field->definition, FALSE),
		       field->name);
		}
	    }
	}
      OUT (typedefs, "} GX%sEvent;\n", event->definition->name);
    }
}

static void
output_errors (GXGenOutputContext *output_context)
{
  const XGenExtension *extension = output_context->extension;
  GList *tmp;
  GXGenPart error_codes = GXGEN_PART_ERROR_CODES_H_ENUMS;
  GXGenPart error_details = GXGEN_PART_ERROR_DETAILS_C;

  for (tmp = extension->errors; tmp != NULL; tmp = tmp->next)
    {
      XGenError *error = tmp->data;
      XGenDefinition *definition = error->definition;

      OUT (error_codes, "GX_PROTOCOL_ERROR_%s,\n",
	   gxgen_get_uppercase_name (definition->name));
      OUT (error_details, "{%d, \"%s\"},\n",
	   error->number,
	   gxgen_get_uppercase_name (definition->name));
    }
}

static void
output_extension_code (GXGenOutputContext *output_context)
{

  OUT (GXGEN_PART_XCB_DEPENDENCIES_H, "#include <xcb/%s.h>\n",
       output_context->extension->header);

  output_typedefs (output_context);

  output_structs_and_unions (output_context);

  output_enums (output_context);

  output_requests (output_context);

  output_event_typedefs (output_context);

  output_errors (output_context);
}

static void
output_all_gx_code (XGenState * state)
{
  GXGenOutputContext *output_context = g_new0 (GXGenOutputContext, 1);
  GString *parts[GXGEN_PART_COUNT];
  int i;
  GList *tmp;

  FILE *connection_header;
  FILE *connection_code;
  FILE *drawable_header;
  FILE *drawable_code;
  FILE *pixmap_header;
  FILE *pixmap_code;
  FILE *window_header;
  FILE *window_code;
  FILE *gcontext_header;
  FILE *gcontext_code;
  FILE *cookie_header;
  FILE *error_codes_header;
  FILE *error_details_code;
  FILE *xcb_dependencies_header;
  FILE *tests_code;

  output_context->state = state;

  for (i = 0; i < GXGEN_PART_COUNT; i++)
    parts[i] = g_string_new ("");

  output_context->parts = parts;

  OUT (GXGEN_PART_XCB_DEPENDENCIES_H, "#include <xcb/xcb.h>\n");

  OUT (GXGEN_PART_COOKIE_OBJ_H_TYPEDEFS,
       "typedef enum _GXCookieType\n{\n");
  for (tmp = state->extensions; tmp != NULL; tmp = tmp->next)
    {
      output_context->extension = tmp->data;
      output_extension_code (output_context);
    }
  OUT (GXGEN_PART_COOKIE_OBJ_H_TYPEDEFS,
       "} GXCookieType;\n");

  connection_header = fopen ("gx-connection-gen.h", "w");
  if (!connection_header)
    {
      perror ("Failed to open header file");
      return;
    }
  connection_code = fopen ("gx-connection-gen.c", "w");
  if (!connection_code)
    {
      perror ("Failed to open code file");
      return;
    }

  /* #includes */
  fwrite (parts[GXGEN_PART_CONNECTION_OBJ_H_INC]->str, 1,
	  parts[GXGEN_PART_CONNECTION_OBJ_H_INC]->len, connection_header);
  /* typedefs */
  fwrite (parts[GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS]->str, 1,
	  parts[GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS]->len,
	  connection_header);
  /* macros */
  fwrite (parts[GXGEN_PART_CONNECTION_OBJ_H_MACROS]->str, 1,
	  parts[GXGEN_PART_CONNECTION_OBJ_H_MACROS]->len, connection_header);
  /* function prototypes */
  fwrite (parts[GXGEN_PART_CONNECTION_OBJ_H_PROTOS]->str, 1,
	  parts[GXGEN_PART_CONNECTION_OBJ_H_PROTOS]->len, connection_header);

  fwrite (parts[GXGEN_PART_CONNECTION_OBJ_C_INC]->str, 1,
	  parts[GXGEN_PART_CONNECTION_OBJ_C_INC]->len, connection_code);
  fwrite (parts[GXGEN_PART_CONNECTION_OBJ_C_PROTOS]->str, 1,
	  parts[GXGEN_PART_CONNECTION_OBJ_C_PROTOS]->len, connection_code);
  fwrite (parts[GXGEN_PART_CONNECTION_OBJ_C_FUNCS]->str, 1,
	  parts[GXGEN_PART_CONNECTION_OBJ_C_FUNCS]->len, connection_code);

  fclose (connection_header);
  fclose (connection_code);

  drawable_header = fopen ("gx-drawable-gen.h", "w");
  if (!drawable_header)
    {
      perror ("Failed to open header file");
      return;
    }
  drawable_code = fopen ("gx-drawable-gen.c", "w");
  if (!drawable_code)
    {
      perror ("Failed to open code file");
      return;
    }

  /* #includes */
  fwrite (parts[GXGEN_PART_DRAWABLE_OBJ_H_INC]->str, 1,
	  parts[GXGEN_PART_DRAWABLE_OBJ_H_INC]->len, drawable_header);
  /* typedefs */
  fwrite (parts[GXGEN_PART_DRAWABLE_OBJ_H_TYPEDEFS]->str, 1,
	  parts[GXGEN_PART_DRAWABLE_OBJ_H_TYPEDEFS]->len, drawable_header);
  /* macros */
  fwrite (parts[GXGEN_PART_DRAWABLE_OBJ_H_MACROS]->str, 1,
	  parts[GXGEN_PART_DRAWABLE_OBJ_H_MACROS]->len, drawable_header);
  /* function prototypes */
  fwrite (parts[GXGEN_PART_DRAWABLE_OBJ_H_PROTOS]->str, 1,
	  parts[GXGEN_PART_DRAWABLE_OBJ_H_PROTOS]->len, drawable_header);

  fwrite (parts[GXGEN_PART_DRAWABLE_OBJ_C_INC]->str, 1,
	  parts[GXGEN_PART_DRAWABLE_OBJ_C_INC]->len, drawable_code);
  fwrite (parts[GXGEN_PART_DRAWABLE_OBJ_C_PROTOS]->str, 1,
	  parts[GXGEN_PART_DRAWABLE_OBJ_C_PROTOS]->len, drawable_code);
  fwrite (parts[GXGEN_PART_DRAWABLE_OBJ_C_FUNCS]->str, 1,
	  parts[GXGEN_PART_DRAWABLE_OBJ_C_FUNCS]->len, drawable_code);

  fclose (drawable_header);
  fclose (drawable_code);



  pixmap_header = fopen ("gx-pixmap-gen.h", "w");
  if (!pixmap_header)
    {
      perror ("Failed to open header file");
      return;
    }
  pixmap_code = fopen ("gx-pixmap-gen.c", "w");
  if (!pixmap_code)
    {
      perror ("Failed to open code file");
      return;
    }

  /* #includes */
  fwrite (parts[GXGEN_PART_PIXMAP_OBJ_H_INC]->str, 1,
	  parts[GXGEN_PART_PIXMAP_OBJ_H_INC]->len, pixmap_header);
  /* typedefs */
  fwrite (parts[GXGEN_PART_PIXMAP_OBJ_H_TYPEDEFS]->str, 1,
	  parts[GXGEN_PART_PIXMAP_OBJ_H_TYPEDEFS]->len, pixmap_header);
  /* macros */
  fwrite (parts[GXGEN_PART_PIXMAP_OBJ_H_MACROS]->str, 1,
	  parts[GXGEN_PART_PIXMAP_OBJ_H_MACROS]->len, pixmap_header);
  /* function prototypes */
  fwrite (parts[GXGEN_PART_PIXMAP_OBJ_H_PROTOS]->str, 1,
	  parts[GXGEN_PART_PIXMAP_OBJ_H_PROTOS]->len, pixmap_header);

  fwrite (parts[GXGEN_PART_PIXMAP_OBJ_C_INC]->str, 1,
	  parts[GXGEN_PART_PIXMAP_OBJ_C_INC]->len, pixmap_code);
  fwrite (parts[GXGEN_PART_PIXMAP_OBJ_C_PROTOS]->str, 1,
	  parts[GXGEN_PART_PIXMAP_OBJ_C_PROTOS]->len, pixmap_code);
  fwrite (parts[GXGEN_PART_PIXMAP_OBJ_C_FUNCS]->str, 1,
	  parts[GXGEN_PART_PIXMAP_OBJ_C_FUNCS]->len, pixmap_code);

  fclose (pixmap_header);
  fclose (pixmap_code);


  window_header = fopen ("gx-window-gen.h", "w");
  if (!window_header)
    {
      perror ("Failed to open header file");
      return;
    }
  window_code = fopen ("gx-window-gen.c", "w");
  if (!window_code)
    {
      perror ("Failed to open code file");
      return;
    }

  /* #includes */
  fwrite (parts[GXGEN_PART_WINDOW_OBJ_H_INC]->str, 1,
	  parts[GXGEN_PART_WINDOW_OBJ_H_INC]->len, window_header);
  /* typedefs */
  fwrite (parts[GXGEN_PART_WINDOW_OBJ_H_TYPEDEFS]->str, 1,
	  parts[GXGEN_PART_WINDOW_OBJ_H_TYPEDEFS]->len, window_header);
  /* macros */
  fwrite (parts[GXGEN_PART_WINDOW_OBJ_H_MACROS]->str, 1,
	  parts[GXGEN_PART_WINDOW_OBJ_H_MACROS]->len, window_header);
  /* function prototypes */
  fwrite (parts[GXGEN_PART_WINDOW_OBJ_H_PROTOS]->str, 1,
	  parts[GXGEN_PART_WINDOW_OBJ_H_PROTOS]->len, window_header);

  fwrite (parts[GXGEN_PART_WINDOW_OBJ_C_INC]->str, 1,
	  parts[GXGEN_PART_WINDOW_OBJ_C_INC]->len, window_code);
  fwrite (parts[GXGEN_PART_WINDOW_OBJ_C_PROTOS]->str, 1,
	  parts[GXGEN_PART_WINDOW_OBJ_C_PROTOS]->len, window_code);
  fwrite (parts[GXGEN_PART_WINDOW_OBJ_C_FUNCS]->str, 1,
	  parts[GXGEN_PART_WINDOW_OBJ_C_FUNCS]->len, window_code);

  fclose (window_header);
  fclose (window_code);



  gcontext_header = fopen ("gx-gcontext-gen.h", "w");
  if (!gcontext_header)
    {
      perror ("Failed to open header file");
      return;
    }
  gcontext_code = fopen ("gx-gcontext-gen.c", "w");
  if (!gcontext_code)
    {
      perror ("Failed to open code file");
      return;
    }

  /* #includes */
  fwrite (parts[GXGEN_PART_GCONTEXT_OBJ_H_INC]->str, 1,
	  parts[GXGEN_PART_GCONTEXT_OBJ_H_INC]->len, gcontext_header);
  /* typedefs */
  fwrite (parts[GXGEN_PART_GCONTEXT_OBJ_H_TYPEDEFS]->str, 1,
	  parts[GXGEN_PART_GCONTEXT_OBJ_H_TYPEDEFS]->len, gcontext_header);
  /* macros */
  fwrite (parts[GXGEN_PART_GCONTEXT_OBJ_H_MACROS]->str, 1,
	  parts[GXGEN_PART_GCONTEXT_OBJ_H_MACROS]->len, gcontext_header);
  /* function prototypes */
  fwrite (parts[GXGEN_PART_GCONTEXT_OBJ_H_PROTOS]->str, 1,
	  parts[GXGEN_PART_GCONTEXT_OBJ_H_PROTOS]->len, gcontext_header);

  fwrite (parts[GXGEN_PART_GCONTEXT_OBJ_C_INC]->str, 1,
	  parts[GXGEN_PART_GCONTEXT_OBJ_C_INC]->len, gcontext_code);
  fwrite (parts[GXGEN_PART_GCONTEXT_OBJ_C_PROTOS]->str, 1,
	  parts[GXGEN_PART_GCONTEXT_OBJ_C_PROTOS]->len, gcontext_code);
  fwrite (parts[GXGEN_PART_GCONTEXT_OBJ_C_FUNCS]->str, 1,
	  parts[GXGEN_PART_GCONTEXT_OBJ_C_FUNCS]->len, gcontext_code);

  fclose (gcontext_header);
  fclose (gcontext_code);


  cookie_header = fopen ("gx-cookie-gen.h", "w");
  if (!cookie_header)
    {
      perror ("Failed to open header file");
      return;
    }

  /* typedefs */
  fwrite (parts[GXGEN_PART_COOKIE_OBJ_H_TYPEDEFS]->str, 1,
	  parts[GXGEN_PART_COOKIE_OBJ_H_TYPEDEFS]->len, cookie_header);

  fclose (cookie_header);

  error_codes_header = fopen ("gx-protocol-error-codes-gen.h", "w");
  if (!error_codes_header)
    {
      perror ("Failed to open header file");
      return;
    }

  /* error codes enum */
  fwrite (parts[GXGEN_PART_ERROR_CODES_H_ENUMS]->str, 1,
	  parts[GXGEN_PART_ERROR_CODES_H_ENUMS]->len, error_codes_header);

  fclose (error_codes_header);

  error_details_code = fopen ("gx-protocol-error-details-gen.h", "w");
  if (!error_details_code)
    {
      perror ("Failed to open header file");
      return;
    }

  /* error details */
  fwrite (parts[GXGEN_PART_ERROR_DETAILS_C]->str, 1,
	  parts[GXGEN_PART_ERROR_DETAILS_C]->len, error_details_code);

  fclose (error_details_code);

  xcb_dependencies_header = fopen ("gx-xcb-dependencies-gen.h", "w");
  if (!xcb_dependencies_header)
    {
      perror ("Failed to open header file");
      return;
    }

  /* xcb dependencies */
  fwrite (parts[GXGEN_PART_XCB_DEPENDENCIES_H]->str, 1,
	  parts[GXGEN_PART_XCB_DEPENDENCIES_H]->len, xcb_dependencies_header);

  fclose (xcb_dependencies_header);

  tests_code = fopen ("gx-tests-gen.c", "w");
  if (!tests_code)
    {
      perror ("Failed to open header file");
      return;
    }

  /* Various auto generated tests */
  fwrite (parts[GXGEN_PART_TESTS_C]->str, 1,
	  parts[GXGEN_PART_TESTS_C]->len, tests_code);

  fclose (tests_code);


  for (i = 0; i < GXGEN_PART_COUNT; i++)
    g_string_free (parts[i], TRUE);
}

int
main (int argc, char **argv)
{
  int i;
  GList *files = NULL;
  XGenState *state;

  for (i = 1; i < argc && argv[i]; i++)
    files = g_list_prepend (files, g_strdup (argv[i]));
  /* NB: we must ensure that xproto.xml is listed first */
  files = g_list_reverse (files);

  state = xgen_parse_xcb_proto_files (files);

  g_list_foreach (files, (GFunc)g_free, NULL);
  g_list_free (files);

  output_all_gx_code (state);

  return 0;
}

