/* XGen - XCB protocol specs parser and toolkit
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

#include <xgen.h>

#include <libxml/parser.h>

#include <glib.h>
#include <glib/gprintf.h>

#include <stdarg.h>
#include <string.h>

#define BASE_TYPE(NAME, TYPE, SIZE) \
  { \
   ._parent = { \
      .name = NAME, \
      .type = TYPE \
    }, \
    .size = SIZE \
  }
static const XGenBaseType core_type_definitions[] = {
  BASE_TYPE("void", XGEN_VOID, 0),
  BASE_TYPE("char", XGEN_CHAR, 1),
  BASE_TYPE("float", XGEN_FLOAT, 4),
  BASE_TYPE("double", XGEN_DOUBLE, 8),
  BASE_TYPE("BOOL", XGEN_BOOLEAN, 1),
  BASE_TYPE("BYTE", XGEN_UNSIGNED, 1),
  BASE_TYPE("CARD8", XGEN_UNSIGNED, 1),
  BASE_TYPE("CARD16", XGEN_UNSIGNED, 2),
  BASE_TYPE("CARD32", XGEN_UNSIGNED, 4),
  BASE_TYPE("INT8", XGEN_SIGNED, 1),
  BASE_TYPE("INT16", XGEN_SIGNED, 2),
  BASE_TYPE("INT32", XGEN_SIGNED, 4)
};
#undef BASE_TYPE


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

  for (tmp = extension->all_definitions; tmp != NULL; tmp = tmp->next)
    {
      XGenDefinition *def = tmp->data;

      if (strcmp (def->name, type_name) == 0)
	  return def;
    }

  return NULL;
}

static XGenDefinition *
xgen_find_type (const XGenState * state,
		const XGenExtension *current_extension,
		const char *name)
{
  GList *tmp;
  char **bits;
  char *extension_header = NULL;
  char *type_name;

  bits = g_strsplit (name, ":", 2);

  if (bits[1] != NULL)
    {
      extension_header = bits[0];
      type_name = bits[1];
    }
  else
    {
      extension_header = current_extension->header;
      type_name = bits[0];
    }

  /* First we try in looking in the extension being parsed
   * or the extension that was explicitly specified. */
  for (tmp = state->extensions; tmp != NULL; tmp = tmp->next)
    {
      XGenExtension *extension = tmp->data;
      if (strcmp (extension->header, extension_header) == 0)
	{
	  XGenDefinition *def =
	    xgen_find_type_in_extension (extension, type_name);
	  if (def)
	    return def;
	}
    }

  /* If an extension was explicitly specified we have no where
   * else to look */
  if (bits[1] != NULL)
    {
      g_critical ("Failed to find type = %s\n", name);
      return NULL;
    }

  for (tmp = state->extensions; tmp != NULL; tmp = tmp->next)
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
      xmlFree (temp);
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
xgen_parse_field_elements (XGenState * state,
			   XGenType definition_type,
			   XGenExtension *extension,
			   xmlNode * elem)
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
	  char *bytes = xgen_xml_get_prop (cur, "bytes");
	  field->name = "pad";
	  field->definition = xgen_find_type (state, extension, "CARD8");
	  field->length = g_new0 (XGenExpression, 1);
	  field->length->type = XGEN_VALUE;
	  field->length->value = atoi (bytes);
	  xmlFree (bytes);
	}
      else if (strcmp (xgen_xml_get_node_name (cur), "field") == 0)
	{
	  char *name = xgen_xml_get_prop (cur, "name");
	  field->name = strdup (name);
	  field->definition =
	    xgen_find_type (state,
			    extension,
			    xgen_xml_get_prop (cur, "type"));
	  xmlFree (name);
	}
      else if (strcmp (xgen_xml_get_node_name (cur), "list") == 0)
	{
	  char *name = xgen_xml_get_prop (cur, "name");
	  char *type = xgen_xml_get_prop (cur, "type");
	  field->name = strdup (name);
	  xmlFree (name);
	  field->definition = xgen_find_type (state, extension, type);
	  xmlFree (type);

	  if (cur->children)
	    {
	      field->length = xgen_parse_expression (state, cur->children);
	    }
	  else if (definition_type == XGEN_REPLY)
	    {
	      XGenExpression *exp = g_new0 (XGenExpression, 1);
	      exp->type = XGEN_FIELDREF;
	      exp->field = g_strdup ("length");
	      field->length = exp;
	    }
	  else
	    {
	      XGenFieldDefinition *len_field;
	      XGenExpression *exp;

	      len_field = g_new0 (XGenFieldDefinition, 1);
	      len_field->name = g_strdup_printf ("%s_len", field->name);
	      len_field->definition =
		xgen_find_type (state, extension, "CARD32");

	      fields = g_list_prepend (fields, len_field);

	      exp = g_new0 (XGenExpression, 1);
	      exp->type = XGEN_FIELDREF;
	      exp->field = g_strdup (field->name);
	      field->length = exp;
	    }
	}
      else if (strcmp (xgen_xml_get_node_name (cur), "valueparam") == 0)
	{
	  XGenValueParam *valueparam = g_new0 (XGenValueParam, 1);
	  XGenDefinition *def;
	  char *value_mask_type;
	  char *value_mask_name;
	  char *value_list_name;

	  def = XGEN_DEF (valueparam);
	  def->extension = extension;
	  def->name = g_strdup ("valueparam");
	  def->type = XGEN_VALUEPARAM;

	  value_mask_type = xgen_xml_get_prop (cur, "value-mask-type");
	  valueparam->reference =
	    xgen_find_type (state, extension, value_mask_type);
	  xmlFree (value_mask_type);

	  value_mask_name = xgen_xml_get_prop (cur, "value-mask-name");
	  valueparam->mask_name = strdup (value_mask_name);
	  xmlFree (value_mask_name);

	  value_list_name = xgen_xml_get_prop (cur, "value-list-name");
	  valueparam->list_name = strdup (value_list_name);
	  xmlFree (value_list_name);

	  field->name = g_strdup ("valueparam");
	  field->definition = def;
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
xgen_parse_reply_fields (XGenState * state,
			 XGenExtension *extension,
			 xmlNode * elem)
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

  return xgen_parse_field_elements (state, XGEN_REPLY, extension, reply);
}

