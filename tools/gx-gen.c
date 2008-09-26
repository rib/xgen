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

  GXGEN_PART_COUNT
} GXGenPart;

typedef enum
{
  GXGEN_VOID,
  GXGEN_BOOLEAN,
  GXGEN_CHAR,
  GXGEN_SIGNED,
  GXGEN_UNSIGNED,
  GXGEN_XID,
  GXGEN_FLOAT,
  GXGEN_DOUBLE,
  GXGEN_STRUCT,
  GXGEN_UNION,
  GXGEN_ENUM,
  GXGEN_XIDUNION,
  GXGEN_TYPEDEF,
  GXGEN_REQUEST,
  GXGEN_EVENT,
  GXGEN_VALUEPARAM
} GXGenType;

#if 0
typedef enum
{
  GXGEN_REQUEST,
  GXGEN_RESPONSE
} GXGenDirection;
#endif

typedef struct _GXGenDefinition GXGenDefinition;
struct _GXGenDefinition
{
  char *name;
  GXGenType type;

  /* base types */
  unsigned int size;

  /* struct, union, enum */
  union {
    GList *fields;
    GList *items;
  };

  /* typedef / valueparam */
  GXGenDefinition *reference;

  /* requests */
  GList *reply_fields;

  /* valueparams */
  gchar *mask_name;
  gchar *list_name;
};

typedef enum
{
  GXGEN_FIELDREF,
  GXGEN_VALUE,
  GXGEN_OP
} GXGenExpressionType;

typedef enum
{
  GXGEN_ADD,
  GXGEN_SUBTRACT,
  GXGEN_MULTIPLY,
  GXGEN_DIVIDE,
  GXGEN_LEFT_SHIFT,
  GXGEN_BITWISE_AND
} GXGenOp;

typedef struct _GXGenExpression GXGenExpression;
struct _GXGenExpression
{
  GXGenExpressionType type;
  union
  {
    char *field;		/* Field name for GXGEN_FIELDREF */
    unsigned long value;	/* Value for GXGEN_VALUE */
    struct			/* Operator and operands for GXGEN_OP */
    {
      GXGenOp op;
      GXGenExpression *left;
      GXGenExpression *right;
    };
  };
};

typedef struct
{
  char *name;
  GXGenDefinition *definition;
  GXGenExpression *length;      /* List length; NULL for non-list */
} GXGenFieldDefinition;

typedef struct
{
  GXGenFieldDefinition *field;
  unsigned int offset;
  union
  {
    unsigned char bool_value;
    char	  char_value;
    signed long	  signed_value;
    unsigned long unsigned_value;
  };
} GXGenFieldValue;

typedef enum
{
  GXGEN_ITEM_AS_VALUE = 1,
  GXGEN_ITEM_AS_BIT,
}GXGenItemType;

typedef struct
{
  GXGenItemType	 type;
  char		*name;
  char		*value;
  guint		 bit;
} GXGenItemDefinition;

typedef struct
{
  char	*name;
  GList *definitions;
  GList *requests;
  GList *events;
  GList *errors;
} GXGenExtension;

/* Concrete definitions for opaque and private structure types. */
typedef struct
{
  /* TODO add extension back reference? */
  unsigned char	   opcode;
  GXGenDefinition *definition;
} GXGenRequest;

typedef struct
{
  /* TODO add extension back reference? */
  unsigned char	   number;
  GXGenDefinition *definition;
} GXGenEvent;

typedef struct
{
  /* TODO add extension back reference? */
  unsigned char	   number;
  GXGenDefinition *definition;
} GXGenError;

typedef struct
{
  unsigned char  host_is_le;
  GList		*definitions;
  GList		*extensions;
} GXGenState;

static const GXGenDefinition core_type_definitions[] = {
  {
    .name = "void",
    .type = GXGEN_VOID,
    .size = 0},
  {
    .name = "char",
    .type = GXGEN_CHAR,
    .size = 1},
  {
    .name = "float",
    .type = GXGEN_FLOAT,
    .size = 4},
  {
    .name = "double",
    .type = GXGEN_DOUBLE,
    .size = 8},
  {
    .name = "BOOL",
    .type = GXGEN_BOOLEAN,
    .size = 1},
  {
    .name = "BYTE",
    .type = GXGEN_UNSIGNED,
    .size = 1},
  {
    .name = "CARD8",
    .type = GXGEN_UNSIGNED,
    .size = 1},
  {
    .name = "CARD16",
    .type = GXGEN_UNSIGNED,
    .size = 2},
  {
    .name = "CARD32",
    .type = GXGEN_UNSIGNED,
    .size = 4},
  {
    .name= "INT8",
    .type = GXGEN_SIGNED,
    .size = 1},
  {
    .name = "INT16",
    .type = GXGEN_SIGNED,
    .size = 2},
  {
    .name = "INT32",
    .type = GXGEN_SIGNED,
    .size = 4}
};

struct type_mapping
{
  char *from;
  char *to;
};

