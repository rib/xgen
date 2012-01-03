#ifndef _XGEN_H_
#define _XGEN_H_

#include <libxml/parser.h>

#include <glib.h>

typedef enum _XGenType
{
  /* Base types */
  XGEN_VOID,
  XGEN_BOOLEAN,
  XGEN_CHAR,
  XGEN_SIGNED,
  XGEN_UNSIGNED,
  XGEN_XID,
  XGEN_FLOAT,
  XGEN_DOUBLE,

  /* Basic composite types */
  XGEN_STRUCT,
  XGEN_UNION,
  XGEN_XIDUNION,
  XGEN_ENUM,

  /* Type definitions */
  XGEN_TYPEDEF,

  /* Protocol data types */
  XGEN_REQUEST,
  XGEN_VALUEPARAM,
  XGEN_REPLY,
  XGEN_EVENT,
  XGEN_ERROR
} XGenType;

typedef struct _XGenExtension
{
  char	*name;
  char  *header;

  GList *imports;

  GList *base_types;
  GList *structs;
  GList *unions;
  GList *xid_unions;
  GList *enums;
  GList *typedefs;
  GList *requests;
  GList *replys;
  GList *errors;
  GList *events;

  GList *all_definitions;

  /* Private */

  xmlDoc *_xml_doc;
  GList *_import_headers;
  gboolean _parsed;

} XGenExtension;

typedef struct _XGenDefinition
{
  const XGenExtension *extension;
  XGenType	       type;
  char		      *name;

  void		      *_private; /* application private data */
} XGenDefinition;

/**
 * Casts a specific definition into a generic definition
 */
#define XGEN_DEF(DEF) ((XGenDefinition *)(DEF))

/**
 * Describes a base type such as CARD8
 */
typedef struct _XGenBaseType
{
  XGenDefinition   _parent;

  unsigned int	   size;
} XGenBaseType;
/**
 * Casts a generic definition into a base type definition
 */
#define XGEN_BASE_TYPE_DEF(DEF) ((XGenBaseType *)(DEF))

/**
 * Describes a basic struct
 */
typedef struct _XGenStruct
{
  XGenDefinition   _parent;

  GList		  *fields;
} XGenStruct;
/**
 * Casts a generic definition into a struct definition
 */
#define XGEN_STRUCT_DEF(DEF) ((XGenStruct *)(DEF))

/**
 * Describes a basic union
 */
typedef struct _XGenUnion
{
  XGenDefinition   _parent;

  GList		  *fields;
} XGenUnion;
/**
 * Casts a generic definition into a union definition
 */
#define XGEN_UNION_DEF(DEF) ((XGenUnion *)(DEF))

/**
 * Describes a basic enum
 */
typedef struct _XGenEnum
{
  XGenDefinition   _parent;

  GList		  *items;
} XGenEnum;
/**
 * Casts a generic definition into an enum definition
 */
#define XGEN_ENUM_DEF(DEF) ((XGenEnum *)(DEF))

/**
 * Describes an XID union
 */
typedef struct _XGenXIDUnion
{
  XGenDefinition   _parent;

  GList		  *fields;
} XGenXIDUnion;
/**
 * Casts a generic definition into an XID union definition
 */
#define XGEN_XID_UNION_DEF(DEF) ((XGenXIDUnion *)(DEF))

/**
 * Describes an simple typedef
 */
typedef struct _XGenTypedef
{
  XGenDefinition   _parent;

  XGenDefinition  *reference;
} XGenTypedef;
/**
 * Casts a generic definition into a typedef definition
 */
#define XGEN_TYPEDEF_DEF(DEF) ((XGenTypedef *)(DEF))

/**
 * Describes a value param
 */
typedef struct _XGenValueParam
{
  XGenDefinition   _parent;

  XGenDefinition  *reference; /* TODO: rename as 'type' */
  gchar		  *mask_name;
  gchar		  *list_name;
} XGenValueParam;
/**
 * Casts a generic definition into a value param definition
 */
#define XGEN_VALUE_PARAM_DEF(DEF) ((XGenValueParam *)(DEF))

/**
 * Describes an X reply
 */
typedef struct
{
  XGenDefinition   _parent;

  unsigned char	   opcode;
  GList		  *fields;
} XGenReply;
/**
 * Casts a generic definition into a reply definition
 */
#define XGEN_REPLYDEF(DEF) ((XGenReply *)(DEF))

/**
 * Describes an X request
 */
typedef struct _XGenRequest
{
  XGenDefinition   _parent;

  unsigned char	   opcode;
  GList		  *fields;
  XGenReply	  *reply;
} XGenRequest;
/**
 * Casts a generic definition into a request definition
 */
#define XGEN_REQUEST_DEF(DEF) ((XGenRequest *)(DEF))

/**
 * Describes an X event
 */
typedef struct _XGenEvent
{
  XGenDefinition   _parent;

  unsigned char	   number;
  GList		  *fields;
  gboolean	   is_copy; /* If true then the fields are owned by another
			       XGenEvent */
} XGenEvent;
/**
 * Casts a generic definition into an event definition
 */
#define XGEN_EVENT_DEF(DEF) ((XGenEvent *)(DEF))

/**
 * Describes an X error
 */
typedef struct _XGenError
{
  XGenDefinition   _parent;

  unsigned char	   number;
  GList *fields;
  gboolean	   is_copy; /* If true then the fields are owned by another
			       XGenError */
} XGenError;
/**
 * Casts a generic definition into an error definition
 */
#define XGEN_ERROR_DEF(DEF) ((XGenError *)(DEF))


typedef enum _XGenExpressionType
{
  XGEN_FIELDREF,
  XGEN_VALUE,
  XGEN_OP
} XGenExpressionType;

typedef enum _XGenOp
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

typedef struct _XGenFieldDefinition
{
  char *name;
  XGenDefinition *definition;
  XGenExpression *length;      /* List length. NULL for non-list */
} XGenFieldDefinition;

typedef struct _XGenFieldValue
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

typedef enum _XGenItemType
{
  XGEN_ITEM_AS_VALUE = 1,
  XGEN_ITEM_AS_BIT,
} XGenItemType;

typedef struct _XGenItemDefinition
{
  XGenItemType	 type;
  char		*name;
  char		*value;
  guint		 bit;
} XGenItemDefinition;

typedef struct _XGenState
{
  gboolean   host_is_little_endian;
  GList	    *extensions;
} XGenState;


typedef struct _XGenEventHandlers
{
  void (*definition_notify) (XGenDefinition *definition);
  void (*base_notify) (XGenBaseType *base_type);
  void (*request_notify) (XGenRequest *request);
  void (*reply_notify) (XGenReply *reply);
  void (*error_notify) (XGenError *error);
  void (*event_notify) (XGenEvent *event);
  void (*struct_notify) (XGenStruct *struct_def);
  void (*xid_union_notify) (XGenXIDUnion *xid_union);
  void (*union_notify) (XGenUnion *union_def);
  void (*enum_notify) (XGenEnum *enum_def);
  void (*typedef_notify) (XGenTypedef *typedef_def);
  void (*valueparam_notify) (XGenValueParam *valueparam);
} XGenEventHandlers;


void xgen_set_handlers (XGenEventHandlers *handlers);
XGenState *xgen_parse_xcb_proto_files (GList *files);

void *xgen_definition_get_private (const XGenDefinition *def);
void xgen_definition_set_private (XGenDefinition *def, void *data);

#endif /* _XGEN_H_ */