/**
 * Simply causes the corresponding xml to be opened, parsed by libxml2
 * and then parsed enough to determine the imports so that we can do
 * full parsing of the xcb definitions in dependency order.
 */
static void
xgen_open_xcb_proto_file (XGenState * state, char *filename)
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
    {
      xmlFreeDoc (doc);
      return;
    }

  extension_name = xgen_xml_get_prop (root, "extension-name");
  if (!extension_name)
    extension_name = g_strdup ("Core");
  extension_header = xgen_xml_get_prop (root, "header");
  g_assert (extension_header);

  g_print ("Extension: %s\n", extension_name);

  extension = g_new0 (XGenExtension, 1);
  extension->_xml_doc = doc; /* FIXME: xmlFreeDoc */
  extension->name = g_strdup (extension_name);
  extension->header = g_strdup (extension_header);
  state->extensions = g_list_prepend (state->extensions, extension);

  xmlFree (extension_name);

  if (strcmp (extension->header, "xproto") == 0)
    {
      int i;

      /* Add definitions of core types. */
      for (i = 0;
	   i < sizeof (core_type_definitions) / sizeof (XGenBaseType);
	   i++)
	{
	  XGenBaseType *base_type = g_new0 (XGenBaseType, 1);
	  XGenDefinition *def = XGEN_DEF (base_type);

	  *base_type = core_type_definitions[i];
	  def->name = g_strdup (core_type_definitions[i]._parent.name);

	  g_assert (def->name);

	  extension->all_definitions =
	    g_list_prepend (extension->all_definitions, base_type);
	  extension->base_types =
	    g_list_prepend (extension->base_types, base_type);
	}
    }

  for (elem = root->children;
       elem != NULL; elem = xgen_xml_next_elem (elem->next))
    {
      if (strcmp (xgen_xml_get_node_name (elem), "import") == 0)
	{
	  char *import_header = xgen_xml_get_node_content (elem);

	  extension->_import_headers =
	    g_list_prepend (extension->_import_headers,
			    g_strdup (import_header));

	  xmlFree (import_header);
	}
    }
}

/**
 * This function deals with parsing all the definitions within the xml protocol
 * specs, but assumes that the imports have been used to ensure that all
 * dependencies are parsed first.
 */
