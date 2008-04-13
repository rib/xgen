/* Xamine - X Protocol Analyzer
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

#include <string.h>

typedef enum
{
    XAMINE_BOOLEAN,
    XAMINE_CHAR,
    XAMINE_SIGNED,
    XAMINE_UNSIGNED,
    XAMINE_STRUCT,
    XAMINE_UNION,
    XAMINE_TYPEDEF
} XamineType;

typedef enum
{
    XAMINE_REQUEST,
    XAMINE_RESPONSE
} XamineDirection;

typedef struct XamineDefinition
{
    char *name;
    XamineType type;
    union
    {
        unsigned int size;		/* base types */
        GList *fields;			/* struct, union */
        struct XamineDefinition *ref;   /* typedef */
    };
} XamineDefinition;

typedef enum XamineExpressionType
{
    XAMINE_FIELDREF,
    XAMINE_VALUE,
    XAMINE_OP
} XamineExpressionType;

typedef enum XamineOp
{
    XAMINE_ADD,
    XAMINE_SUBTRACT,
    XAMINE_MULTIPLY,
    XAMINE_DIVIDE,
    XAMINE_LEFT_SHIFT,
    XAMINE_BITWISE_AND
} XamineOp;

typedef struct XamineExpression
{
    XamineExpressionType type;
    union
    {
        char *field;         /* Field name for XAMINE_FIELDREF */
        unsigned long value; /* Value for XAMINE_VALUE */
        struct               /* Operator and operands for XAMINE_OP */
        {
            XamineOp op;
            struct XamineExpression *left;
            struct XamineExpression *right;
        };
    };
} XamineExpression;

typedef struct XamineFieldDefinition
{
    char *name;
    XamineDefinition *definition;
    XamineExpression *length;           /* List length; NULL for non-list */
//    struct XamineFieldDefinition *next;
} XamineFieldDefinition;

typedef struct XaminedItem
{
    char *name;
    XamineDefinition *definition;
    unsigned int offset;
    union
    {
        unsigned char bool_value;
        char          char_value;
        signed long   signed_value;
        unsigned long unsigned_value;
    };
    struct XaminedItem *child;
    struct XaminedItem *next;
} XaminedItem;


/* Concrete definitions for opaque and private structure types. */
typedef struct XamineRequest
{
    unsigned char opcode;
    XamineDefinition *definition;
} XamineRequest;

typedef struct XamineEvent
{
    unsigned char number;
    XamineDefinition *definition;
} XamineEvent;

typedef struct XamineError
{
    unsigned char number;
    XamineDefinition *definition;
} XamineError;

typedef struct XamineExtension
{
    char *name;
    char *xname;
    GList *requests;
    GList *events;
    GList *errors;
} XamineExtension;


typedef struct XamineState
{
    unsigned char host_is_le;
    GList *definitions;
    //XamineDefinition *core_events[64];  /* Core events 2-63 (0-1 unused) */
    //XamineDefinition *core_errors[128]; /* Core errors 0-127             */
    GList *extensions;
} XamineState;

static const XamineDefinition core_type_definitions[] =
{
    { "char",   XAMINE_CHAR,     1 },
    { "BOOL",   XAMINE_BOOLEAN,  1 },
    { "BYTE",   XAMINE_UNSIGNED, 1 },
    { "CARD8",  XAMINE_UNSIGNED, 1 },
    { "CARD16", XAMINE_UNSIGNED, 2 },
    { "CARD32", XAMINE_UNSIGNED, 4 },
    { "INT8",   XAMINE_SIGNED,   1 },
    { "INT16",  XAMINE_SIGNED,   2 },
    { "INT32",  XAMINE_SIGNED,   4 }
};

//static char* xamine_make_name(XamineExtension *extension, char *name);
static XamineDefinition *xamine_find_type(XamineState *state, char *name);
static xmlNode *xamine_xml_next_elem(xmlNode *elem);
static GList *xamine_parse_fields(XamineState *state,
                                  xmlNode *elem);
static XamineExpression *xamine_parse_expression(XamineState *state,
                                                 xmlNode *elem);

