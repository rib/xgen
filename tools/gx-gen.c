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

  GXGEN_PART_COUNT
} GXPart;

typedef enum
{
  GXGEN_VOID,
  GXGEN_BOOLEAN,
  GXGEN_CHAR,
  GXGEN_SIGNED,
  GXGEN_UNSIGNED,
  GXGEN_XID,
  GXGEN_STRUCT,
  GXGEN_UNION,
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

typedef struct GXGenDefinition
{
  char *name;
  GXGenType type;
  union
  {
    unsigned int size;		/* base types */
    GList *fields;		/* struct, union */
    struct GXGenDefinition *ref;	/* typedef / valueparam */
  };

  /* requests */
  GList *reply_fields;

  /* valueparams */
  gchar *mask_name;
  gchar *list_name;

} GXGenDefinition;

typedef enum GXGenExpressionType
{
  GXGEN_FIELDREF,
  GXGEN_VALUE,
  GXGEN_OP
} GXGenExpressionType;

typedef enum GXGenOp
{
  GXGEN_ADD,
  GXGEN_SUBTRACT,
  GXGEN_MULTIPLY,
  GXGEN_DIVIDE,
  GXGEN_LEFT_SHIFT,
  GXGEN_BITWISE_AND
} GXGenOp;

typedef struct GXGenExpression
{
  GXGenExpressionType type;
  union
  {
    char *field;		/* Field name for GXGEN_FIELDREF */
    unsigned long value;	/* Value for GXGEN_VALUE */
    struct			/* Operator and operands for GXGEN_OP */
    {
      GXGenOp op;
      struct GXGenExpression *left;
      struct GXGenExpression *right;
    };
  };
} GXGenExpression;

typedef struct GXGenFieldDefinition
{
  char *name;
  GXGenDefinition *definition;
  GXGenExpression *length;	/* List length; NULL for non-list */
} GXGenFieldDefinition;

typedef struct GXGenFieldValue
{
  GXGenFieldDefinition *field;
  unsigned int offset;
  union
  {
    unsigned char bool_value;
    char char_value;
    signed long signed_value;
    unsigned long unsigned_value;
  };
} GXGenFieldValue;


/* Concrete definitions for opaque and private structure types. */
typedef struct GXGenRequest
{
  unsigned char opcode;
  GXGenDefinition *definition;
} GXGenRequest;

typedef struct GXGenEvent
{
  unsigned char number;
  GXGenDefinition *definition;
} GXGenEvent;

typedef struct GXGenError
{
  unsigned char number;
  GXGenDefinition *definition;
} GXGenError;

typedef struct GXGenExtension
{
  char *name;
  char *xname;
  GList *requests;
  GList *events;
  GList *errors;
} GXGenExtension;

typedef struct GXGenState
{
  unsigned char host_is_le;
  GList *definitions;
  GList *extensions;
} GXGenState;

static const GXGenDefinition core_type_definitions[] = {
  {"void", GXGEN_VOID, 0},
  {"char", GXGEN_CHAR, 1},
  {"BOOL", GXGEN_BOOLEAN, 1},
  {"BYTE", GXGEN_UNSIGNED, 1},
  {"CARD8", GXGEN_UNSIGNED, 1},
  {"CARD16", GXGEN_UNSIGNED, 2},
  {"CARD32", GXGEN_UNSIGNED, 4},
  {"INT8", GXGEN_SIGNED, 1},
  {"INT16", GXGEN_SIGNED, 2},
  {"INT32", GXGEN_SIGNED, 4}
};

struct type_mapping
{
  char *from;
  char *to;
};

static struct type_mapping core_type_mappings[] = {
  {"void", "void"},
  {"char", "gchar"},
  {"BOOL", "guint8"},
  {"BYTE", "guint8"},
  {"CARD8", "guint8"},
  {"CARD16", "guint16"},
  {"CARD32", "guint32"},
  {"INT8", "gint8"},
  {"INT16", "gint16"},
  {"INT32", "gint32"},

  NULL
};


static GXGenDefinition *gxgen_find_type (GXGenState * state, char *name);
static xmlNode *gxgen_xml_next_elem (xmlNode * elem);
static GList *gxgen_parse_reply_fields (GXGenState * state, xmlNode * elem);
static GList *gxgen_parse_fields (GXGenState * state, xmlNode * elem);
static GXGenExpression *gxgen_parse_expression (GXGenState * state,
						 xmlNode * elem);

static void
gxgen_cleanup (GXGenState * state)
{
  GList *tmp;
  for (tmp = state->definitions; tmp != NULL; tmp = tmp->next)
    {
      g_free (tmp->data);
    }
  g_list_free (state->definitions);
  /* FIXME: incomplete */
}


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

static void
gxgen_parse_xmlxcb_file (GXGenState * state, char *filename)
{
  xmlDoc *doc;
  xmlNode *root, *elem;
  char *extension_xname;
  GList *tmp;
  GXGenExtension *extension = NULL;

  /* Ignore text nodes consisting entirely of whitespace. */
  xmlKeepBlanksDefault (0);

  doc = xmlParseFile (filename);
  if (!doc)
    return;

  root = xmlDocGetRootElement (doc);
  if (!root)
    return;

  extension_xname = gxgen_xml_get_prop (root, "extension-xname");
  if (!extension_xname)
    extension_xname = g_strdup ("Core");

  printf ("Extension: %s\n", extension_xname);

  for (tmp = state->extensions; tmp != NULL; tmp = tmp->next)
    {
      if (strcmp (((GXGenExtension *) tmp->data)->xname,
		  extension_xname) == 0)
	{
	  extension = tmp->data;
	  break;
	}
    }

  if (extension == NULL)
    {
      extension = g_new0 (GXGenExtension, 1);
      extension->name = g_strdup (gxgen_xml_get_prop (root,
						       "extension-name"));
      if (!extension->name && strcmp (extension_xname, "Core") == 0)
	extension->name = g_strdup ("Core");
      extension->xname = g_strdup (extension_xname);
      state->extensions = g_list_prepend (state->extensions, extension);
    }

  for (elem = root->children;
       elem != NULL; elem = gxgen_xml_next_elem (elem->next))
    {
#if 0
      /* FIXME: Remove this */
      {
	char *name = gxgen_xml_get_prop (elem, "name");
	printf ("DEBUG:    Parsing element \"%s\", name=\"%s\"\n",
		gxgen_xml_get_node_name (elem),
		name ? name : "<not present>");
      }
#endif

      if (strcmp (gxgen_xml_get_node_name (elem), "request") == 0)
	{
	  GXGenDefinition *def;
	  GXGenFieldDefinition *field;
	  GXGenFieldDefinition *first_byte_field;
	  GList *fields;
	  int opcode = atoi (gxgen_xml_get_prop (elem, "opcode"));

	  def = g_new0 (GXGenDefinition, 1);
	  //def->name = gxgen_make_name(extension,
	  //                             gxgen_xml_get_prop(elem, "name"));
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_REQUEST;

	  fields = gxgen_parse_fields (state, elem);
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

	  if (extension)
	    {
	      GXGenRequest *request = g_new0 (GXGenRequest, 1);
	      request->opcode = opcode;
	      request->definition = def;
	      extension->requests =
		g_list_prepend (extension->requests, request);
	    }
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "event") == 0)
	{
	  char *no_sequence_number;
	  GXGenDefinition *def;
	  GXGenFieldDefinition *field;
	  GList *fields;
	  int number = atoi (gxgen_xml_get_prop (elem, "number"));

	  def = g_new0 (GXGenDefinition, 1);
	  //def->name = gxgen_make_name(extension,
	  //                             gxgen_xml_get_prop(elem, "name"));
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_EVENT;

	  fields = gxgen_parse_fields (state, elem);
#if 0
	  if (fields == NULL)
	    {
	      field = g_new0 (GXGenFieldDefinition, 1);
	      field->name = g_strdup ("pad");
	      field->definition = gxgen_find_type (state, "CARD8");
	      fields = g_list_prepend (fields, field);
	    }
#endif

	  no_sequence_number =
	    gxgen_xml_get_prop (elem, "no-sequence-number");
	  if (!no_sequence_number)
	    {
	      field = g_new0 (GXGenFieldDefinition, 1);
	      field->name = g_strdup ("sequence");
	      field->definition = gxgen_find_type (state, "CARD16");
	      fields = g_list_prepend (fields, field);
	    }

	  field = g_new0 (GXGenFieldDefinition, 1);
	  field->name = g_strdup ("type");
	  field->definition = gxgen_find_type (state, "BYTE");
	  fields = g_list_prepend (fields, field);

	  def->fields = fields;

	  state->definitions = g_list_prepend (state->definitions, def);

	  if (extension)
	    {
	      GXGenEvent *event = g_new0 (GXGenEvent, 1);
	      event->number = number;
	      event->definition = def;
	      extension->events = g_list_prepend (extension->events, event);
	    }
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "eventcopy") == 0)
	{
	  GXGenDefinition *def;
	  int number = atoi (gxgen_xml_get_prop (elem, "number"));

	  def = g_new0 (GXGenDefinition, 1);
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_TYPEDEF;
	  def->ref = gxgen_find_type (state,
				       gxgen_xml_get_prop (elem, "ref"));

	  if (extension)
	    {
	      GXGenEvent *event = g_new0 (GXGenEvent, 1);
	      event->number = number;
	      event->definition = def;
	      extension->events = g_list_prepend (extension->events, event);
	    }
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
	  //def->name = gxgen_make_name(extension,
	  //                             gxgen_xml_get_prop(elem, "name"));
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_STRUCT;
	  def->fields = gxgen_parse_fields (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "xidunion") == 0)
	{
	  GXGenDefinition *def = g_new0 (GXGenDefinition, 1);
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_XIDUNION;
	  def->fields = gxgen_parse_fields (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "union") == 0)
	{
	  GXGenDefinition *def = g_new0 (GXGenDefinition, 1);
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_UNION;
	  def->fields = gxgen_parse_fields (state, elem);
	  state->definitions = g_list_prepend (state->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "xidtype") == 0)
	{
	  GXGenDefinition *def = g_new0 (GXGenDefinition, 1);
	  //def->name = gxgen_make_name(extension,
	  //                             gxgen_xml_get_prop(elem, "name"));
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "name"));
	  def->type = GXGEN_XID;
	  def->size = 4;
	  state->definitions = g_list_prepend (state->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "enum") == 0)
	{
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "typedef") == 0)
	{
	  GXGenDefinition *def = g_new0 (GXGenDefinition, 1);
	  //def->name = gxgen_make_name(extension,
	  //                             gxgen_xml_get_prop(elem, "newname"));
	  def->name = g_strdup (gxgen_xml_get_prop (elem, "newname"));
	  def->type = GXGEN_TYPEDEF;
	  def->ref = gxgen_find_type (state,
				       gxgen_xml_get_prop (elem, "oldname"));
	  state->definitions = g_list_prepend (state->definitions, def);
	}
      else if (strcmp (gxgen_xml_get_node_name (elem), "import") == 0)
	{
	}
    }
}