static struct type_mapping core_type_mappings[] = {
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

struct camelcase_mapping
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
static struct camelcase_mapping camelcase_dictionary[] = {
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
  {"KEY","Key"},
  {"MAP","Map"},
  {"SET","Set"},
  {"SYM","Sym"},
  {NULL}
};

/* TODO - rename these
 * (The idea is to seperate the parser API from the code generation
 * code)*/

typedef enum {
  GXGEN_IS_CONNECTION_OBJ,
  GXGEN_IS_DRAWABLE_OBJ,
  GXGEN_IS_PIXMAP_OBJ,
  GXGEN_IS_WINDOW_OBJ,
  GXGEN_IS_GCONTEXT_OBJ
} GXGenOutputObjectType;

typedef struct _GXGenOutputObject {
  GXGenOutputObjectType type;
  const char *name;
  const char *first_arg;
  GXGenFieldDefinition *first_object_field;
  GXGenPart h_typedefs, h_protos, c_funcs;
} GXGenOutputObject;

typedef struct _OutputRequest
{
  GXGenExtension    *extension;
  GXGenRequest	    *request;
  GXGenOutputObject *obj;
  char		    *xcb_name;
  char		    *gx_name;
} OutputRequest;

/* Helper function to avoid casting. */
static char *
gxgen_xml_get_prop (xmlNodePtr node, const char *name)
{
  return (char *) xmlGetProp (node, (xmlChar *) name);
}

/* Helper function to avoid casting. */
static char *
gxgen_xml_get_node_name (xmlNodePtr node)
{
  return (char *) node->name;
}

/* Helper function to avoid casting. */
static char *
gxgen_xml_get_node_content (xmlNodePtr node)
{
  return (char *) xmlNodeGetContent (node);
}

static xmlNode *
gxgen_xml_next_elem (xmlNode * elem)
{
  while (elem && elem->type != XML_ELEMENT_NODE)
    elem = elem->next;
  return elem;
}

static GXGenDefinition *
gxgen_find_type_in_extension (GXGenExtension *extension, char *type_name)
{
  GList *tmp;

  for (tmp = extension->definitions; tmp != NULL; tmp = tmp->next)
    {
      GXGenDefinition *def = tmp->data;

      if (strcmp (def->name, type_name) == 0)
	  return def;
    }

  return NULL;
}

static GXGenDefinition *
gxgen_find_type (GXGenState * state, char *name)
{
  GList *tmp;
  char **bits;
  char *extension_name = NULL;
  char *type_name;
  GXGenExtension *current_extension;

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
      GXGenExtension *extension = tmp->data;
      if (strcmp (extension->name, extension_name) == 0)
	{
	  GXGenDefinition *def =
	    gxgen_find_type_in_extension (extension, type_name);
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
      GXGenExtension *extension = tmp->data;
      GXGenDefinition *def =
	gxgen_find_type_in_extension (extension, type_name);
      if (def)
	return def;
    }

  g_critical ("Failed to find type = %s\n", name);

  g_strfreev(bits);
  return NULL;
}

static GXGenExpression *
gxgen_parse_expression (GXGenState * state, xmlNode * elem)
{
  GXGenExpression *e = g_new0 (GXGenExpression, 1);
  elem = gxgen_xml_next_elem (elem);
  if (strcmp (gxgen_xml_get_node_name (elem), "op") == 0)
    {
      char *temp = gxgen_xml_get_prop (elem, "op");
      e->type = GXGEN_OP;
      if (strcmp (temp, "+") == 0)
	e->op = GXGEN_ADD;
      else if (strcmp (temp, "-") == 0)
	e->op = GXGEN_SUBTRACT;
      else if (strcmp (temp, "*") == 0)
	e->op = GXGEN_MULTIPLY;
      else if (strcmp (temp, "/") == 0)
	e->op = GXGEN_DIVIDE;
      else if (strcmp (temp, "<<") == 0)
	e->op = GXGEN_LEFT_SHIFT;
      else if (strcmp (temp, "&") == 0)
	e->op = GXGEN_BITWISE_AND;
      elem = gxgen_xml_next_elem (elem->children);
      e->left = gxgen_parse_expression (state, elem);
      elem = gxgen_xml_next_elem (elem->next);
      e->right = gxgen_parse_expression (state, elem);
    }
  else if (strcmp (gxgen_xml_get_node_name (elem), "value") == 0)
    {
      e->type = GXGEN_VALUE;
      e->value = strtol (gxgen_xml_get_node_content (elem), NULL, 0);
    }
  else if (strcmp (gxgen_xml_get_node_name (elem), "fieldref") == 0)
    {
      e->type = GXGEN_FIELDREF;
      e->field = strdup (gxgen_xml_get_node_content (elem));
    }
  return e;
}

static GList *
gxgen_parse_field_elements (GXGenState * state, xmlNode * elem)
{
  xmlNode *cur;
  GList *fields = NULL;

  for (cur = elem->children;
       cur != NULL;
       cur = gxgen_xml_next_elem (cur->next))
    {
      GXGenFieldDefinition *field;
      field = g_new0 (GXGenFieldDefinition, 1);
      if (strcmp (gxgen_xml_get_node_name (cur), "pad") == 0)
	{
	  field->name = "pad";
	  field->definition = gxgen_find_type (state, "CARD8");
	  field->length = g_new0 (GXGenExpression, 1);
	  field->length->type = GXGEN_VALUE;
	  field->length->value = atoi (gxgen_xml_get_prop (cur, "bytes"));
	}
      else if (strcmp (gxgen_xml_get_node_name (cur), "field") == 0)
	{
	  field->name = strdup (gxgen_xml_get_prop (cur, "name"));
	  field->definition = gxgen_find_type (state,
						gxgen_xml_get_prop (cur,
								     "type"));
	}
      else if (strcmp (gxgen_xml_get_node_name (cur), "list") == 0)
	{
	  field->name = strdup (gxgen_xml_get_prop (cur, "name"));
	  field->definition = gxgen_find_type (state,
						gxgen_xml_get_prop (cur,
								     "type"));
	  if (cur->children)
	    {
	      field->length = gxgen_parse_expression (state, cur->children);
	    }
	  else
	    {
	      GXGenFieldDefinition *len_field;
	      GXGenExpression *exp;

	      len_field = g_new0 (GXGenFieldDefinition, 1);
	      len_field->name = g_strdup_printf ("%s_len", field->name);
	      len_field->definition = gxgen_find_type (state, "CARD32");

	      fields = g_list_prepend (fields, len_field);

	      exp = g_new0 (GXGenExpression, 1);
	      exp->type = GXGEN_FIELDREF;
	      exp->field = g_strdup (field->name);
	      field->length = exp;
	    }
	}
      else if (strcmp (gxgen_xml_get_node_name (cur), "valueparam") == 0)
	{
	  GXGenDefinition *definition;
	  field->name = g_strdup ("valueparam");
	  definition = g_new0 (GXGenDefinition, 1);
	  definition->name = g_strdup ("valueparam");
	  definition->type = GXGEN_VALUEPARAM;
	  definition->reference =
	    gxgen_find_type (state,
			      gxgen_xml_get_prop (cur, "value-mask-type"));
	  definition->mask_name =
	    strdup (gxgen_xml_get_prop (cur, "value-mask-name"));
	  definition->list_name =
	    strdup (gxgen_xml_get_prop (cur, "value-list-name"));
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
gxgen_parse_item_elements (GXGenState * state, xmlNode * elem)
{
  xmlNode *cur, *cur2;
  GList *items = NULL;
  long last_value = -1;

  for (cur = elem->children;
       cur != NULL;
       cur = gxgen_xml_next_elem (cur->next))
    {
      GXGenItemDefinition *item = g_new0 (GXGenItemDefinition, 1);

      item->name = gxgen_xml_get_prop (cur, "name");

      for (cur2 = cur->children;
	   cur2 != NULL;
	   cur2 = gxgen_xml_next_elem (cur2->next))
      {
	if (strcmp (gxgen_xml_get_node_name (cur2), "value") == 0)
	  {
	    char *endptr;
	    item->type = GXGEN_ITEM_AS_VALUE;
	    item->value = gxgen_xml_get_node_content (cur2);
	    last_value = strtol (item->value, &endptr, 0);
	    g_assert (item->value[0] != '\0' && endptr[0] == '\0');
	    break;
	  }
	else if (strcmp (gxgen_xml_get_node_name (cur2), "bit") == 0)
	  {
	    char *bit = gxgen_xml_get_node_content (cur2);
	    item->type = GXGEN_ITEM_AS_BIT;
	    item->bit = atoi (bit);
	    xmlFree (bit);
	    last_value = (1 << item->bit);
	    break;
	  }
      }
      if (!item->type)
	{
	  item->type = GXGEN_ITEM_AS_VALUE;
	  item->value = g_strdup_printf("%ld", ++last_value);
	}

      items = g_list_prepend (items, item);
    }

  items = g_list_reverse (items);

  return items;
}

static GList *
gxgen_parse_reply_fields (GXGenState * state, xmlNode * elem)
{
  xmlNode *cur;
  xmlNode *reply = NULL;

  for (cur = elem->children;
       cur != NULL; cur = gxgen_xml_next_elem (cur->next))
    {
      if (strcmp (gxgen_xml_get_node_name (cur), "reply") == 0)
	{
	  reply = cur;
	  break;
	}
    }

  if (!reply)
    return NULL;

  return gxgen_parse_field_elements (state, reply);
}


static void
gxgen_parse_xmlxcb_file (GXGenState * state, char *filename)
{
  xmlDoc *doc;
  xmlNode *root, *elem;
  char *extension_name;
  GXGenExtension *extension = NULL;

  /* Ignore text nodes consisting entirely of whitespace. */
  xmlKeepBlanksDefault (0);

  doc = xmlParseFile (filename);
  if (!doc)
    return;

  root = xmlDocGetRootElement (doc);
  if (!root)
    return;

  extension_name = gxgen_xml_get_prop (root, "header");
  g_assert (extension_name);

  printf ("Extension: %s\n", extension_name);

  extension = g_new0 (GXGenExtension, 1);
  extension->name = g_strdup (extension_name);
  state->extensions = g_list_prepend (state->extensions, extension);

  xmlFree (extension_name);

  if (strcmp (extension->name, "xproto") == 0)
    {
      int i;

      /* Add definitions of core types. */
      for (i = 0;
	   i < sizeof (core_type_definitions) / sizeof (GXGenDefinition);
	   i++)
	{
	  GXGenDefinition *temp = g_new0 (GXGenDefinition, 1);

	  *temp = core_type_definitions[i];
	  temp->name = g_strdup (core_type_definitions[i].name);

	  state->definitions = g_list_prepend (state->definitions, temp);
	  extension->definitions =
	    g_list_prepend (extension->definitions, temp);
	}
    }

  for (elem = root->children;
       elem != NULL; elem = gxgen_xml_next_elem (elem->next))
    {

      if (strcmp (gxgen_xml_get_node_name (elem), "request") == 0)
	{
	  GXGenRequest *request;
	  GXGenDefinition *def;
	  GXGenFieldDefinition *field;
	  GXGenFieldDefinition *first_byte_field;
	  GList *fields;
	  int opcode = atoi (gxgen_xml_get_prop (elem, "opcode"));

	  def = g_new0 (GXGenDefinition, 1);
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_REQUEST;

	  fields = gxgen_parse_field_elements (state, elem);
	  if (!fields)
	    {
	      field = g_new0 (GXGenFieldDefinition, 1);
	      field->name = g_strdup ("pad");
	      field->definition = gxgen_find_type (state, "CARD8");
	      fields = g_list_prepend (fields, field);
	    }

	  field = g_new0 (GXGenFieldDefinition, 1);
	  field->name = g_strdup ("length");
	  field->definition = gxgen_find_type (state, "CARD16");
	  fields = g_list_prepend (fields, field);

	  field = g_new0 (GXGenFieldDefinition, 1);
	  field->name = g_strdup ("opcode");
	  field->definition = gxgen_find_type (state, "BYTE");
	  fields = g_list_prepend (fields, field);

	  def->fields = fields;

	  fields = gxgen_parse_reply_fields (state, elem);
	  if (fields)
	    {
	      /* FIXME: assert that sizeof(first_byte_field)==1 */
	      first_byte_field = fields->data;
	      fields = g_list_remove (fields, first_byte_field);

	      field = g_new0 (GXGenFieldDefinition, 1);
	      field->name = g_strdup ("length");
	      field->definition = gxgen_find_type (state, "CARD32");
	      fields = g_list_prepend (fields, field);

	      field = g_new0 (GXGenFieldDefinition, 1);
	      field->name = g_strdup ("sequence");
	      field->definition = gxgen_find_type (state, "CARD16");
	      fields = g_list_prepend (fields, field);

	      fields = g_list_prepend (fields, first_byte_field);

	      field = g_new0 (GXGenFieldDefinition, 1);
	      field->name = g_strdup ("response_type");
	      field->definition = gxgen_find_type (state, "BYTE");
	      fields = g_list_prepend (fields, field);
	    }

	  def->reply_fields = fields;

	  state->definitions = g_list_prepend (state->definitions, def);

	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	  request = g_new0 (GXGenRequest, 1);
	  request->opcode = opcode;
	  request->definition = def;
	  extension->requests =
	    g_list_prepend (extension->requests, request);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "event") == 0)
	{
	  GXGenEvent *event;
	  char *no_sequence_number;
	  GXGenDefinition *def;
	  GXGenFieldDefinition *field;
	  GXGenFieldDefinition *first_byte_field;
	  GList *fields;
	  int number = atoi (gxgen_xml_get_prop (elem, "number"));

	  def = g_new0 (GXGenDefinition, 1);
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_EVENT;

	  fields = gxgen_parse_field_elements (state, elem);

	  first_byte_field = fields->data;
	  fields = g_list_remove (fields, first_byte_field);

	  no_sequence_number =
	    gxgen_xml_get_prop (elem, "no-sequence-number");
	  if (!no_sequence_number)
	    {
	      field = g_new0 (GXGenFieldDefinition, 1);
	      field->name = g_strdup ("sequence");
	      field->definition = gxgen_find_type (state, "CARD16");
	      fields = g_list_prepend (fields, field);
	    }

	  fields = g_list_prepend (fields, first_byte_field);

	  field = g_new0 (GXGenFieldDefinition, 1);
	  field->name = g_strdup ("response_type");
	  field->definition = gxgen_find_type (state, "BYTE");
	  fields = g_list_prepend (fields, field);

	  def->fields = fields;

	  state->definitions = g_list_prepend (state->definitions, def);

	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	  event = g_new0 (GXGenEvent, 1);
	  event->number = number;
	  event->definition = def;
	  extension->events = g_list_prepend (extension->events, event);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "eventcopy") == 0)
	{
	  GXGenEvent *event;
	  GXGenDefinition *def;
	  int number = atoi (gxgen_xml_get_prop (elem, "number"));
	  GXGenDefinition *copy_of;

	  def = g_new0 (GXGenDefinition, 1);
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_EVENT;
	  /* NB: This isn't a very nice thing to be doing if we
	   * are ever going to care about freeing resources!
	   * We should deep copy the fields instead. */
	  copy_of = gxgen_find_type (state,
				       gxgen_xml_get_prop (elem, "ref"));
	  def->fields = copy_of->fields;

	  state->definitions = g_list_prepend (state->definitions, def);

	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	  event = g_new0 (GXGenEvent, 1);
	  event->number = number;
	  event->definition = def;
	  extension->events = g_list_prepend (extension->events, event);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "error") == 0)
	{
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "errorcopy") == 0)
	{
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "struct") == 0)
	{
	  GXGenDefinition *def = g_new0 (GXGenDefinition, 1);

	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_STRUCT;
	  def->fields = gxgen_parse_field_elements (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "xidunion") == 0)
	{
	  GXGenDefinition *def = g_new0 (GXGenDefinition, 1);

	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_XIDUNION;
	  def->fields = gxgen_parse_field_elements (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "union") == 0)
	{
	  GXGenDefinition *def = g_new0 (GXGenDefinition, 1);

	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_UNION;
	  def->fields = gxgen_parse_field_elements (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "xidtype") == 0)
	{
	  GXGenDefinition *def = g_new0 (GXGenDefinition, 1);

	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_XID;
	  def->size = 4;
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "enum") == 0)
	{
	  GXGenDefinition *def = g_new0 (GXGenDefinition, 1);
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_ENUM;

	  def->items = gxgen_parse_item_elements (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "typedef") == 0)
	{
	  GXGenDefinition *def = g_new0 (GXGenDefinition, 1);

	  def->name = g_strdup (gxgen_xml_get_prop (elem, "newname"));
	  def->type = GXGEN_TYPEDEF;
	  def->reference = gxgen_find_type (state,
				       gxgen_xml_get_prop (elem, "oldname"));
	  state->definitions = g_list_prepend (state->definitions, def);
	  extension->definitions =
	    g_list_prepend (extension->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "import") == 0)
	{
	}
    }

  extension->definitions = g_list_reverse (extension->definitions);
}