void xamine_cleanup(XamineState *state)
{
    XamineDefinition *temp;
    GList *tmp;
    for (tmp = state->definitions; tmp!=NULL; tmp=tmp->next)
    {
	g_free(tmp->data);
    }
    g_list_free(state->definitions);
    /* FIXME: incomplete */
}


/* Helper function to avoid casting. */
static char *
xamine_xml_get_prop(xmlNodePtr node, const char *name)
{
    return (char *)xmlGetProp(node, (xmlChar *)name);
}

/* Helper function to avoid casting. */
static char *
xamine_xml_get_node_name(xmlNodePtr node)
{
    return (char *)node->name;
}

/* Helper function to avoid casting. */
static char *
xamine_xml_get_node_content(xmlNodePtr node)
{
    return (char *)xmlNodeGetContent(node);
}

static void
xamine_parse_xmlxcb_file(XamineState *state, char *filename)
{
    xmlDoc  *doc;
    xmlNode *root, *elem;
    char *extension_xname;
    GList *tmp;
    XamineExtension *extension = NULL;
    
    /* FIXME: Remove this. */
    printf("DEBUG: Parsing file \"%s\"\n", filename);
    
    /* Ignore text nodes consisting entirely of whitespace. */
    xmlKeepBlanksDefault(0); 
    
    doc = xmlParseFile(filename);
    if(!doc)
        return;

    root = xmlDocGetRootElement(doc);
    if(!root)
        return;

    extension_xname = xamine_xml_get_prop(root, "extension-xname");
    if (!extension_xname)
	extension_xname = g_strdup("Core");

    printf("Extension: %s\n", extension_xname);

    for(tmp = state->extensions; tmp != NULL; tmp = tmp->next)
    {
	if(strcmp(((XamineExtension *)tmp->data)->xname,
		  extension_xname) == 0)
	{
	    extension = tmp->data;
	    break;
	}
    }
    
    if(extension == NULL)
    {
	extension = g_new0(XamineExtension, 1);
	extension->name = g_strdup(xamine_xml_get_prop(root,
						       "extension-name"));
	if (!extension->name && strcmp(extension_xname, "Core") == 0)
	    extension->name = g_strdup("Core");
	extension->xname = g_strdup(extension_xname);
	state->extensions = g_list_prepend(state->extensions, extension);
    }
    
    for(elem = root->children; elem != NULL;
        elem = xamine_xml_next_elem(elem->next))
    {
        /* FIXME: Remove this */
        {
            char *name = xamine_xml_get_prop(elem, "name");
            printf("DEBUG:    Parsing element \"%s\", name=\"%s\"\n",
                   xamine_xml_get_node_name(elem),
                   name ? name : "<not present>");
        }
        
        if(strcmp(xamine_xml_get_node_name(elem), "request") == 0)
        {
            XamineDefinition *def;
	    XamineFieldDefinition *field;
            GList *fields;
            int opcode = atoi(xamine_xml_get_prop(elem, "opcode"));

            def = g_new0(XamineDefinition, 1);
            //def->name = xamine_make_name(extension,
            //                             xamine_xml_get_prop(elem, "name"));
	    def->name = g_strdup(xamine_xml_get_prop(elem, "name"));
            def->type = XAMINE_STRUCT;

            fields = xamine_parse_fields(state, elem);
            if(!fields)
            {	
		field = g_new0(XamineFieldDefinition, 1);
                field->name = g_strdup("pad");
                field->definition = xamine_find_type(state, "CARD8");
		fields = g_list_prepend(fields, field);
            }

	    field = g_new0(XamineFieldDefinition, 1);
	    field->name = g_strdup("length");
	    field->definition = xamine_find_type(state, "CARD16");
	    fields = g_list_prepend(fields, field);
            
	    field = g_new0(XamineFieldDefinition, 1);
            field->name = g_strdup("opcode");
            field->definition = xamine_find_type(state, "BYTE");
	    fields = g_list_prepend(fields, field);
    
	    def->fields = fields;

	    state->definitions = g_list_prepend(state->definitions, def);

            if(extension)
            {
                XamineRequest *request = g_new0(XamineRequest, 1);
                request->opcode = opcode;
                request->definition = def;
		extension->requests = g_list_prepend (extension->requests, request);
            }
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "event") == 0)
        {
            char *no_sequence_number;
            XamineDefinition *def;
	    XamineFieldDefinition *field;
            GList *fields;
            int number = atoi(xamine_xml_get_prop(elem, "number"));

            def = g_new0(XamineDefinition, 1);
            //def->name = xamine_make_name(extension,
            //                             xamine_xml_get_prop(elem, "name"));
	    def->name = g_strdup(xamine_xml_get_prop(elem, "name"));
            def->type = XAMINE_STRUCT;
            
            fields = xamine_parse_fields(state, elem);
#if 0
            if(fields == NULL)
            {	
		field = g_new0(XamineFieldDefinition, 1);
                field->name = g_strdup("pad");
                field->definition = xamine_find_type(state, "CARD8");
		fields = g_list_prepend(fields, field);
            }
#endif
	    
            no_sequence_number = xamine_xml_get_prop(elem, "no-sequence-number");
	    if (!no_sequence_number)
            {
                field = g_new0(XamineFieldDefinition, 1);
                field->name = g_strdup("sequence");
                field->definition = xamine_find_type(state, "CARD16");
		fields = g_list_prepend(fields, field);
            }
            
	    field = g_new0(XamineFieldDefinition, 1);
            field->name = g_strdup("type");
            field->definition = xamine_find_type(state, "BYTE");
	    fields = g_list_prepend(fields, field);
    
	    def->fields = fields;

	    state->definitions = g_list_prepend(state->definitions, def);

            if(extension)
            {
                XamineEvent *event = g_new0(XamineEvent, 1);
                event->number = number;
                event->definition = def;
		extension->events = g_list_prepend (extension->events, event);
            }
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "eventcopy") == 0)
        {
            XamineDefinition *def;
            int number = atoi(xamine_xml_get_prop(elem, "number"));

            def = g_new0(XamineDefinition, 1);
            def->name = g_strdup(xamine_xml_get_prop(elem, "name"));
            def->type = XAMINE_TYPEDEF;
            def->ref = xamine_find_type(state,
                                        xamine_xml_get_prop(elem, "ref"));
            
            if(extension)
            {
                XamineEvent *event = g_new0(XamineEvent, 1);
                event->number = number;
                event->definition = def;
		extension->events = g_list_prepend (extension->events, event);
            }
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "error") == 0)
        {
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "errorcopy") == 0)
        {
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "struct") == 0)
        {
            XamineDefinition *def = g_new0(XamineDefinition, 1);
            //def->name = xamine_make_name(extension,
            //                             xamine_xml_get_prop(elem, "name"));
	    def->name = g_strdup(xamine_xml_get_prop(elem, "name"));
            def->type = XAMINE_STRUCT;
            def->fields = xamine_parse_fields(state, elem);
	    state->definitions = g_list_prepend(state->definitions, def);
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "xidunion") == 0
		|| strcmp(xamine_xml_get_node_name(elem), "union") == 0)
        {
            XamineDefinition *def = g_new0(XamineDefinition, 1);
	    def->name = g_strdup(xamine_xml_get_prop(elem, "name"));
            def->type = XAMINE_UNION;
            def->fields = xamine_parse_fields(state, elem);
	    state->definitions = g_list_prepend(state->definitions, def);
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "xidtype") == 0)
        {
            XamineDefinition *def = g_new0(XamineDefinition, 1);
            //def->name = xamine_make_name(extension,
            //                             xamine_xml_get_prop(elem, "name"));
	    def->name = g_strdup(xamine_xml_get_prop(elem, "name"));
            def->type = XAMINE_UNSIGNED;
            def->size = 4;
	    state->definitions = g_list_prepend(state->definitions, def);
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "enum") == 0)
        {
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "typedef") == 0)
        {
            XamineDefinition *def = g_new0(XamineDefinition, 1);
            //def->name = xamine_make_name(extension,
            //                             xamine_xml_get_prop(elem, "newname"));
	    def->name = g_strdup(xamine_xml_get_prop(elem, "newname"));
            def->type = XAMINE_TYPEDEF;
            def->ref = xamine_find_type(state,
                                        xamine_xml_get_prop(elem, "oldname"));
	    state->definitions = g_list_prepend(state->definitions, def);
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "import") == 0)
        {
        }
    }
}