#if 0
static char *
gxgen_make_name (GXGenExtension * extension, char *name)
{
  if (extension)
    {
      char *temp = malloc (strlen (extension->name) + strlen (name) + 1);
      if (temp == NULL)
	return NULL;
      strcpy (temp, extension->name);
      strcat (temp, name);
      return temp;
    }
  else
    return strdup (name);
}
#endif

static GXGenDefinition *
gxgen_find_type (GXGenState * state, char *name)
{
  GList *tmp;
  for (tmp = state->definitions; tmp != NULL; tmp = tmp->next)
    {
      GXGenDefinition *def = tmp->data;

      /* FIXME: does not work for extension types. */
      if (strcmp (def->name, name) == 0)
	return def;
    }
  g_assert (0);
  return NULL;
}

static xmlNode *
gxgen_xml_next_elem (xmlNode * elem)
{
  while (elem && elem->type != XML_ELEMENT_NODE)
    elem = elem->next;
  return elem;
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

  return gxgen_parse_fields (state, reply);
}

static GList *
gxgen_parse_fields (GXGenState * state, xmlNode * elem)
{
  xmlNode *cur;
  //GXGenFieldDefinition *head;
  //GXGenFieldDefinition **tail = &head;
  GList *fields = NULL;

  for (cur = elem->children; cur != NULL;
       cur = gxgen_xml_next_elem (cur->next))
    {
      GXGenFieldDefinition *field;
      /* FIXME: handle elements other than "field", "pad", and "list". */
      //*tail = g_new0(GXGenFieldDefinition, 1);
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
	  definition->ref =
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
      //tail = &((*tail)->next);
      fields = g_list_prepend (fields, field);
    }

  fields = g_list_reverse (fields);
  //*tail = NULL;
  return fields;
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
normalise_name (const char *name)
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

static const char *
definition_to_gx_type (GXGenDefinition * definition,
		       gboolean object_types)
{
  struct type_mapping *mapping;

  for (mapping = core_type_mappings; mapping->from != NULL; mapping++)
    {
      if (strcmp (mapping->from, definition->name) == 0)
	return mapping->to;
    }

  if (strcmp (definition->name, "WINDOW") == 0)
    return object_types ? "GXWindow *" : "guint32";
  else if (strcmp (definition->name, "DRAWABLE") == 0)
    return object_types ? "GXDrawable *" : "guint32";
  else if (strcmp (definition->name, "PIXMAP") == 0)
    return object_types ? "GXPixmap *" : "guint32";
  else if (strcmp (definition->name, "GCONTEXT") == 0)
    return object_types ? "GXGContext *" : "guint32";
  //else if (strcmp (definition->name, "COLORMAP") == 0)
  //    return "GXColorMap";

  return g_strdup_printf ("GX%s", definition->name);
}

static void
out (GString ** parts, GXPart part, const char *format, ...)
{
  va_list ap;

  va_start (ap, format);
  g_string_append_vprintf (parts[part], format, ap);
  va_end (ap);
}

static void
out_list_get (GString ** parts,
	      GXPart part,
	      const char *obj_name, GXGenFieldDefinition * list_field)
{

}

static void
out_pad_member (GString ** parts,
		GXPart part, GXGenFieldDefinition * field, int index)
{
  char *pad_name;

  if (field->length->value == 1)
    pad_name = g_strdup_printf ("pad%u", index);
  else
    pad_name = g_strdup_printf ("pad%u[%lu]", index, field->length->value);

  out (parts, part, "\t%s %s;\n",
       definition_to_gx_type (field->definition, FALSE), pad_name);
  g_free (pad_name);
}

static void
out_field_xcb_reference (GString ** parts,
			 GXPart part, GXGenFieldDefinition * field)
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
out_typedefs (GXGenState * state, GXGenExtension * extension, GString ** parts)
{
  GList *tmp;

  for (tmp = state->definitions; tmp != NULL; tmp = tmp->next)
    {
      GXGenDefinition *def = tmp->data;

      if (def->type == GXGEN_BOOLEAN
	  || def->type == GXGEN_CHAR
	  || def->type == GXGEN_SIGNED || def->type == GXGEN_UNSIGNED)
	{
	  if (strcmp (def->name, "char") == 0)
	    continue;
	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef %s %s;\n", definition_to_gx_type (def, FALSE),
	       def->name);
	}
      else if (def->type == GXGEN_XID || def->type == GXGEN_XIDUNION)
	{
	  if (is_special_xid_definition (def))
	    continue;

	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef guint32 %s;\n", definition_to_gx_type (def, FALSE));
	}
    }
  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "\n");

  for (tmp = state->definitions; tmp != NULL; tmp = tmp->next)
    {
      GXGenDefinition *def = tmp->data;

      if (def->type == GXGEN_TYPEDEF)
	{
	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef %s %s;\n",
	       def->ref->name, definition_to_gx_type (def, FALSE));
	}
    }
  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "\n\n\n");

  /* NB: Some of the struct definitions depend on the typedef definitions
   * above. */
  for (tmp = state->definitions; tmp != NULL; tmp = tmp->next)
    {
      GXGenDefinition *def = tmp->data;
      guint pad = 0;

      if (def->type == GXGEN_STRUCT)
	{
	  GList *tmp2;

	  /* Some types are special cased if they are represented as
	   * objects */
	  if (strcmp (def->name, "SCREEN") == 0)
	    continue;

	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "typedef struct {\n");

	  for (tmp2 = def->fields; tmp2 != NULL; tmp2 = tmp2->next)
	    {
	      GXGenFieldDefinition *field = tmp2->data;
	      if (strcmp (field->name, "pad") == 0)
		out_pad_member (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
				field, pad++);
	      else
		{
		  /* Dont print trailing list members */
		  if (!(tmp2->next == NULL && field->length != NULL))
		    {
		      out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
			   "\t%s %s;\n",
			   definition_to_gx_type (field->definition, FALSE),
			   field->name);
		    }
		}
	    }

	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "");
	  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS,
	       "} %s;\n\n", definition_to_gx_type (def, FALSE));
	}
    }
  out (parts, GXGEN_PART_CONNECTION_OBJ_H_TYPEDEFS, "\n");

}

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
  GXPart h_typedefs, h_protos, c_funcs;
} GXGenOutputObject;