static long
gxgen_evaluate_expression (GXGenExpression * expression,
			   GList *field_values)
{
  switch (expression->type)
    {
    case GXGEN_VALUE:
      return expression->value;

    case GXGEN_FIELDREF:
      {
	GList *tmp;
	for (tmp = field_values; tmp != NULL; tmp = tmp->next)
	  {
	    GXGenFieldValue *field_value = tmp->data;
	    if (strcmp (field_value->field->name, expression->field) == 0)
	      switch (field_value->field->definition->type)
		{
		case GXGEN_BOOLEAN:
		  return field_value->bool_value;
		case GXGEN_CHAR:
		  return field_value->char_value;
		case GXGEN_SIGNED:
		  return field_value->signed_value;
		case GXGEN_UNSIGNED:
		case GXGEN_XID:
		  return field_value->unsigned_value;
		default:
		  g_error ("un-expected fieldref type");
		}
	  }
	g_error ("Failed to find expresion's fieldref=\"%s\"\n",
		expression->field);
      }

    case GXGEN_OP:
      {
	long left =
	  gxgen_evaluate_expression (expression->left, field_values);
	long right =
	  gxgen_evaluate_expression (expression->right, field_values);
	switch (expression->op)
	  {
	  case GXGEN_ADD:
	    return left + right;
	  case GXGEN_SUBTRACT:
	    return left - right;
	  case GXGEN_MULTIPLY:
	    return left * right;
	    /* FIXME: divide by zero */
	  case GXGEN_DIVIDE:
	    return left / right;
	  case GXGEN_LEFT_SHIFT:
	    return left << right;
	  case GXGEN_BITWISE_AND:
	    return left & right;
	  }
      }
    default:
      g_error ("un-handled operator type!");
    }

  g_error ("Failed to parse expression!");
  return 0;
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

  for (i = 0; camelcase_dictionary[i].uppercase; i++)
    while ((str = strstr (name_cc, camelcase_dictionary[i].uppercase)))
      memcpy (str, camelcase_dictionary[i].camelcase,
	      strlen(camelcase_dictionary[i].uppercase));
  return name_cc;
}