#if 0
static char* xamine_make_name(XamineExtension *extension, char *name)
{
    if(extension)
    {
        char *temp = malloc(strlen(extension->name) + strlen(name) + 1);
        if(temp == NULL)
            return NULL;
        strcpy(temp, extension->name);
        strcat(temp, name);
        return temp;
    }
    else
        return strdup(name);
}
#endif

static XamineDefinition *xamine_find_type(XamineState *state, char *name)
{
    GList *tmp;
    for(tmp = state->definitions; tmp != NULL; tmp=tmp->next)
    {
	XamineDefinition *def = tmp->data;

        /* FIXME: does not work for extension types. */
	if(strcmp(def->name, name) == 0)
            return def;
    }
    return NULL;
}

static xmlNode *xamine_xml_next_elem(xmlNode *elem)
{
    while(elem && elem->type != XML_ELEMENT_NODE)
        elem = elem->next;
    return elem;
}

static GList *xamine_parse_fields(XamineState *state,
                                  xmlNode *elem)
{
    xmlNode *cur;
    //XamineFieldDefinition *head;
    //XamineFieldDefinition **tail = &head;
    GList *fields = NULL;

    for(cur = elem->children; cur!=NULL; cur = xamine_xml_next_elem(cur->next))
    {
	XamineFieldDefinition *field;
        /* FIXME: handle elements other than "field", "pad", and "list". */
        //*tail = g_new0(XamineFieldDefinition, 1);
        field = g_new0(XamineFieldDefinition, 1);
        if(strcmp(xamine_xml_get_node_name(cur), "pad") == 0)
        {
            field->name = "pad";
            field->definition = xamine_find_type(state, "CARD8");
            field->length = g_new0(XamineExpression, 1);
            field->length->type = XAMINE_VALUE;
            field->length->value = atoi(xamine_xml_get_prop(cur, "bytes"));
        }
        else if(strcmp(xamine_xml_get_node_name(cur), "field") == 0)
        {
            field->name = strdup(xamine_xml_get_prop(cur, "name"));
            field->definition = xamine_find_type(state,
                                      xamine_xml_get_prop(cur, "type"));
            /* FIXME: handle missing length expressions. */
            if(strcmp(xamine_xml_get_node_name(cur), "list") == 0)
                field->length = xamine_parse_expression(state,
                                                          cur->children);
        }
	else if(strcmp(xamine_xml_get_node_name(cur), "valueparam") == 0)
	{
            field->name = strdup(xamine_xml_get_prop(cur,
						       "value-mask-name"));
            field->definition = xamine_find_type(state,
                                      xamine_xml_get_prop(cur, "value-mask-type"));
	    fields = g_list_prepend(fields, field);

	    field = g_new0(XamineFieldDefinition, 1);
            field->name = strdup(xamine_xml_get_prop(cur,
						       "value-list-name"));
            field->definition = xamine_find_type(state,
                                      xamine_xml_get_prop(cur, "value-mask-type"));
	}
	else
	    continue;
        //tail = &((*tail)->next);
	fields = g_list_prepend(fields, field);
    }
    
    fields = g_list_reverse (fields);
    //*tail = NULL;
    return fields;
}