static GXGenOutputObject *
identify_request_object (GXGenRequest *request)
{
  GXGenOutputObject *obj = g_new0(GXGenOutputObject, 1);
  GList *tmp;

  /* Look at the first field (after those that are ignored)
   * to identify what object this request belongs too.
   *
   * The default object for requests to become members of is
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
out_reply_typedef (GXGenState * state,
		   GXGenExtension * extension,
		   GString ** parts,
		   GXGenRequest *request,
		   GXGenOutputObject *obj)
{
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
	out_pad_member (parts, obj->h_typedefs, field, pad++);
      else
	{
	  /* Dont print trailing list members */
	  if (!(tmp->next == NULL && field->length != NULL))
	    {
	      //out_list_get (parts, c_funcs, object_name, field);

	      out (parts, obj->h_typedefs,
		   "\t%s %s;\n",
		   definition_to_gx_type (field->definition, FALSE),
		   field->name);
	    }
	}
    }
  out (parts, obj->h_typedefs,
       "\n} GX%sX11Reply;\n\n", request->definition->name);

  out (parts, obj->h_typedefs, "typedef struct {\n");
  out (parts, obj->h_typedefs, "\tGXConnection *connection;\n");
  out (parts, obj->h_typedefs, "\tGX%sX11Reply *x11_reply;\n",
       request->definition->name);
  out (parts, obj->h_typedefs,
       "\n} GX%sReply;\n\n", request->definition->name);
}