static char *
gxgen_definition_to_gx_type (GXGenDefinition * definition,
			     gboolean object_types)
{
  struct type_mapping *mapping;
  char *name_cc;
  char *ret;

  for (mapping = core_type_mappings; mapping->from != NULL; mapping++)
    if (strcmp (mapping->from, definition->name) == 0)
      return gxgen_get_camelcase_name (mapping->to);

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
out (GString ** parts, GXGenPart part, const char *format, ...)
{
  va_list ap;

  va_start (ap, format);
  g_string_append_vprintf (parts[part], format, ap);
  va_end (ap);
}

static void
output_pad_field (GString ** parts,
		  GXGenPart part, GXGenFieldDefinition * field, int index)
{
  char *type_name_cc;
  char *pad_name;

  if (field->length->value == 1)
    pad_name = g_strdup_printf ("pad%u", index);
  else
    pad_name = g_strdup_printf ("pad%u[%lu]", index, field->length->value);

  type_name_cc = gxgen_definition_to_gx_type (field->definition, FALSE);
  out (parts, part, "\t%s %s;\n",
       type_name_cc, pad_name);
  g_free (type_name_cc);
  g_free (pad_name);
}

static void
output_field_xcb_reference (GString ** parts,
			    GXGenPart part, GXGenFieldDefinition * field)
{
  if (strcmp (field->definition->name, "DRAWABLE") == 0)
    {
      out (parts, part, "gx_drawable_get_xid (%s)", field->name);
    }
  else if (strcmp (field->definition->name, "PIXMAP") == 0
	   || strcmp (field->definition->name, "WINDOW") == 0)
    {
      out (parts, part,
	   "gx_drawable_get_xid (\n"
	   "\t\t\t\tGX_DRAWABLE(%s))", field->name);
    }
  else if (strcmp (field->definition->name, "GCONTEXT") == 0)
    {
      out (parts, part, "gx_gcontext_get_xcb_gcontext (%s)", field->name);
    }
  else
    out (parts, part, "%s", field->name);
}

static gboolean
is_special_xid_definition (GXGenDefinition *definition)
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
output_typedefs (GXGenState * state,
		 GXGenExtension * extension,
		 GString ** parts)
{
  GList *tmp;
  char *typedef_type_cc;
  char *typedef_name_cc;

  for (tmp = extension->definitions; tmp != NULL; tmp = tmp->next)
    {
      GXGenDefinition *def = tmp->data;

      if (def->type == GXGEN_BOOLEAN
	  || def->type == GXGEN_CHAR
	  || def->type == GXGEN_SIGNED || def->type == GXGEN_UNSIGNED)
	{

	  if (strcmp (def->name, "char") == 0)
	    continue;

	  typedef_type_cc = gxgen_definition_to_gx_type (def, FALSE);
	  typedef_name_cc = gxgen_get_camelcase_name (def->name);

	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef %s %s;\n", typedef_type_cc, typedef_name_cc);

	  g_free (typedef_type_cc);
	  g_free (typedef_name_cc);
	}
      else if (def->type == GXGEN_XID || def->type == GXGEN_XIDUNION)
	{
	  if (is_special_xid_definition (def))
	    continue;

	  typedef_name_cc = gxgen_definition_to_gx_type (def, FALSE);

	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef guint32 %s;\n", typedef_name_cc);

	  g_free (typedef_name_cc);
	}
    }
  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "\n");

  for (tmp = extension->definitions; tmp != NULL; tmp = tmp->next)
    {
      GXGenDefinition *def = tmp->data;

      if (def->type == GXGEN_TYPEDEF)
	{
	  typedef_type_cc = gxgen_get_camelcase_name (def->reference->name);
	  typedef_name_cc = gxgen_definition_to_gx_type (def, FALSE);

	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef %s %s;\n", typedef_type_cc, typedef_name_cc);

	  g_free (typedef_type_cc);
	  g_free (typedef_name_cc);
	}
    }
  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "\n\n\n");


}

static void
output_structs_and_unions (GXGenState * state,
			   GXGenExtension * extension,
			   GString ** parts)
{
  GList *tmp;

  for (tmp = extension->definitions; tmp != NULL; tmp = tmp->next)
    {
      GXGenDefinition *def = tmp->data;
      guint pad = 0;

      if (def->type == GXGEN_STRUCT || def->type == GXGEN_UNION)
	{
	  GList *tmp2;

	  /* Some types are special cased if they are represented as
	   * objects */
	  if (strcmp (def->name, "SCREEN") == 0)
	    continue;

	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef %s {\n",
	       def->type == GXGEN_STRUCT ? "struct" : "union");

	  for (tmp2 = def->fields; tmp2 != NULL; tmp2 = tmp2->next)
	    {
	      GXGenFieldDefinition *field = tmp2->data;
	      if (strcmp (field->name, "pad") == 0)
		output_pad_field (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
				field, pad++);
	      else
		{
		  /* Dont print trailing list fields */
		  if (!(tmp2->next == NULL && field->length != NULL))
		    {
		      out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
			   "\t%s %s;\n",
			   gxgen_definition_to_gx_type (field->definition, FALSE),
			   field->name);
		    }
		}
	    }

	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "");
	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "} %s;\n\n", gxgen_definition_to_gx_type (def, FALSE));
	}
    }
  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "\n");

}

static void
output_enums (GXGenState * state,
	      GXGenExtension * extension,
	      GString ** parts)
{
  GList *tmp;

  for (tmp = extension->definitions; tmp != NULL; tmp = tmp->next)
    {
      GXGenDefinition *def = tmp->data;
      GList *tmp2;

      if (def->type != GXGEN_ENUM)
	continue;

      out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	      "typedef enum\n{\n");

      for (tmp2 = def->items; tmp2 != NULL; tmp2 = tmp2->next)
	{
	  GXGenItemDefinition *item = tmp2->data;
	  char *enum_stem_uc = gxgen_get_uppercase_name (item->name);
	  char *enum_prefix_uc = gxgen_get_uppercase_name (def->name);

	  if (item->type == GXGEN_ITEM_AS_VALUE)
	    {
	      out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
		" GX_%s_%s = %s,\n",
		enum_prefix_uc,
		enum_stem_uc,
		item->value);
	    }
	  else
	    {
	      out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
		" GX_%s_%s = (1 << %u),\n",
		enum_prefix_uc,
		enum_stem_uc,
		item->bit);
	    }

	  g_free (enum_stem_uc);
	  g_free (enum_prefix_uc);
	}
      out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
		"} GX%s;\n\n", def->name);
    }
}