static XamineExpression *xamine_parse_expression(XamineState *state,
                                                 xmlNode *elem)
{
    XamineExpression *e = g_new0(XamineExpression, 1);
    elem = xamine_xml_next_elem(elem);
    if(strcmp(xamine_xml_get_node_name(elem), "op") == 0)
    {
        char *temp = xamine_xml_get_prop(elem, "op");
        e->type = XAMINE_OP;
        if(strcmp(temp, "+") == 0)
            e->op = XAMINE_ADD;
        else if(strcmp(temp, "-") == 0)
            e->op = XAMINE_SUBTRACT;
        else if(strcmp(temp, "*") == 0)
            e->op = XAMINE_MULTIPLY;
        else if(strcmp(temp, "/") == 0)
            e->op = XAMINE_DIVIDE;
        else if(strcmp(temp, "<<") == 0)
            e->op = XAMINE_LEFT_SHIFT;
        else if(strcmp(temp, "&") == 0)
            e->op = XAMINE_BITWISE_AND;
        elem = xamine_xml_next_elem(elem->children);
        e->left = xamine_parse_expression(state, elem);
        elem = xamine_xml_next_elem(elem->next);
        e->right = xamine_parse_expression(state, elem);
    }
    else if(strcmp(xamine_xml_get_node_name(elem), "value") == 0)
    {
        e->type = XAMINE_VALUE;
        e->value = strtol(xamine_xml_get_node_content(elem), NULL, 0);
    }
    else if(strcmp(xamine_xml_get_node_name(elem), "fieldref") == 0)
    {
        e->type = XAMINE_FIELDREF;
        e->field = strdup(xamine_xml_get_node_content(elem));
    }
    return e;
}