static void
out_reply_list_get (GXGenState * state,
		    GXGenExtension * extension,
		    GString ** parts,
		    GXGenRequest *request,
		    GXGenOutputObject *obj)
{
  GXGenFieldDefinition *field;
  char *name_lc;

  if (!request->definition->reply_fields)
    return;

  field = (g_list_last (request->definition->reply_fields))->data;
  if (field->length == NULL)
    return;

  /* FIXME - shouldn't be restricted to FIELDREF length types */
  if (field->length->type != GXGEN_FIELDREF)
    return;

  name_lc = normalise_name (request->definition->name);

  out (parts, obj->h_protos,
       "GArray *\n"
       "gx_%s_%s_get_%s (GX%sReply *%s_reply);\n",
       obj->name,
       name_lc,
       field->name,
       request->definition->name,
       name_lc);

  out (parts, obj->c_funcs,
       "GArray *\n"
       "gx_%s_%s_get_%s (GX%sReply *%s_reply)\n",
       obj->name,
       name_lc,
       field->name,
       request->definition->name,
       name_lc);

  out (parts, obj->c_funcs,
       "{\n");

  out (parts, obj->c_funcs,
       "  %s *p = (%s *)(%s_reply->x11_reply + 1);\n",
       definition_to_gx_type (field->definition, FALSE),
       definition_to_gx_type (field->definition, FALSE),
       name_lc);


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
       definition_to_gx_type (field->definition, FALSE));

  if (is_special_xid_definition (field->definition))
    {
      out (parts, obj->c_funcs,
       "  for (i = 0; i< %s_reply->x11_reply->%s; i++)\n"
       "    {\n",
       name_lc,
       field->length->field);

      out (parts, obj->c_funcs,
       "      /* FIXME - mutex */\n");

      out (parts, obj->c_funcs,
       "      %s item = _gx_%s_find_from_xid (p[i]);\n",
       definition_to_gx_type (field->definition, TRUE),
       obj->name);

      out (parts, obj->c_funcs,
       "      if (!item)\n"
       "	item = g_object_new (gx_%s_get_type(),\n"
       "			     \"connection\", %s_reply->connection,\n"
       "			     \"xid\", p[i],\n"
       "			     \"wrap\", TRUE,\n"
       "			     NULL);\n",
       obj->name, name_lc);

      out (parts, obj->c_funcs,
       "      tmp = g_array_append_val (tmp, item);\n"
       "    }\n");
    }
  else
    {
      out (parts, obj->c_funcs,
       "  tmp = g_array_append_vals (tmp, p, %s_reply->x11_reply->%s);\n",
       name_lc, field->length->field);
    }

  out (parts, obj->c_funcs,
       "  return tmp;\n");

  out (parts, obj->c_funcs,
       "}\n");

  g_free (name_lc);
}