static GXGenOutputObject *
identify_request_object (GXGenRequest *request)
{
  GXGenOutputObject *obj = g_new0(GXGenOutputObject, 1);
  GList *tmp;

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
	  GXGenFieldDefinition *field = tmp->data;

	  if (strcmp (field->name, "opcode") == 0
	      || strcmp (field->name, "pad") == 0
	      || strcmp (field->name, "length") == 0)
	    continue;

	  if (strcmp (field->name, "window") == 0)
	    {
	      obj->type = GXGEN_IS_WINDOW_OBJ;
	      obj->name = "window";
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
	      obj->name = "pixmap";
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
	      obj->name = "drawable";
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
	      obj->name = "gcontext";
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
  if (!obj->name)
    {
      obj->type = GXGEN_IS_CONNECTION_OBJ;
      obj->name = "connection";
      obj->first_arg = "GXConnection *connection";
      obj->h_typedefs = GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS;
      obj->h_protos = GXGEN_PART_CONNECTION_OBJ_H_PROTOS;
      obj->c_funcs = GXGEN_PART_CONNECTION_OBJ_C_FUNCS;
    }

  return obj;
}

static void
output_reply_typedef (GXGenState * state,
		      GString ** parts,
		      OutputRequest *out_request)
{
  GXGenRequest *request = out_request->request;
  GXGenOutputObject *obj = out_request->obj;
  GList *tmp;
  guint pad = 0;

  if (g_list_length (request->definition->reply_fields) <= 1)
    return;

  out (parts, obj->h_typedefs, "typedef struct {\n");

  for (tmp = request->definition->reply_fields;
       tmp != NULL; tmp = tmp->next)
    {
      GXGenFieldDefinition *field = tmp->data;
      if (strcmp (field->name, "pad") == 0)
	output_pad_field (parts, obj->h_typedefs, field, pad++);
      else
	{
	  /* Dont print trailing list members */
	  if (!(tmp->next == NULL && field->length != NULL))
	    {
	      out (parts, obj->h_typedefs,
		   "\t%s %s;\n",
		   gxgen_definition_to_gx_type (field->definition, FALSE),
		   field->name);
	    }
	}
    }
  out (parts, obj->h_typedefs,
       "\n} GX%sX11Reply;\n\n", out_request->gx_name);

  out (parts, obj->h_typedefs, "typedef struct {\n");
  out (parts, obj->h_typedefs, "\tGXConnection *connection;\n");
  out (parts, obj->h_typedefs, "\tGX%sX11Reply *x11_reply;\n",
       out_request->gx_name);
  out (parts, obj->h_typedefs,
       "\n} GX%sReply;\n\n", out_request->gx_name);
}

static void
output_reply_list_get (GXGenState *state,
		       GString **parts,
		       OutputRequest *out_request)
{
  GXGenRequest *request = out_request->request;
  GXGenOutputObject *obj = out_request->obj;
  GXGenFieldDefinition *field;
  char *gx_name_lc;

  if (!request->definition->reply_fields)
    return;

  field = (g_list_last (request->definition->reply_fields))->data;
  if (field->length == NULL)
    return;

  /* FIXME - shouldn't be restricted to FIELDREF length types */
  if (field->length->type != GXGEN_FIELDREF)
    return;

  gx_name_lc = gxgen_get_lowercase_name (out_request->gx_name);
  /* name_lc = out_request->gx_name_lc; */

  out (parts, obj->h_protos,
       "GArray *\n"
       "gx_%s_%s_get_%s (GX%sReply *%s_reply);\n",
       obj->name,
       gx_name_lc,
       field->name,
       out_request->gx_name,
       gx_name_lc);

  out (parts, obj->c_funcs,
       "GArray *\n"
       "gx_%s_%s_get_%s (GX%sReply *%s_reply)\n",
       obj->name,
       gx_name_lc,
       field->name,
       out_request->gx_name,
       gx_name_lc);

  out (parts, obj->c_funcs,
       "{\n");

  out (parts, obj->c_funcs,
       "  %s *p = (%s *)(%s_reply->x11_reply + 1);\n",
       gxgen_definition_to_gx_type (field->definition, FALSE),
       gxgen_definition_to_gx_type (field->definition, FALSE),
       gx_name_lc);


  if (is_special_xid_definition (field->definition))
    {
      out (parts, obj->c_funcs,
       "  GArray *tmp = g_array_new(TRUE, FALSE, sizeof(void *));\n");
      out (parts, obj->c_funcs,
       "  int i;\n");
    }
  else
    out (parts, obj->c_funcs,
       "  GArray *tmp = g_array_new(TRUE, FALSE, sizeof(%s));\n",
       gxgen_definition_to_gx_type (field->definition, FALSE));

  if (is_special_xid_definition (field->definition))
    {
      out (parts, obj->c_funcs,
       "  for (i = 0; i< %s_reply->x11_reply->%s; i++)\n"
       "    {\n",
       gx_name_lc,
       field->length->field);

      out (parts, obj->c_funcs,
       "      /* FIXME - mutex */\n");

      out (parts, obj->c_funcs,
       "      %s item = _gx_%s_find_from_xid (p[i]);\n",
       gxgen_definition_to_gx_type (field->definition, TRUE),
       obj->name);

      out (parts, obj->c_funcs,
       "      if (!item)\n"
       "	item = g_object_new (gx_%s_get_type(),\n"
       "			     \"connection\", %s_reply->connection,\n"
       "			     \"xid\", p[i],\n"
       "			     \"wrap\", TRUE,\n"
       "			     NULL);\n",
       obj->name, gx_name_lc);

      out (parts, obj->c_funcs,
       "      tmp = g_array_append_val (tmp, item);\n"
       "    }\n");
    }
  else
    {
      out (parts, obj->c_funcs,
       "  tmp = g_array_append_vals (tmp, p, %s_reply->x11_reply->%s);\n",
       gx_name_lc, field->length->field);
    }

  out (parts, obj->c_funcs,
       "  return tmp;\n");

  out (parts, obj->c_funcs,
       "}\n");

  g_free (gx_name_lc);
}

static void
output_reply_list_free (GXGenState * state,
		        GString ** parts,
			OutputRequest *out_request)
{
  GXGenRequest *request = out_request->request;
  GXGenOutputObject *obj = out_request->obj;
  GXGenFieldDefinition *field;
  char *gx_name_lc;

  if (!request->definition->reply_fields)
    return;

  field = (g_list_last (request->definition->reply_fields))->data;
  if (field->length == NULL)
    return;

  /* FIXME - shouldn't be restricted to FIELDREF length types */
  if (field->length->type != GXGEN_FIELDREF)
    return;

  gx_name_lc = gxgen_get_lowercase_name (out_request->gx_name);
  /*name_lc = out_request->gx_name_lc;*/

  out (parts, obj->h_protos,
       "\nvoid\n"
       "gx_%s_%s_free_%s (GArray *%s);\n",
       obj->name,
       gx_name_lc,
       field->name,
       field->name);

  out (parts, obj->c_funcs,
       "\nvoid\n"
       "gx_%s_%s_free_%s (GArray *%s)\n",
       obj->name,
       gx_name_lc,
       field->name,
       field->name);

  out (parts, obj->c_funcs,
       "{\n");

  if (is_special_xid_definition (field->definition))
    {
      out (parts, obj->c_funcs,
       "\t%s*p = %s->data;\n"
       "\tint i;\n"
       "\tfor (i = 0; i < %s->len; i++)\n"
       "\t\tg_object_unref (p[i]);\n",
       gxgen_definition_to_gx_type (field->definition, TRUE),
       field->name,
       field->name);
    }

  out (parts, obj->c_funcs, "\tg_array_free (%s, TRUE);\n", field->name);

  out (parts, obj->c_funcs,
       "}\n");

  g_free (gx_name_lc);
}

static void
output_reply_free (GXGenState *state,
		   GString **parts,
		   OutputRequest *out_request)
{
  GXGenRequest *request = out_request->request;
  GXGenOutputObject *obj = out_request->obj;
  char *gx_name_lc;

  if (!request->definition->reply_fields)
    return;

  gx_name_lc = gxgen_get_lowercase_name (out_request->gx_name);
  /* name_lc = out_request->gx_name_lc; */

  out (parts, obj->h_protos,
       "void\n"
       "gx_%s_%s_reply_free (GX%sReply *%s_reply);\n",
       obj->name,
       gx_name_lc,
       out_request->gx_name,
       gx_name_lc);

  out (parts, obj->c_funcs,
       "void\n"
       "gx_%s_%s_reply_free (GX%sReply *%s_reply)\n",
       obj->name,
       gx_name_lc,
       out_request->gx_name,
       gx_name_lc);

  out (parts, obj->c_funcs,
       "{\n");

  out (parts, obj->c_funcs,
       "  free (%s_reply->x11_reply);\n",
       gx_name_lc);
  out (parts, obj->c_funcs,
       "  g_slice_free (GX%sReply, %s_reply);\n",
       out_request->gx_name, gx_name_lc);

  out (parts, obj->c_funcs,
       "}\n");

  g_free (gx_name_lc);
}

static void
output_mask_value_variable_declarations (GString **parts, GXGenPart part)
{
  out (parts, part,
       "\tguint32 value_list_len = "
	  "gx_mask_value_items_get_count (mask_value_items);\n");
  out (parts, part,
       "\tguint32 *value_list = "
	  "alloca (value_list_len * 4);\n");
  out (parts, part,
	"\tguint32 value_mask;\n");
  out (parts, part,
	"\n");

  out (parts, part,
       "\tgx_mask_value_items_get_list (mask_value_items, "
	  "&value_mask, value_list);\n");
}

/**
 * output_reply_variable_definitions:
 * @parts: Your output streams
 * @part: The particular stream you to output too
 * @request: The request to which you will be replying
 *
 * This function outputs the variable declarations needed
 * for preparing a reply. This should be called in the
 * *_reply () funcs that take a cookie or the synchronous
 * request functions.
 */
static void
output_reply_variable_declarations (GXGenState *state,
				    GString **parts,
				    OutputRequest *out_request)
{
  /* GXGenRequest *request = out_request->request; */
  GXGenOutputObject *obj = out_request->obj;
  /* char *gx_name_cc; */

  /* name_lc = gxgen_get_lowercase_name (out_request->gx_name); */
  /* gx_name_cc = gxgen_get_camelcase_name (out_request->gx_name); */

  out (parts, obj->c_funcs,
       "\txcb_generic_error_t *xcb_error;\n");
  out (parts, obj->c_funcs,
       "\tGX%sReply *reply = g_slice_new (GX%sReply);\n",
       out_request->gx_name,
       out_request->gx_name);

  /* g_free (name_cc); */
}

/**
 * output_async_request:
 *
 * This function outputs the code for all gx_*_async () functions
 */
void
output_async_request (GXGenState *state,
		      GString **parts,
		      OutputRequest *out_request)
{
  GXGenRequest *request = out_request->request;
  GXGenOutputObject *obj = out_request->obj;
  GList *tmp;
  char *cookie_type_uc;
  char *obj_name_uc;
  char *gx_name_uc;
  char *gx_name_lc;
  char *xcb_name_lc;
  gboolean has_mask_value_items = FALSE;

  gx_name_lc = gxgen_get_lowercase_name (out_request->gx_name);
  xcb_name_lc = gxgen_get_lowercase_name (out_request->xcb_name);


  /* FIXME */
  out (parts, obj->h_protos, "\nGXCookie *\ngx_%s_%s_async (%s",
       obj->name, gx_name_lc, obj->first_arg);
  out (parts, obj->c_funcs, "\nGXCookie *\ngx_%s_%s_async (%s",
       obj->name, gx_name_lc, obj->first_arg);

  for (tmp = request->definition->fields; tmp != NULL; tmp = tmp->next)
    {
      GXGenFieldDefinition *field = tmp->data;
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
	{
	  out (parts, obj->h_protos,
	       ",\n\t\tconst %s *%s", type, field->name);
	  out (parts, obj->c_funcs,
	       ",\n\t\tconst %s *%s", type, field->name);
	}
      else if (field->definition->type == GXGEN_VALUEPARAM)
	{
	  out (parts, obj->h_protos,
	       ",\n\t\tGXMaskValueItem *mask_value_items");
	  out (parts, obj->c_funcs,
	       ",\n\t\tGXMaskValueItem *mask_value_items");
	  has_mask_value_items = TRUE;
	}
      else
	{
	  out (parts, obj->h_protos, ",\n\t\t%s %s", type, field->name);
	  out (parts, obj->c_funcs, ",\n\t\t%s %s", type, field->name);
	}
    }
  out (parts, obj->h_protos, ");\n\n");
  out (parts, obj->c_funcs, ")\n{\n");

  /*
   * *_async() code
   */
  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    {
      out (parts, obj->c_funcs,
	   "\tGXConnection *connection = gx_%s_get_connection (%s);\n",
	   obj->name, obj->name);
    }

  if (g_list_length (request->definition->reply_fields) <= 1)
    out (parts, obj->c_funcs,
	 "\txcb_void_cookie_t xcb_cookie;\n");
  else
    out (parts, obj->c_funcs,
	 "\txcb_%s_cookie_t xcb_cookie;\n", xcb_name_lc);
  out (parts, obj->c_funcs,
	 "\tGXCookie *cookie;\n\n");

  if (has_mask_value_items)
    output_mask_value_variable_declarations (parts, obj->c_funcs);

  out (parts, obj->c_funcs, "\n");

  out (parts, obj->c_funcs,
       "\txcb_cookie =\n"
       "\t\txcb_%s(\n"
       "\t\t\tgx_connection_get_xcb_connection (connection)", xcb_name_lc);

  for (tmp = request->definition->fields; tmp != NULL;
       tmp = tmp->next)
    {
      GXGenFieldDefinition *field = tmp->data;

      if (strcmp (field->name, "opcode") == 0
	  || strcmp (field->name, "pad") == 0
	  || strcmp (field->name, "length") == 0)
	continue;

      if (field->definition->type != GXGEN_VALUEPARAM)
	{
	  /* Some special cased field types require a function call
	   * to lookup their counterpart xcb value */
	  out (parts, obj->c_funcs, ",\n\t\t\t");
	  output_field_xcb_reference (parts, obj->c_funcs, field);
	}
      else
	{
	  out (parts, obj->c_funcs, ",\n\t\t\tvalue_mask");
	  out (parts, obj->c_funcs, ",\n\t\t\tvalue_list");
	}
    }
  out (parts, obj->c_funcs, ");\n\n");

  obj_name_uc = gxgen_get_uppercase_name (obj->name);
  gx_name_uc = gxgen_get_uppercase_name (out_request->gx_name);

  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    cookie_type_uc =
      g_strdup_printf ("GX_COOKIE_%s_%s", obj_name_uc, gx_name_uc);
  else
    cookie_type_uc =
      g_strdup_printf ("GX_COOKIE_%s", gx_name_uc);

  out (parts, GXGEN_PART_COOKIE_OBJ_H_TYPEDEFS,
	"\t%s,\n", cookie_type_uc);

  g_free (obj_name_uc);
  g_free (gx_name_uc);


  out (parts, obj->c_funcs,
       "\tcookie = gx_cookie_new (connection, %s, xcb_cookie.sequence);\n",
       cookie_type_uc);
  /*
  out (parts, obj->c_funcs,
       "\tgx_connection_register_cookie (connection, cookie);\n");
  */

  g_free (cookie_type_uc);

  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    out (parts, obj->c_funcs, "\tg_object_unref (connection);\n");

#if 0
  out (parts, obj->c_funcs,
       "/* NB: normally developers dont need to manager cookie references\n"
       "   since the GX API owns a reference until the cookie is expired\n"
       "   upon recieving its corresponding reply */\n");
  out (parts, obj->c_funcs, "\tg_object_ref (cookie);\n");
#endif

  out (parts, obj->c_funcs,
       "\tgx_connection_register_cookie (connection, cookie);\n");

  out (parts, obj->c_funcs, "\treturn cookie;\n");

  out (parts, obj->c_funcs, "}\n");

}

void
output_reply (GXGenState *state,
	      GString **parts,
	      OutputRequest *out_request)
{
  GXGenRequest *request = out_request->request;
  GXGenOutputObject *obj = out_request->obj;
  char *gx_name_lc =
    gxgen_get_lowercase_name (out_request->gx_name);
  char *xcb_name_lc =
    gxgen_get_lowercase_name (out_request->xcb_name);
  char *obj_name_uc = gxgen_get_uppercase_name (obj->name);

  g_assert (g_list_length (request->definition->reply_fields) > 1);

  out (parts, obj->c_funcs, "\nGX%sReply *\n",
       out_request->gx_name);
  out (parts, obj->h_protos, "\nGX%sReply *\n",
       out_request->gx_name);

  out (parts, obj->c_funcs,
       "gx_%s_%s_reply (GXCookie *cookie, GError **error)\n",
       obj->name, gx_name_lc);
  out (parts, obj->h_protos,
       "gx_%s_%s_reply (GXCookie *cookie, GError **error);\n",
       obj->name, gx_name_lc);

  out (parts, obj->c_funcs,
       "{\n");

  out (parts, obj->c_funcs,
       "\tGXConnection *connection = gx_cookie_get_connection (cookie);\n");
#if 0
  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    {
      out (parts, obj->c_funcs,
	   "\tGXConnection *connection = gx_%s_get_connection (%s);\n",
	   obj->name, obj->name);
    }
#endif

#if 0
  if (g_list_length (request->definition->reply_fields) <= 1)
    out (parts, obj->c_funcs, "\txcb_void_cookie_t xcb_cookie;\n");
  else
    {
#endif
      out (parts, obj->c_funcs, "\txcb_%s_cookie_t xcb_cookie;\n",
	   xcb_name_lc);
      output_reply_variable_declarations (state, parts, out_request);
#if 0
    }
#endif
  out (parts, obj->c_funcs, "\n");

  out (parts, obj->c_funcs,
       "\tg_return_val_if_fail (error == NULL || *error == NULL, NULL);\n");

  out (parts, obj->c_funcs, "\n");

  out (parts, obj->c_funcs,
       "\txcb_cookie.sequence = gx_cookie_get_sequence (cookie);\n");

#if 0
  if (g_list_length (request->definition->reply_fields) > 1)
    out (parts, obj->c_funcs, "\t reply = ");
  out (parts, obj->c_funcs, "\txcb_%s_reply (xcb_cookie);\n",
       xcb_name_lc);
#endif

#if 0
  out (parts, obj->c_funcs,
       "\treply->x11_reply = (GX%sX11Reply *)\n"
       "\t\txcb_%s_reply (\n"
       "\t\t\tgx_connection_get_xcb_connection (connection),\n"
       "\t\t\tcookie,\n" "\t\t\t&xcb_error);\n",
       out_request->gx_name,
       xcb_name_lc);
#endif
  out (parts, obj->c_funcs,
       "\treply->x11_reply = (GX%sX11Reply *)\n"
       "\t\tgx_cookie_get_reply (cookie);\n",
       out_request->gx_name);

  /* FIXME - we need a mechanism for translating X errors into a glib
   * error domain, code and message. */
  out (parts, obj->c_funcs,
       "\tif (!reply->x11_reply)\n"
       "\t  {\n"
       "\t\t/* FIXME \n"
       "\t\t * gx_error = gx_cookie_get_error (cookie);\n"
       "\t\t * error = g_set_error (error, GX_CONNECTION,...)\n"
       "\t\t */\n"
       "\t  }\n");


  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    out (parts, obj->c_funcs, "\tg_object_unref (connection);\n");

#if 0
#error "FIXME - remember to release the API reference if the connection"
#error "gets finalised too!!!"

  out (parts, obj->c_funcs,
       "/* NB: The GX API internally manages a cookie reference that\n"
       " * expires once the cookies corresponding reply has been recieved\n"
       " */\n");
  out (parts, obj->c_funcs, "\tg_object_unref (cookie);\n");
#endif
  out (parts, obj->c_funcs,
       "\tgx_connection_unregister_cookie (connection, cookie);\n");

#if 0
  if (g_list_length (request->definition->reply_fields) <= 1)
    out (parts, obj->c_funcs, "\treturn;\n");
  else
#endif
    out (parts, obj->c_funcs, "\treturn reply;\n");

  out (parts, obj->c_funcs,
       "}\n");

  g_free (gx_name_lc);
  g_free (xcb_name_lc);
  g_free (obj_name_uc);
}

void
output_sync_request (GXGenState *state,
		     GString **parts,
		     OutputRequest *out_request)
{
  GXGenRequest *request = out_request->request;
  GXGenOutputObject *obj = out_request->obj;
  char *gx_name_lc =
    gxgen_get_lowercase_name (out_request->gx_name);
  char *xcb_name_lc =
    gxgen_get_lowercase_name (out_request->xcb_name);
  gboolean has_mask_value_items = FALSE;
  GList *tmp;

  if (g_list_length (request->definition->reply_fields) <= 1)
    {
      out (parts, obj->h_protos, "\ngboolean\n");
      out (parts, obj->c_funcs, "\ngboolean\n");
    }
  else
    {
      out (parts, obj->h_protos, "\nGX%sReply *\n",
	   out_request->gx_name);
      out (parts, obj->c_funcs, "\nGX%sReply *\n",
	   out_request->gx_name);
    }

  out (parts, obj->h_protos, "gx_%s_%s (%s",
       obj->name, gx_name_lc, obj->first_arg);
  out (parts, obj->c_funcs, "gx_%s_%s (%s",
       obj->name, gx_name_lc, obj->first_arg);

  for (tmp = request->definition->fields;
       tmp != NULL; tmp = tmp->next)
    {
      GXGenFieldDefinition *field = tmp->data;
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
	  out (parts, obj->h_protos,
	       ",\n\t\tconst %s *%s", type, field->name);
	  out (parts, obj->c_funcs,
	       ",\n\t\tconst %s *%s", type, field->name);
	}
      else if (field->definition->type == GXGEN_VALUEPARAM)
	{
	  out (parts, obj->h_protos,
	       ",\n\t\tGXMaskValueItem *mask_value_items");
	  out (parts, obj->c_funcs,
	       ",\n\t\tGXMaskValueItem *mask_value_items");
	  has_mask_value_items = TRUE;
	}
      else
	{
	  out (parts, obj->h_protos, ",\n\t\t%s %s", type, field->name);
	  out (parts, obj->c_funcs, ",\n\t\t%s %s", type, field->name);
	}
    }
  out (parts, obj->h_protos, ",\n\t\tGError **error);\n\n");
  out (parts, obj->c_funcs, ",\n\t\tGError **error)\n{\n");


  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    {
      out (parts, obj->c_funcs,
	   "\tGXConnection *connection = gx_%s_get_connection (%s);\n",
	   obj->name, obj->name);
    }

  if (g_list_length (request->definition->reply_fields) <= 1)
    out (parts, obj->c_funcs, "\txcb_void_cookie_t cookie;\n");
  else
    {
      out (parts, obj->c_funcs, "\txcb_%s_cookie_t cookie;\n", xcb_name_lc);
      output_reply_variable_declarations (state, parts, out_request);
    }

  /* If the request has a GXGEN_VALUEPARAM field, then we will need
   * to translate an array of GXMaskValueItems from the user.
   */
  if (has_mask_value_items)
    output_mask_value_variable_declarations (parts, obj->c_funcs);

  out (parts, obj->c_funcs, "\n");
  if (g_list_length (request->definition->reply_fields) <= 1)
    out (parts, obj->c_funcs,
	 "\tg_return_val_if_fail (error == NULL || *error == NULL, FALSE);\n");
  else
    out (parts, obj->c_funcs,
	 "\tg_return_val_if_fail (error == NULL || *error == NULL, NULL);\n");

  if (g_list_length (request->definition->reply_fields) > 1)
      out (parts, obj->c_funcs,
	   "\treply->connection = connection;\n\n");

  out (parts, obj->c_funcs,
       "\tcookie =\n"
       "\t\txcb_%s(\n"
       "\t\t\tgx_connection_get_xcb_connection (connection)", xcb_name_lc);

  for (tmp = request->definition->fields; tmp != NULL;
       tmp = tmp->next)
    {
      GXGenFieldDefinition *field = tmp->data;

      if (strcmp (field->name, "opcode") == 0
	  || strcmp (field->name, "pad") == 0
	  || strcmp (field->name, "length") == 0)
	continue;

      if (field->definition->type != GXGEN_VALUEPARAM)
	{
	  /* Some special cased field types require a function call
	   * to lookup their counterpart xcb value */
	  out (parts, obj->c_funcs, ",\n\t\t\t");
	  output_field_xcb_reference (parts, obj->c_funcs, field);
	}
      else
	{
	  out (parts, obj->c_funcs, ",\n\t\t\tvalue_mask");
	  out (parts, obj->c_funcs, ",\n\t\t\tvalue_list");
	}
    }
  out (parts, obj->c_funcs, ");\n\n");

  if (g_list_length (request->definition->reply_fields) > 1)
    {
      out (parts, obj->c_funcs,
	   "\treply->x11_reply = (GX%sX11Reply *)\n"
	   "\t\txcb_%s_reply (\n"
	   "\t\t\tgx_connection_get_xcb_connection (connection),\n"
	   "\t\t\tcookie,\n" "\t\t\t&xcb_error);\n",
	   out_request->gx_name,
	   xcb_name_lc);
    }

  if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
    out (parts, obj->c_funcs, "\tg_object_unref (connection);\n");

  if (g_list_length (request->definition->reply_fields) <= 1)
    out (parts, obj->c_funcs, "\n\treturn TRUE;\n");
  else
    out (parts, obj->c_funcs, "\n\treturn reply;\n");

  out (parts, obj->c_funcs, "}\n\n");

  g_free (gx_name_lc);
  g_free (xcb_name_lc);

}