static void
xgen_parse_xcb_proto_file (XGenState *state, XGenExtension *extension)
{
  xmlNode *root, *elem;

  if (extension->_parsed)
    return;

  root = xmlDocGetRootElement (extension->_xml_doc);
  /* This should have already been checked in xgen_open_xcb_proto_file: */
  g_assert (root);

  for (elem = root->children;
       elem != NULL; elem = xgen_xml_next_elem (elem->next))
    {
      /* Since most cases will result in a new XGenDefinition, we declare
       * a pointer in this block so we can put some common code at the end
       * that can fiddle with the definition. */
      XGenDefinition *def = NULL;

      if (strcmp (xgen_xml_get_node_name (elem), "request") == 0)
	{
	  XGenRequest *request = g_new0 (XGenRequest, 1);
	  XGenFieldDefinition *field;
	  XGenFieldDefinition *first_byte_field;
	  GList *fields;
	  int opcode = atoi (xgen_xml_get_prop (elem, "opcode"));

	  def = XGEN_DEF (request);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_REQUEST;

	  if (strcmp (extension->header, "xevie") == 0
	      && strcmp (def->name, "Send") == 0)
	    g_print ("DEBUG xevie send\n");

	  request->opcode = opcode;

	  fields = xgen_parse_field_elements (state, XGEN_REQUEST,
					      extension, elem);
	  if (!fields)
	    {
	      field = g_new0 (XGenFieldDefinition, 1);
	      field->name = g_strdup ("pad");
	      field->definition =
		xgen_find_type (state, extension, "CARD8");
	      fields = g_list_prepend (fields, field);
	    }

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("length");
	  field->definition =
	    xgen_find_type (state, extension, "CARD16");
	  fields = g_list_prepend (fields, field);

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("opcode");
	  field->definition =
	    xgen_find_type (state, extension, "BYTE");
	  fields = g_list_prepend (fields, field);

	  request->fields = fields;
	  extension->requests =
	    g_list_prepend (extension->requests, request);

	  fields = xgen_parse_reply_fields (state, extension, elem);
	  if (fields)
	    {
	      XGenReply *reply = g_new0 (XGenReply, 1);
	      XGenDefinition *reply_def = XGEN_DEF (reply);

	      reply_def->extension = extension;
	      reply_def->name = g_strdup (def->name);
	      reply_def->type = XGEN_REPLY;

	      /* FIXME: assert that sizeof(first_byte_field)==1 */
	      first_byte_field = fields->data;
	      fields = g_list_remove (fields, first_byte_field);

	      field = g_new0 (XGenFieldDefinition, 1);
	      field->name = g_strdup ("length");
	      field->definition =
		xgen_find_type (state, extension, "CARD32");
	      fields = g_list_prepend (fields, field);

	      field = g_new0 (XGenFieldDefinition, 1);
	      field->name = g_strdup ("sequence");
	      field->definition =
		xgen_find_type (state, extension, "CARD16");
	      fields = g_list_prepend (fields, field);

	      fields = g_list_prepend (fields, first_byte_field);

	      field = g_new0 (XGenFieldDefinition, 1);
	      field->name = g_strdup ("response_type");
	      field->definition =
		xgen_find_type (state, extension, "BYTE");
	      fields = g_list_prepend (fields, field);

	      reply->fields = fields;

	      extension->replys =
		g_list_prepend (extension->replys, reply);

	      /* FIXME: NB this is duplicated below for non reply definitions
	       * which is a bit yukky. */
	      g_assert (reply_def->name);
	      extension->all_definitions =
		g_list_prepend (extension->all_definitions, reply_def);

	      request->reply = reply;
	    }
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "event") == 0)
	{
	  XGenEvent *event = g_new0 (XGenEvent, 1);
	  char *no_sequence_number;
	  XGenFieldDefinition *field;
	  XGenFieldDefinition *first_byte_field;
	  GList *fields;
	  int number = atoi (xgen_xml_get_prop (elem, "number"));

	  def = XGEN_DEF (event);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_EVENT;

	  event->number = number;

	  fields = xgen_parse_field_elements (state, XGEN_EVENT,
					      extension, elem);
	  first_byte_field = fields->data;
	  fields = g_list_remove (fields, first_byte_field);

	  no_sequence_number =
	    xgen_xml_get_prop (elem, "no-sequence-number");
	  if (!no_sequence_number)
	    {
	      field = g_new0 (XGenFieldDefinition, 1);
	      field->name = g_strdup ("sequence");
	      field->definition = xgen_find_type (state, extension, "CARD16");
	      fields = g_list_prepend (fields, field);
	    }

	  fields = g_list_prepend (fields, first_byte_field);

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("response_type");
	  field->definition = xgen_find_type (state, extension, "BYTE");
	  fields = g_list_prepend (fields, field);

	  event->fields = fields;

	  extension->events = g_list_prepend (extension->events, event);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "eventcopy") == 0)
	{
	  XGenEvent *event = g_new0 (XGenEvent, 1);
	  int number = atoi (xgen_xml_get_prop (elem, "number"));
	  XGenEvent *copy_of;

	  def = XGEN_DEF (event);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_EVENT;

	  event->number = number;

	  copy_of =
	    XGEN_EVENT_DEF (xgen_find_type (state,
					    extension,
					    xgen_xml_get_prop (elem, "ref")));
	  event->fields = copy_of->fields;
	  /* So that we don't double free the fields: */
	  event->is_copy = TRUE;

	  extension->events = g_list_prepend (extension->events, event);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "error") == 0)
	{
	  XGenError *error = g_new0 (XGenError, 1);
	  int number = atoi (xgen_xml_get_prop (elem, "number"));
	  GList *fields;
	  XGenFieldDefinition *field;

	  def = XGEN_DEF (error);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_ERROR;

	  error->number = number;

	  fields = xgen_parse_field_elements (state, XGEN_ERROR,
					      extension, elem);

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("response");
	  field->definition = xgen_find_type (state, extension, "BYTE");
	  fields = g_list_prepend (fields, field);

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("error_code");
	  field->definition = xgen_find_type (state, extension, "BYTE");
	  fields = g_list_prepend (fields, field);

	  field = g_new0 (XGenFieldDefinition, 1);
	  field->name = g_strdup ("sequence");
	  field->definition = xgen_find_type (state, extension, "CARD16");
	  fields = g_list_prepend (fields, field);

	  error->fields = fields;

	  extension->errors = g_list_prepend (extension->errors, error);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "errorcopy") == 0)
	{
	  XGenError *error = g_new0 (XGenError, 1);
	  int number = atoi (xgen_xml_get_prop (elem, "number"));
	  XGenError *copy_of;

	  def = XGEN_DEF (error);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_ERROR;

	  error->number = number;

	  copy_of =
	    XGEN_ERROR_DEF (xgen_find_type (state,
					    extension,
					    xgen_xml_get_prop (elem, "ref")));
	  error->fields = copy_of->fields;
	  /* So that we don't double free the fields: */
	  error->is_copy = TRUE;

	  extension->errors = g_list_prepend (extension->errors, error);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "struct") == 0)
	{
	  XGenStruct *struct_def = g_new0 (XGenStruct, 1);

	  def = XGEN_DEF (struct_def);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_STRUCT;

	  struct_def->fields =
	    xgen_parse_field_elements (state, XGEN_STRUCT,
				       extension, elem);
	  extension->structs =
	    g_list_prepend (extension->structs, struct_def);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "xidunion") == 0)
	{
	  XGenXIDUnion *xid_union = g_new0 (XGenXIDUnion, 1);

	  def = XGEN_DEF (xid_union);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_XIDUNION;

	  xid_union->fields =
	    xgen_parse_field_elements (state, XGEN_XIDUNION,
				       extension, elem);
	  extension->xid_unions =
	    g_list_prepend (extension->xid_unions, xid_union);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "union") == 0)
	{
	  XGenUnion *union_def = g_new0 (XGenUnion, 1);

	  def = XGEN_DEF (union_def);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_UNION;

	  union_def->fields =
	    xgen_parse_field_elements (state, XGEN_UNION,
				       extension, elem);
	  extension->unions =
	    g_list_prepend (extension->unions, union_def);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "xidtype") == 0)
	{
	  XGenBaseType *xid_def = g_new0 (XGenBaseType, 1);

	  def = XGEN_DEF (xid_def);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_XID;

	  xid_def->size = 4;
	  extension->base_types =
	    g_list_prepend (extension->base_types, xid_def);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "enum") == 0)
	{
	  XGenEnum *enum_def = g_new0 (XGenEnum, 1);

	  def = XGEN_DEF (enum_def);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "name"));
	  def->type = XGEN_ENUM;

	  enum_def->items = xgen_parse_item_elements (state, elem);
	  extension->enums =
	    g_list_prepend (extension->enums, enum_def);
	}
      else if (strcmp (xgen_xml_get_node_name (elem), "typedef") == 0)
	{
	  XGenTypedef *typedef_def = g_new0 (XGenTypedef, 1);
	  char *oldname;

	  def = XGEN_DEF (typedef_def);
	  def->extension = extension;
	  def->name = g_strdup (xgen_xml_get_prop (elem, "newname"));
	  def->type = XGEN_TYPEDEF;

	  oldname = xgen_xml_get_prop (elem, "oldname");
	  typedef_def->reference =
	    xgen_find_type (state, extension, oldname);
	  xmlFree (oldname);

	  extension->typedefs =
	    g_list_prepend (extension->typedefs, typedef_def);
	}

      if (def)
	{
	  /* FIXME: NB this stuff is duplicated above for reply definitions
	   * which is a bit yukky. */
	  g_assert (def->name);
	  extension->all_definitions =
	    g_list_prepend (extension->all_definitions, def);
	}
    }

  extension->all_definitions = g_list_reverse (extension->all_definitions);

  extension->_parsed = TRUE;
}