static void
out_reply_list_free (GXGenState * state,
		     GXGenExtension * extension,
		     GString ** parts,
		     GXGenRequest *request,
		     GXGenOutputObject *obj)
{
  GXGenFieldDefinition *field;
  char *name_lc;

  if (!request->definition->reply_fields)
    return;

  field = (g_list_last (request->definition->reply_fields))->data;
  if (field->length == NULL)
    return;

  /* FIXME - shouldn't be restricted to FIELDREF length types */
  if (field->length->type != GXGEN_FIELDREF)
    return;

  name_lc = normalise_name (request->definition->name);

  out (parts, obj->h_protos,
       "void\n"
       "gx_%s_%s_free_%s (GArray *%s);\n",
       obj->name,
       name_lc,
       field->name,
       field->name);

  out (parts, obj->c_funcs,
       "void\n"
       "gx_%s_%s_free_%s (GArray *%s)\n",
       obj->name,
       name_lc,
       field->name,
       field->name);

  out (parts, obj->c_funcs,
       "{\n");

  if (is_special_xid_definition (field->definition))
    {
      out (parts, obj->c_funcs,
       "  %s*p = %s->data;\n"
       "  int i;\n"
       "  for (i = 0; i < %s->len; i++)\n"
       "      g_object_unref (p[i]);\n",
       definition_to_gx_type (field->definition, TRUE),
       field->name,
       field->name);
    }

  out (parts, obj->c_funcs, "g_array_free (%s, TRUE);\n", field->name);

  out (parts, obj->c_funcs,
       "}\n");
}