static void
output_requests (GXGenState * state,
		 GXGenExtension * extension,
		 GString ** parts)
{
  GList *tmp;

  out (parts, GXGEN_PART_COOKIE_OBJ_H_TYPEDEFS,
       "typedef enum _GXCookieType\n{\n");

  for (tmp = extension->requests; tmp != NULL; tmp = tmp->next)
    {
      GXGenRequest *request = tmp->data;
      OutputRequest *out_request;
      GXGenOutputObject *obj = NULL;
      gchar *gx_name;

      /* Some constructor type requests are special cased as object
       * constructors */
      if (strcmp (request->definition->name, "CreateWindow") == 0
	  || strcmp (request->definition->name, "CreatePixmap") == 0
	  || strcmp (request->definition->name, "CreateGC") == 0)
	continue;

      obj = identify_request_object (request);

      /* the get/set_property names clash with the gobject
       * property accessor functions */
      if (strcmp (request->definition->name, "GetProperty") == 0)
	gx_name = g_strdup ("GetXProperty");
      else if (strcmp (request->definition->name, "SetProperty") == 0)
	gx_name = g_strdup ("SetXProperty");
      else
	gx_name = g_strdup (request->definition->name);

      out_request = g_new0 (OutputRequest, 1);
      out_request->extension = extension;
      out_request->request = request;
      out_request->obj = obj;
      out_request->xcb_name = g_strdup (request->definition->name);
      out_request->gx_name = gx_name;

      /* If the request has a reply definition then we typedef
       * the reply struct.
       */
      output_reply_typedef (state, parts, out_request);

      /* Some replys include a list of data. If this is such a request
       * then we output a getter function for trailing list fields */
      output_reply_list_get (state, parts, out_request);
      output_reply_list_free (state, parts, out_request);

      output_reply_free (state, parts, out_request);

      /* All requests that have a reply may be issued asynchronously such
       * that the user gets a cookie back for the request and can
       * demand the result at the last moment */
      if (g_list_length (request->definition->reply_fields) > 1)
	{
	  output_async_request (state, parts, out_request);
	  output_reply (state, parts, out_request);
	}

      output_sync_request (state, parts, out_request);

      g_free (out_request->xcb_name);
      g_free (out_request->gx_name);
      g_free (out_request);
    }

  out (parts, GXGEN_PART_COOKIE_OBJ_H_TYPEDEFS,
       "} GXCookieType;\n");
}

