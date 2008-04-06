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
        unsigned int size;                    /* base types */
        struct XamineFieldDefinition *fields; /* struct, union */
        struct XamineDefinition *ref;         /* typedef */
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
    struct XamineFieldDefinition *next;
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
typedef struct XamineEvent
{
    unsigned char number;
    XamineDefinition *definition;
    struct XamineEvent *next;
} XamineEvent;

typedef struct XamineError
{
    unsigned char number;
    XamineDefinition *definition;
    struct XamineError *next;
} XamineError;

typedef struct XamineExtension
{
    char *name;
    char *xname;
    XamineEvent *events;
    XamineError *errors;
    struct XamineExtension *next;
} XamineExtension;


typedef struct XamineState
{
    unsigned char host_is_le;
    GList *definitions;
    XamineDefinition *core_events[64];  /* Core events 2-63 (0-1 unused) */
    XamineDefinition *core_errors[128]; /* Core errors 0-127             */
    XamineExtension  *extensions;
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

static char* xamine_make_name(XamineExtension *extension, char *name);
static XamineDefinition *xamine_find_type(XamineState *state, char *name);
static xmlNode *xamine_xml_next_elem(xmlNode *elem);
static XamineFieldDefinition *xamine_parse_fields(XamineState *state,
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

    if(extension_xname)
    {
        /* FIXME: Remove this. */
        printf("Extension: %s\n", extension_xname);

        for(extension = state->extensions;
            extension != NULL;
            extension = extension->next)
        {
            if(strcmp(extension->xname, extension_xname) == 0)
                break;
        }
        
        if(extension == NULL)
        {
            extension = g_new0(XamineExtension, 1);
            extension->name = g_strdup(xamine_xml_get_prop(root,
                                                         "extension-name"));
            extension->xname = g_strdup(extension_xname);
            extension->next = state->extensions;
            state->extensions = extension;
        }
    }
    else                           /* FIXME: Remove this. */
        printf("Core Protocol\n");
    
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
            /* Not yet implemented. */
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "event") == 0)
        {
            char *no_sequence_number;
            XamineDefinition *def;
            XamineFieldDefinition *fields;
            int number = atoi(xamine_xml_get_prop(elem, "number"));
            if(number > 64)
                continue;
            def = g_new0(XamineDefinition, 1);
            def->name = xamine_make_name(extension,
                                         xamine_xml_get_prop(elem, "name"));
            def->type = XAMINE_STRUCT;
            
            fields = xamine_parse_fields(state, elem);
            if(fields == NULL)
            {
                fields = g_new0(XamineFieldDefinition, 1);
                fields->name = g_strdup("pad");
                fields->definition = xamine_find_type(state, "CARD8");
            }
            def->fields = g_new0(XamineFieldDefinition, 1);
            def->fields->name = g_strdup("response_type");
            def->fields->definition = xamine_find_type(state, "BYTE");
            def->fields->next = fields;
            fields = fields->next;
            no_sequence_number = xamine_xml_get_prop(elem, "no-sequence-number");
            if(no_sequence_number && strcmp(no_sequence_number, "true") == 0)
            {
                def->fields->next->next = fields;
            }
            else
            {
                def->fields->next->next = g_new0(XamineFieldDefinition, 1);
                def->fields->next->next->name = g_strdup("sequence");
                def->fields->next->next->definition =
                    xamine_find_type(state, "CARD16");
                def->fields->next->next->next = fields;
            }
	    state->definitions = g_list_prepend(state->definitions, def);
            
            if(extension)
            {
                XamineEvent *event = g_new0(XamineEvent, 1);
                event->number = number;
                event->definition = def;
                event->next = extension->events;
            }
            else
                state->core_events[number] = def;
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "eventcopy") == 0)
        {
            XamineDefinition *def;
            int number = atoi(xamine_xml_get_prop(elem, "number"));
            if(number > 64)
                continue;
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
                event->next = extension->events;
            }
            else
                state->core_events[number] = def;
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
            def->name = xamine_make_name(extension,
                                         xamine_xml_get_prop(elem, "name"));
            def->type = XAMINE_STRUCT;
            def->fields = xamine_parse_fields(state, elem);
	    state->definitions = g_list_prepend(state->definitions, def);
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "union") == 0)
        {
        }
        else if(strcmp(xamine_xml_get_node_name(elem), "xidtype") == 0)
        {
            XamineDefinition *def = g_new0(XamineDefinition, 1);
            def->name = xamine_make_name(extension,
                                         xamine_xml_get_prop(elem, "name"));
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
            def->name = xamine_make_name(extension,
                                         xamine_xml_get_prop(elem, "newname"));
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

static XamineFieldDefinition *xamine_parse_fields(XamineState *state,
                                                  xmlNode *elem)
{
    xmlNode *cur;
    XamineFieldDefinition *head;
    XamineFieldDefinition **tail = &head;
    for(cur = elem->children; cur!=NULL; cur = xamine_xml_next_elem(cur->next))
    {
        /* FIXME: handle elements other than "field", "pad", and "list". */
        *tail = g_new0(XamineFieldDefinition, 1);
        if(strcmp(xamine_xml_get_node_name(cur), "pad") == 0)
        {
            (*tail)->name = "pad";
            (*tail)->definition = xamine_find_type(state, "CARD8");
            (*tail)->length = g_new0(XamineExpression, 1);
            (*tail)->length->type = XAMINE_VALUE;
            (*tail)->length->value = atoi(xamine_xml_get_prop(cur, "bytes"));
        }
        else
        {
            (*tail)->name = strdup(xamine_xml_get_prop(cur, "name"));
            (*tail)->definition = xamine_find_type(state,
                                      xamine_xml_get_prop(cur, "type"));
            /* FIXME: handle missing length expressions. */
            if(strcmp(xamine_xml_get_node_name(cur), "list") == 0)
                (*tail)->length = xamine_parse_expression(state,
                                                          cur->children);
        }
        tail = &((*tail)->next);
    }
    
    *tail = NULL;
    return head;
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


int
main(int argc, char **argv)
{
    int i;
    XamineState *state = g_new0(XamineState, 1);

    {
        unsigned long l = 1;
        state->host_is_le = *(unsigned char*)&l;
    }

    /* Add definitions of core types. */
    for(i = 0; i < sizeof(core_type_definitions)/sizeof(XamineDefinition); i++)
    {
	XamineDefinition *temp = g_new0(XamineDefinition, 1);

        *temp = core_type_definitions[i];
        
        //temp->next = state->definitions;
        //state->definitions = temp;
	state->definitions = g_list_prepend(state->definitions, temp);
        
        temp->name = g_strdup(core_type_definitions[i].name);
    }

    xamine_parse_xmlxcb_file(state, "./xproto.xml");
    xamine_parse_xmlxcb_file(state, "./composite.xml");

}