static long xamine_evaluate_expression(XamineExpression *expression,
                                       XaminedItem *parent)
{
    switch(expression->type)
    {
    case XAMINE_VALUE:
        return expression->value;
        
    case XAMINE_FIELDREF:
    {
        XaminedItem *cur;
        for(cur = parent->child; cur != NULL; cur = cur->next)
            if(strcmp(cur->name, expression->field) == 0)
                switch(cur->definition->type)
                {
                case XAMINE_BOOLEAN: return cur->bool_value;
                case XAMINE_CHAR: return cur->char_value;
                case XAMINE_SIGNED: return cur->signed_value;
                case XAMINE_UNSIGNED: return cur->unsigned_value;
                }
        /* FIXME: handle not found or wrong type */
    }

    case XAMINE_OP:
    {
        long left  = xamine_evaluate_expression(expression->left, parent);
        long right = xamine_evaluate_expression(expression->right, parent);
        switch(expression->op)
        {
        case XAMINE_ADD:         return left+right;
        case XAMINE_SUBTRACT:    return left-right;
        case XAMINE_MULTIPLY:    return left*right;
        case XAMINE_DIVIDE:      return left/right; /* FIXME: divide by zero */
        case XAMINE_LEFT_SHIFT:  return left<<right;
        case XAMINE_BITWISE_AND: return left&right;
        }
    }
    }
}

static char *
normalise_name(const char *name)
{
    gint pos;
    GString *new_name;
    gchar *tmp, *ret;

    g_return_val_if_fail(name && name[0], NULL);
    new_name = g_string_new(name);

    for (pos = 0; new_name->str[pos+1]; pos++)
    {
	if (!new_name->str[pos] || !new_name->str[pos+1])
	    break;
	
	if (g_ascii_islower(new_name->str[pos])
	    && g_ascii_isupper(new_name->str[pos+1]))
	{
	    new_name = g_string_insert_c(new_name, pos+1, '_');
	    pos++;
	}
    }

    tmp = g_string_free(new_name, FALSE);
    ret = g_ascii_strdown(tmp, -1);
    g_free(tmp);

    return ret;
}

static char *
name_to_gx_name(char *name)
{
    char *normalised_name = normalise_name(name);
    char *ret = g_strdup_printf("gx_%s", normalised_name);
    g_free(normalised_name);
    return ret;
}

static char *
name_to_xcb_name(char *name)
{
    char *normalised_name = normalise_name(name);
    char *ret = g_strdup_printf("xcb_%s", normalised_name);
    g_free(normalised_name);
    return ret;
}

struct type_mapping {
    char *from;
    char *to;
};