/**
 * This function recursivly works to parse all imported dependencies of an xcb
 * protocol extensions and then parse the current extension.
 */
static void
xgen_parse_xcb_proto_and_imports (XGenState *state, XGenExtension *extension)
{
  GList *tmp;

  if (extension->_parsed)
    return;

  if (!extension->imports)
    {
      xgen_parse_xcb_proto_file (state, extension);
      return;
    }

  for (tmp = extension->imports; tmp != NULL; tmp = tmp->next)
    {
      XGenExtension *import = tmp->data;

      xgen_parse_xcb_proto_and_imports (state, import);
    }

  xgen_parse_xcb_proto_file (state, extension);
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

static XGenExtension *
find_extension (XGenState *state, gchar *extension_header)
{
  GList *tmp;

  for (tmp = state->extensions; tmp != NULL; tmp=tmp->next)
    {
      XGenExtension *extension = tmp->data;

      if (strcmp (extension->header, extension_header) == 0)
	return extension;
    }
  return NULL;
}

/**
 * When first parsing the xml files we create a GList of import_header
 * names for each extension. This function uses the header names to
 * create a list of XGenExtension pointers.
 *
 * This function returns FALSE if any of the imported extensions can't
 * be resolved.
 */
static gboolean
resolve_imports (XGenState *state)
{
  GList *tmp;

  for (tmp = state->extensions; tmp != NULL; tmp=tmp->next)
    {
      XGenExtension *extension = tmp->data;
      GList *tmp2;

      for (tmp2 = extension->_import_headers; tmp2 != NULL; tmp2 = tmp2->next)
	{
	  gchar *import_header = tmp2->data;
	  XGenExtension *import;

	  import = find_extension (state, import_header);
	  if (!import)
	    {
	      g_warning ("Failed to resolve imports: "
			 "%s imports %s which wasn't found",
			 extension->header,
			 import_header);
	      return FALSE;
	    }

	  extension->imports = g_list_prepend (extension->imports, import);
	}
    }
  return TRUE;
}

/**
 * xgen_parse_xcb_proto_files:
 * @files: A list of xcb xml protocol descriptions
 *
 * This function takes a list of protocol description files and parses
 * them in to structures that can be accessed via the returned XGenState
 * pointer.
 *
 * This function returns NULL if there was a problem in parsing the files
 */
XGenState *
xgen_parse_xcb_proto_files (GList *files)
{
  XGenState  *state = g_new0 (XGenState, 1);
  unsigned long l = 1;
  GList *tmp;

  state->host_is_little_endian = *(unsigned char *)&l ? TRUE : FALSE;

  for (tmp = files; tmp != NULL; tmp = tmp->next)
    xgen_open_xcb_proto_file (state, tmp->data);

  if (!resolve_imports (state))
    return NULL;

  for (tmp = state->extensions; tmp != NULL; tmp = tmp->next)
    {
      XGenExtension *extension = tmp->data;
      xgen_parse_xcb_proto_and_imports (state, extension);
    }

  /* FIXME: Clean things up if there was an error resolving things! */

  return state;
}