static void
output_event_typedefs (GXGenState * state,
		    GXGenExtension * extension,
		    GString ** parts)
{
  GXGenPart typedefs = GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS;
  GList *tmp;
  guint pad = 0;

  for (tmp = extension->events; tmp != NULL; tmp = tmp->next)
    {
      GXGenEvent *event = tmp->data;
      GList *tmp2;

      out (parts, typedefs, "\ntypedef struct {\n");

      for (tmp2 = event->definition->fields; tmp2 != NULL; tmp2 = tmp2->next)
	{
	  GXGenFieldDefinition *field = tmp2->data;
	  if (strcmp (field->name, "pad") == 0)
	    output_pad_field (parts, typedefs, field, pad++);
	  else
	    {
	      /* Dont print trailing list fields */
	      if (!(tmp2->next == NULL && field->length != NULL))
		{
		  out (parts, typedefs,
		       "\t%s %s;\n",
		       gxgen_definition_to_gx_type (field->definition, FALSE),
		       field->name);
		}
	    }
	}
      out (parts, typedefs, "} GX%sEvent;\n", event->definition->name);
    }
}

static void
output_extension_code (GXGenState * state,
		       GXGenExtension * extension,
		       GString ** parts)
{
  output_typedefs (state, extension, parts);

  output_structs_and_unions (state, extension, parts);

  output_enums (state, extension, parts);

  output_requests (state, extension, parts);

  output_event_typedefs (state, extension, parts);
}

static void
output_all_gx_code (GXGenState * state)
{
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
  GString *parts[GXGEN_PART_COUNT];
  int i;
  GList *tmp;

  for (i = 0; i < GXGEN_PART_COUNT; i++)
    parts[i] = g_string_new ("");

  for (tmp = state->extensions; tmp != NULL; tmp = tmp->next)
    {
      GXGenExtension *extension = tmp->data;
      output_extension_code (state, extension, parts);
    }

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


  for (i = 0; i < GXGEN_PART_COUNT; i++)
    g_string_free (parts[i], TRUE);
}

int
main (int argc, char **argv)
{
  int i;
  unsigned long l = 1;
  GXGenState *state = g_new0 (GXGenState, 1);

  state->host_is_le = *(unsigned char *) &l;

  for (i = 1; i < argc && argv[i]; i++)
    gxgen_parse_xmlxcb_file (state, argv[i]);

  state->definitions = g_list_reverse (state->definitions);

  output_all_gx_code (state);

  return 0;
}