static void
out_reply_free (GXGenState * state,
		GXGenExtension * extension,
		GString ** parts,
		GXGenRequest *request,
		GXGenOutputObject *obj)
{
  char *name_lc;

  if (!request->definition->reply_fields)
    return;

  name_lc = normalise_name (request->definition->name);

  out (parts, obj->h_protos,
       "void\n"
       "gx_%s_%s_reply_free (GX%sReply *%s_reply);\n",
       obj->name,
       name_lc,
       request->definition->name,
       name_lc);

  out (parts, obj->c_funcs,
       "void\n"
       "gx_%s_%s_reply_free (GX%sReply *%s_reply)\n",
       obj->name,
       name_lc,
       request->definition->name,
       name_lc);

  out (parts, obj->c_funcs,
       "{\n");

  out (parts, obj->c_funcs,
       "  free (%s_reply->x11_reply);\n",
       name_lc);
  out (parts, obj->c_funcs,
       "  g_slice_free (GX%sReply, %s_reply);\n",
       request->definition->name, name_lc);

  out (parts, obj->c_funcs,
       "}\n");
}

static void
out_requests (GXGenState * state, GXGenExtension * extension, GString ** parts)
{
  GList *tmp;

  for (tmp = extension->requests; tmp != NULL; tmp = tmp->next)
    {
      GXGenRequest *request = tmp->data;
      GXGenOutputObject *obj = NULL;
      GList *tmp2;
      char *name_lc;
      GString *scratch;

      /* Some constructor type requests are special cased as object
       * constructors */
      if (strcmp (request->definition->name, "CreateWindow") == 0
	  || strcmp (request->definition->name, "CreatePixmap") == 0
	  || strcmp (request->definition->name, "CreateGC") == 0)
	continue;

      obj = identify_request_object (request);

      name_lc = normalise_name (request->definition->name);

      /* If the request has a reply definition then we typedef
       * the reply struct.
       */
      out_reply_typedef (state, extension, parts, request, obj);

      /* Some replys include a list of data. If this is such a request
       * then we output a getter function for trailing list fields */
      out_reply_list_get (state, extension, parts, request, obj);
      out_reply_list_free (state, extension, parts, request, obj);

      out_reply_free (state, extension, parts, request, obj);

      /* the get/set_property names clash with the gobject
       * property accessor functions */
      if (strcmp (name_lc, "get_property") == 0)
	out (parts, obj->h_typedefs, "GXCookie *\ngx_%s_get_xproperty_async(%s",
	     obj->name, obj->first_arg);
      else if (strcmp (name_lc, "set_property") == 0)
	out (parts, obj->h_typedefs, "GXCookie *\ngx_%s_set_xproperty_async(%s",
	     obj->name, obj->first_arg);
      else
	out (parts, obj->h_typedefs, "GXCookie *\ngx_%s_%s_async(%s",
	     obj->name, name_lc, obj->first_arg);

      for (tmp2 = request->definition->fields; tmp2 != NULL; tmp2 = tmp2->next)
	{
	  GXGenFieldDefinition *field = tmp2->data;
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

	  type = definition_to_gx_type (field->definition, TRUE);

	  if (field->length)
	    out (parts, obj->h_typedefs, ",\n\t\tconst %s *%s", type, field->name);
	  else if (field->definition->type == GXGEN_VALUEPARAM)
	    {
	      const char *valueparam_type =
		definition_to_gx_type (field->definition->ref, FALSE);
	      out (parts, obj->h_typedefs, ",\n\t\t%s %s", valueparam_type,
		   field->definition->mask_name);
	      out (parts, obj->h_typedefs, ",\n\t\t%s *%s", valueparam_type,
		   field->definition->list_name);
	    }
	  else
	    out (parts, obj->h_typedefs, ",\n\t\t%s %s", type, field->name);
	}
      out (parts, obj->h_typedefs, ");\n\n");


      scratch = g_string_new ("");
      if (!request->definition->reply_fields
	  || g_list_length (request->definition->reply_fields) == 1)
	g_string_append (scratch, "void\n");
      else
	g_string_append_printf (scratch, "GX%sReply *\n",
				request->definition->name);


      /* the get/set_property names clash with the gobject
       * property accessor functions */
      if (strcmp (name_lc, "get_property") == 0)
	g_string_append_printf (scratch, "gx_%s_get_xproperty(%s",
				obj->name, obj->first_arg);
      else if (strcmp (name_lc, "set_property") == 0)
	g_string_append_printf (scratch, "gx_%s_set_xproperty(%s",
				obj->name, obj->first_arg);
      else
	g_string_append_printf (scratch, "gx_%s_%s(%s",
				obj->name, name_lc, obj->first_arg);

      out (parts, obj->h_protos, "%s", scratch->str);
      out (parts, obj->c_funcs, "%s", scratch->str);
      //g_free (name_lc);
      g_string_free (scratch, TRUE);


      for (tmp2 = request->definition->fields;
	   tmp2 != NULL; tmp2 = tmp2->next)
	{
	  GXGenFieldDefinition *field = tmp2->data;
	  const char *type;

	  if (strcmp (field->name, "opcode") == 0
	      || strcmp (field->name, "pad") == 0
	      || strcmp (field->name, "length") == 0)
	    continue;

	  if (obj->first_object_field && field == obj->first_object_field)
	    continue;

	  type = definition_to_gx_type (field->definition, TRUE);

	  if (field->length)
	    {
	      out (parts, obj->h_protos, ",\n\t\tconst %s *%s", type, field->name);
	      out (parts, obj->c_funcs, ",\n\t\tconst %s *%s", type, field->name);
	    }
	  else if (field->definition->type == GXGEN_VALUEPARAM)
	    {
	      const char *valueparam_type =
		definition_to_gx_type (field->definition->ref, FALSE);

	      out (parts, obj->h_protos, ",\n\t\t%s %s", valueparam_type,
		   field->definition->mask_name);
	      out (parts, obj->h_protos, ",\n\t\t%s *%s", valueparam_type,
		   field->definition->list_name);
	      out (parts, obj->c_funcs, ",\n\t\t%s %s", valueparam_type,
		   field->definition->mask_name);
	      out (parts, obj->c_funcs, ",\n\t\t%s *%s", valueparam_type,
		   field->definition->list_name);
	    }
	  else
	    {
	      out (parts, obj->h_protos, ",\n\t\t%s %s", type, field->name);
	      out (parts, obj->c_funcs, ",\n\t\t%s %s", type, field->name);
	    }
	}
      out (parts, obj->h_protos, ");\n\n");
      out (parts, obj->c_funcs, ")\n{\n");

      //name_lc = normalise_name (request->definition->name);

      if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
	{
	  out (parts, obj->c_funcs,
	       "\n\tGXConnection *connection = gx_%s_get_connection (%s);\n",
	       obj->name, obj->name);
	}

      if (g_list_length (request->definition->reply_fields) <= 1)
	{
	  out (parts, obj->c_funcs, "\txcb_void_cookie_t cookie;\n");
	}
      else
	{
	  out (parts, obj->c_funcs,
	       "\txcb_generic_error_t *xcb_error;\n", name_lc);
	  out (parts, obj->c_funcs, "\txcb_%s_cookie_t cookie;\n", name_lc);
	  out (parts, obj->c_funcs,
	       "\tGX%sReply *reply = g_slice_new (GX%sReply);\n",
	       request->definition->name,
	       request->definition->name);
	}

      out (parts, obj->c_funcs, "\n");

      if (g_list_length (request->definition->reply_fields) > 1)
	  out (parts, obj->c_funcs,
	       "\treply->connection = connection;\n\n");

      out (parts, obj->c_funcs,
	   "\tcookie =\n"
	   "\t\txcb_%s(\n"
	   "\t\t\tgx_connection_get_xcb_connection (connection)", name_lc);

      for (tmp2 = request->definition->fields; tmp2 != NULL;
	   tmp2 = tmp2->next)
	{
	  GXGenFieldDefinition *field = tmp2->data;

	  if (strcmp (field->name, "opcode") == 0
	      || strcmp (field->name, "pad") == 0
	      || strcmp (field->name, "length") == 0)
	    continue;

	  if (field->definition->type != GXGEN_VALUEPARAM)
	    {
	      /* Some special cased field types require a function call
	       * to lookup their counterpart xcb value */
	      out (parts, obj->c_funcs, ",\n\t\t\t");
	      out_field_xcb_reference (parts, obj->c_funcs, field);
	    }
	  else
	    {
	      const char *valueparam_type =
		definition_to_gx_type (field->definition->ref, FALSE);
	      out (parts, obj->c_funcs, ",\n\t\t\t%s",
		   field->definition->mask_name);
	      out (parts, obj->c_funcs, ",\n\t\t\t%s",
		   field->definition->list_name);
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
	       request->definition->name,
	       name_lc);
	}

      if (!obj->type == GXGEN_IS_CONNECTION_OBJ)
	out (parts, obj->c_funcs, "\tg_object_unref (connection);\n");

      if (g_list_length (request->definition->reply_fields) > 1)
	{
	  out (parts, obj->c_funcs, "\n\treturn reply;\n");
	}
      out (parts, obj->c_funcs, "}\n\n");

      g_free (name_lc);
    }

}

