#ifndef _gfproto2_h_
#define _gfproto2_h_

#include <stdint.h>

#define GF_MAX_NAME_LEN 32


#ifndef bool
#define bool char
#ifndef true
#define true 1
#endif
#ifndef false
#define false 0
#endif
#endif

struct GF_Proto_Node;

typedef int64_t GF_PROTO_INT;
typedef double GF_PROTO_FLOAT;
typedef struct GF_Proto_Node** GF_PROTO_ARRAY;
typedef bool GF_PROTO_BOOL;
typedef char* GF_PROTO_STRING;
typedef struct GF_Proto_Message* GF_PROTO_MESSAGE;


typedef enum
{
	GF_LITTLE_ENDIAN = 0,
	GF_BIG_ENDIAN,
}GF_PROTO_BYTE_ORDER;

typedef enum
{
	GF_PROTO_TYPE_INT = 0,
	GF_PROTO_TYPE_FLOAT,
	GF_PROTO_TYPE_ARRAY,
	GF_PROTO_TYPE_BOOL,
	GF_PROTO_TYPE_STRING,
	GF_PROTO_TYPE_MESSAGE,
}GF_PROTO_TYPE;

typedef enum
{
	GF_PROTO_INT1 = 1,
	GF_PROTO_INT2,
	GF_PROTO_INT3,
	GF_PROTO_INT4,
	GF_PROTO_INT5,
	GF_PROTO_INT6,
	GF_PROTO_INT7,
	GF_PROTO_INT8,
}GF_PROTO_NUMBER_TYPE;

typedef enum
{
	GF_TALBLE_NONE = 0,
	GF_TALBLE_EXIST,
}GF_PROTO_TALBLE_EXIST;

typedef union GF_Proto_Value
{
	GF_PROTO_INT     _ivalue;
	GF_PROTO_FLOAT   _fvalue;
	GF_PROTO_ARRAY   _avalue;
	GF_PROTO_BOOL    _bvalue;
	GF_PROTO_STRING  _svalue;
	GF_PROTO_MESSAGE _mvalue;
}GF_Proto_Value;

typedef struct GF_Proto_Node
{
	int16_t _len; //string, array length
	char _name[GF_MAX_NAME_LEN];
	GF_PROTO_TYPE _type;
	GF_Proto_Value _value;
	
}GF_Proto_Node;

typedef struct GF_Proto_Array_Node
{
	int16_t _len; //string, array length
	char _name[GF_MAX_NAME_LEN];
	GF_PROTO_TYPE _type;
	GF_Proto_Value _value;

	GF_PROTO_TYPE _nodetype;
	char _nodetypename[GF_MAX_NAME_LEN];
}GF_Proto_Array_Node;


typedef struct GF_Proto_Message
{
	int _len; 
	char _name[GF_MAX_NAME_LEN];
	GF_Proto_Array_Node **_nodes;
}GF_Proto_Message;

typedef struct GF_Proto_Message_Array
{
	int _msg_def_len;
	const GF_Proto_Message **_msg_defs;
}GF_Proto_Message_Array;


#define GFMalloc malloc
#define GFRealloc realloc
#define GFFree   free

#define GF_PROTO_ERROR(xx) printf("[GF_PROTO_ERROR] %s %s %d, %s\n", __FILE__, __FUNCTION__, __LINE__, xx)
#define GF_PROTO_INFO(xx) printf("[GF_PROTO_INFO] %s %s %d, %s\n", __FILE__, __FUNCTION__, __LINE__, xx)

extern void gf_proto_load(const char **filelist, int n);
extern void gf_proto_unload();

extern GF_Proto_Node *gf_proto_new_node(GF_PROTO_TYPE t, const char *msgname);
extern GF_Proto_Message *gf_proto_new_msg(const char *msgname);
extern void gf_proto_destroy_node(GF_Proto_Node *node);
extern void gf_proto_destroy_msg(GF_Proto_Message *msg);

extern void *gf_proto_get(GF_Proto_Node *node, int *len);
extern bool gf_proto_set(GF_Proto_Node *node, void *value, int len);

extern void *gf_proto_get_filed(GF_Proto_Message *msg, const char *key);
extern bool gf_proto_set_filed(GF_Proto_Message *msg, const char *key, void *value, int len);