static const char *
definition_to_gx_type(XamineDefinition *definition)
{
    static struct type_mapping typemap[] = {
	{ "WINDOW", "GXWindow" },
	{ "CARD8", "guint8" },
	{ "CARD16", "guint16" },
	{ "CARD32", "guint32" },
	{ "INT8", "gint8" },
	{ "INT16", "gint16" },
	{ "INT32", "gint32" },
	{ "BOOL", "gboolean" },
	NULL
    };
    struct type_mapping *mapping;
    
    for (mapping = typemap; mapping->from != NULL; mapping++)
    {
	if(strcmp(mapping->from, definition->name) == 0)
	    return mapping->to;
    }
    return "FIXME";
}

static void
gen_gx_h_code(XamineState *state, XamineExtension *extension, FILE *header)
{
    GList *tmp;

    for(tmp = extension->requests; tmp != NULL; tmp = tmp->next)
    {
	XamineRequest *request = tmp->data;
	GList *tmp2;
	char *gx_name;
	
	gx_name = name_to_gx_name(request->definition->name);
	fprintf(header, "GXCookie *\n%s_async(GXDisplay *display", gx_name);
	g_free(gx_name);

	/* FIXME - debug */
	fflush(header);
	for(tmp2 = request->definition->fields; tmp2 != NULL; tmp2 = tmp2->next)
	{
	    XamineFieldDefinition *field = tmp2->data;
	    
	    if(strcmp(field->name, "opcode") != 0
	       && strcmp(field->name, "pad") != 0
	       && strcmp(field->name, "length") != 0)
	    {
		const char *type = definition_to_gx_type(field->definition);
		fprintf(header, ",\n\t\t%s %s", type, field->name);
	    }
	}
	fprintf(header, ");\n\n");

	gx_name = name_to_gx_name(request->definition->name);
	fprintf(header, "FIXME\n%s(GXDisplay *display", gx_name);
	g_free(gx_name);

	/* FIXME - debug */
	fflush(header);
	for(tmp2 = request->definition->fields; tmp2 != NULL; tmp2 = tmp2->next)
	{
	    XamineFieldDefinition *field = tmp2->data;
	    
	    if(strcmp(field->name, "opcode") != 0
	       && strcmp(field->name, "pad") != 0
	       && strcmp(field->name, "length") != 0)
	    {
		const char *type = definition_to_gx_type(field->definition);
		fprintf(header, ",\n\t\t%s %s", type, field->name);
	    }
	}
	fprintf(header, ");\n\n");

    }
}

static void
gen_gx_c_code(XamineState *state, XamineExtension *extension, FILE *code)
{
}

static void
gen_gx_code(XamineState *state, XamineExtension *extension)
{
    char *h_name = g_strdup_printf("%s.h",extension->name);
    char *c_name = g_strdup_printf("%s.c",extension->name);
    FILE *header;
    FILE *code;

    header = fopen(h_name, "w");
    if(!header)
    {
	perror("Failed to open header file");
	return;
    }
    code = fopen(c_name, "w");
    if(!code)
    {
	perror("Failed to open code file");
	return;
    }
    
    gen_gx_h_code(state, extension, header);
    gen_gx_c_code(state, extension, code);

    fclose(header);
    fclose(code);
}

int
main(int argc, char **argv)
{
    int i;
    XamineState *state = g_new0(XamineState, 1);
    GList *tmp;

    {
        unsigned long l = 1;
        state->host_is_le = *(unsigned char*)&l;
    }

    /* Add definitions of core types. */
    for(i = 0; i < sizeof(core_type_definitions)/sizeof(XamineDefinition); i++)
    {
	XamineDefinition *temp = g_new0(XamineDefinition, 1);

        *temp = core_type_definitions[i];
        temp->name = g_strdup(core_type_definitions[i].name);
        
	state->definitions = g_list_prepend(state->definitions, temp);
    }

    xamine_parse_xmlxcb_file(state, "./xproto.xml");
    //xamine_parse_xmlxcb_file(state, "./composite.xml");

    for (tmp = state->extensions; tmp != NULL; tmp = tmp->next)
    {
	XamineExtension *extension = tmp->data;

	gen_gx_code(state, extension);
    }
}

