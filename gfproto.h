#ifndef _gfproto_h_
#define _gfproto_h_

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

#define GFPMalloc malloc
#define GFPRealloc realloc
#define GFPFree   free

#define GFP_ERROR(xx) printf("[GF_PROTO_ERROR] %s %s %d, %s\n", __FILE__, __FUNCTION__, __LINE__, xx)
#define GFP_INFO(xx) printf("[GF_PROTO_INFO] %s %s %d, %s\n", __FILE__, __FUNCTION__, __LINE__, xx)


struct GF_Proto_Node;
struct GF_Proto_String;
struct GF_Proto_Array;

typedef int64_t GF_Proto_Int;
typedef float GF_Proto_Float; 
typedef double GF_Proto_Double;
typedef struct GF_Proto_Array GF_Proto_Array;
typedef bool GF_Proto_Bool;
typedef struct GF_Proto_String GF_Proto_String;
typedef struct GF_Proto_Node* GF_Proto_Msg;


typedef enum
{
	GF_LITTLE_ENDIAN = 0,
	GF_BIG_ENDIAN,
}GF_PROTO_BYTE_ORDER;

typedef enum
{
	GF_PROTO_TYPE_INT = 0,
	GF_PROTO_TYPE_FLOAT,
	GF_PROTO_TYPE_DOUBLE,
	GF_PROTO_TYPE_ARRAY,
	GF_PROTO_TYPE_BOOL,
	GF_PROTO_TYPE_STRING,
	GF_PROTO_TYPE_MSG,
}GF_PROTO_TYPE;

typedef struct GF_Proto_String
{
	int _len;
	char *_str;
}GF_Proto_String;

typedef struct GF_Proto_Array
{
	GF_PROTO_TYPE _type;
	char _name[GF_MAX_NAME_LEN];
	int _len;
	struct GF_Proto_Node **_nodes;
}GF_Proto_Array;

typedef union GF_Proto_Value
{
	GF_Proto_Int     _ivalue;
	GF_Proto_Float   _fvalue;
	GF_Proto_Double  _dvalue;
	GF_Proto_Array   _avalue;
	GF_Proto_Bool    _bvalue;
	GF_Proto_String  _svalue;
	GF_Proto_Msg     _mvalue;
}GF_Proto_Value;

typedef struct GF_Proto_Node
{
	GF_PROTO_TYPE _type;
	char _name[GF_MAX_NAME_LEN];
	GF_Proto_Value _value;
	int16_t _len; //string, array length
}GF_Proto_Node;

typedef struct GF_Proto_Array GF_Proto_Msgdef;


#if 0

extern void gf_proto_load(const char **filelist, int n);
extern void gf_proto_unload();

extern GF_Proto_Node *gf_proto_new_node(GF_PROTO_TYPE t, const char *msgname);
extern GF_Proto_Message *gf_proto_new_msg(const char *msgname);
extern void gf_proto_destroy_node(GF_Proto_Node *node);
extern void gf_proto_destroy_msg(GF_Proto_Message *msg);

extern void *gf_proto_get(GF_Proto_Node *node, int *len);
extern bool gf_proto_set(GF_Proto_Node *node, void *value, int len);

extern GF_Proto_Node *gf_proto_get_filed(GF_Proto_Message *msg, const char *key);
extern bool gf_proto_set_filed(GF_Proto_Message *msg, const char *key, void *value, int len);

extern bool gf_proto_push(GF_Proto_Array_Node *arr, GF_Proto_Node *node);
extern bool gf_proto_pop(GF_Proto_Array_Node *arr);
extern bool gf_proto_erase(GF_Proto_Array_Node *arr, int idx);

extern char *gf_proto_encode(GF_Proto_Message *msg, int *len);
extern GF_Proto_Message *gf_proto_decode(const char * str);

/****************************** interfaces ******************************/

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
#define gfp_new_message(name) gfp_new_node(GF_PROTO_TYPE_MESSAGE, name)


#define gfp_set_int(node, value, len) { if(node->_type != GF_PROTO_TYPE_INT){GF_PROTO_ERROR("wrong type: not int!");}else{GF_PROTO_INT v = (value); gf_proto_set((node), &v, (len));} }
#define gfp_set_float(node, value, len)  { if(node->_type != GF_PROTO_TYPE_FLOAT){GF_PROTO_ERROR("wrong type: not int!");}else{ GF_PROTO_FLOAT v = (value); gf_proto_set((node), &v, (len));} }
//#define gfp_set_array(node, value, len)  { if(node->_type != GF_PROTO_TYPE_ARRAY){GF_PROTO_ERROR("wrong type: not int!");}else{ GF_PROTO_TYPE_ARRAY v = (value); gf_proto_set((node), &v, (len));} }
#define gfp_set_bool(node, value, len)  { if((node)->_type != GF_PROTO_TYPE_BOOL){GF_PROTO_ERROR("wrong type: not int!");}else{ GF_PROTO_BOOL v = (value); gf_proto_set((node), &v, (len));} }
#define gfp_set_string(node, value, len)  { if(node->_type != GF_PROTO_TYPE_STRING){GF_PROTO_ERROR("wrong type: not int!");}else{ GF_PROTO_STRING v = (value); gf_proto_set((node), &v, (len));} }
#define gfp_set_message(node, value, len)  { if(node->_type != GF_PROTO_TYPE_MESSAGE){GF_PROTO_ERROR("wrong type: not int!");}else{ GF_PROTO_MESSAGE v = (value); gf_proto_set((node), &v, (len));} }

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

#endif

/****************************** interfaces ******************************/
#define gfp_node GF_Proto_Node

#define gfp_children(node) (node->_value._avalue._nodes)
#define gfp_array(node) (&(node->_value._avalue))
#define gfp_len(node) ((node)->_len)
extern void gfp_destroy(GF_Proto_Node*);
extern void gfp_unload();
extern void gfp_load_files(const char **filelist, int n);
extern GF_Proto_Node *gfp_new(GF_PROTO_TYPE t, const char *msgname);

#define gfp_new_int(v)   {GF_PROTO_INT v = (value); gfp_new_node(GF_PROTO_TYPE_INT, &v, (0));}
#define gfp_new_float(v)      {GF_PROTO_FLOAT v = (value); gfp_new_node(GF_PROTO_TYPE_FLOAT, &v, (0));}
#define gfp_new_double(v)       gfp_new_node(GF_PROTO_TYPE_DOUBLE, 0)
//#define gfp_new_array()       gfp_new_node(GF_PROTO_TYPE_INT, 0)
#define gfp_new_bool(v)        gfp_new_node(GF_PROTO_TYPE_BOOL, 0)
#define gfp_new_string(v)      gfp_new_node(GF_PROTO_TYPE_STRING, 0)
#define gfp_new_msg(name) gfp_new(GF_PROTO_TYPE_MSG, name)


#if 0
gfp_new()
gfp_clone_node(node)
gfp_clone_array(node)
gfp_get(key)
//gfp_fields(node)
gfp_children(node)
gfp_delete(node)

gfp_int(node)
gfp_float(node)
gfp_double(node)
gfp_array(node)
gfp_bool(node)
gfp_string(node)
gfp_msg(node)

gfp_push(arr, value)
gfp_pop(arr)
gfp_insert(arr, i, v)
gfp_erase(arr, i)
#endif

#endif // _gfproto_h_