extern bool gf_proto_push(GF_Proto_Array_Node *arr, GF_Proto_Node *node);
extern bool gf_proto_pop(GF_Proto_Array_Node *arr);
extern bool gf_proto_erase(GF_Proto_Array_Node *arr, int idx);

extern char *gf_proto_encode(GF_Proto_Message *msg, int *len);
extern GF_Proto_Message *gf_proto_decode(const char * str);

/****************************** public interfaces ******************************/

#define gfp_msg GF_Proto_Message
#define gfp_node GF_Proto_Node
#define gfp_array_node GF_Proto_Array_Node

#define gfp_new_node gf_proto_new_node
#define gfp_new_msg gf_proto_new_msg
#define gfp_dele_node gf_proto_destroy_node
#define gfp_dele_msg gf_proto_destroy_msg
#define gfp_get gf_proto_get

#define gfp_new_int()         gfp_new_node(GF_PROTO_TYPE_INT, 0)
#define gfp_new_float()       gfp_new_node(GF_PROTO_TYPE_FLOAT, 0)
//#define gfp_new_array()       gfp_new_node(GF_PROTO_TYPE_INT, 0)
#define gfp_new_bool()        gfp_new_node(GF_PROTO_TYPE_BOOL, 0)
#define gfp_new_string()      gfp_new_node(GF_PROTO_TYPE_STRING, 0)
#define gfp_new_mes_node(name) gfp_new_node(GF_PROTO_TYPE_MESSAGE, name)


#define gfp_set_int(node, value) { if(node->_type != GF_PROTO_TYPE_INT){GF_PROTO_ERROR("wrong type: not int!");}else{GF_PROTO_INT v = (value); gf_proto_set((node), &v, (0));} }
#define gfp_set_float(node, value)  { if(node->_type != GF_PROTO_TYPE_FLOAT){GF_PROTO_ERROR("wrong type: not int!");}else{ GF_PROTO_FLOAT v = (value); gf_proto_set((node), &v, (0));} }
//#define gfp_set_array(node, value, len)  { if(node->_type != GF_PROTO_TYPE_ARRAY){GF_PROTO_ERROR("wrong type: not int!");}else{ GF_PROTO_TYPE_ARRAY v = (value); gf_proto_set((node), &v, (len));} }
#define gfp_set_bool(node, value)  { if((node)->_type != GF_PROTO_TYPE_BOOL){GF_PROTO_ERROR("wrong type: not int!");}else{ GF_PROTO_BOOL v = (value); gf_proto_set((node), &v, (0));} }
#define gfp_set_string(node, value, len)  { if(node->_type != GF_PROTO_TYPE_STRING){GF_PROTO_ERROR("wrong type: not int!");}else{ GF_PROTO_STRING v = (value); gf_proto_set((node), &v, (len));} }
#define gfp_set_msg(node, value)  { if(node->_type != GF_PROTO_TYPE_MESSAGE){GF_PROTO_ERROR("wrong type: not int!");}else{ GF_PROTO_MESSAGE v = (value); gf_proto_set((node), &v, (0));} }

#define gfp_get_field gf_proto_get_filed
#define gfp_set_field gf_proto_set_filed

#define gfp_int(node)    (GF_PROTO_INT)(((GF_Proto_Value*)gfp_get(node, 0))->_ivalue)
#define gfp_float(node)  (GF_PROTO_FLOAT)(((GF_Proto_Value*)gfp_get(node, 0))->_fvalue)
#define gfp_array(node)  (GF_PROTO_ARRAY)(((GF_Proto_Value*)gfp_get(node, 0))->_avalue)
#define gfp_bool(node)  (GF_PROTO_BOOL)(((GF_Proto_Value*)gfp_get(node, 0))->_bvalue)
#define gfp_string(node, len)  (GF_PROTO_STRING)(((GF_Proto_Value*)gfp_get(node, len))->_svalue)
#define gfp_message(node)  (GF_PROTO_MESSAGE)(node->_value._avalue)

#define gfp_push gf_proto_push
#define gfp_pop gf_proto_pop
#define gfp_erase gf_proto_erase

#define gfp_encode gf_proto_encode
#define gfp_decode gf_proto_decode

/****************************** interfaces ******************************/


#endif // _gfproto2_h_