static void
gen_code (GXGenState * state, GXGenExtension * extension, GString ** parts)
{
  out_typedefs (state, extension, parts);

  out_requests (state, extension, parts);
}

static void
gen_gx_code (GXGenState * state, GXGenExtension * extension)
{
  //char *extension_h_name = g_strdup_printf("%s.h",extension->name);
  //char *extension_c_name = g_strdup_printf("%s.c",extension->name);
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
  GString *parts[GXGEN_PART_COUNT];
  int i;

  for (i = 0; i < GXGEN_PART_COUNT; i++)
    parts[i] = g_string_new ("");

  gen_code (state, extension, parts);

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


  for (i = 0; i < GXGEN_PART_COUNT; i++)
    g_string_free (parts[i], TRUE);
}

int
main (int argc, char **argv)
{
  int i;
  GXGenState *state = g_new0 (GXGenState, 1);
  GList *tmp;

  {
    unsigned long l = 1;
    state->host_is_le = *(unsigned char *) &l;
  }

  /* Add definitions of core types. */
  for (i = 0; i < sizeof (core_type_definitions) / sizeof (GXGenDefinition);
       i++)
    {
      GXGenDefinition *temp = g_new0 (GXGenDefinition, 1);

      *temp = core_type_definitions[i];
      temp->name = g_strdup (core_type_definitions[i].name);

      state->definitions = g_list_prepend (state->definitions, temp);
    }

  gxgen_parse_xmlxcb_file (state, "./xproto.xml");
  //gxgen_parse_xmlxcb_file(state, "./composite.xml");

  state->definitions = g_list_reverse (state->definitions);

  for (tmp = state->extensions; tmp != NULL; tmp = tmp->next)
    {
      GXGenExtension *extension = tmp->data;

      gen_gx_code (state, extension);
    }

  return 0;
}

