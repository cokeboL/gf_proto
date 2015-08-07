#if _WIN32
#include "vld.h"
#endif

#include "gfproto.h"

#if 0
#define gfp_new gf_proto_new
#define gfp_dele_node gf_proto_destroy_node
#define gfp_dele_msg gf_proto_destroy_msg
#define gfp_get gf_proto_get
#define gfp_set gf_proto_set
#define gfp_get_field gf_proto_get_filed
#define gfp_set_field gf_proto_set_filed
#define gfp_encode gf_proto_encode
#define gfp_decode gf_proto_decode

#define gfp_int(node)    (GF_PROTO_INT)(((GF_Proto_Value*)gfp_get(node, 0))->_ivalue)
#define gfp_float(node)  (GF_PROTO_FLOAT)(((GF_Proto_Value*)gfp_get(node, 0))->_fvalue)
#define gfp_array(node)  (GF_PROTO_ARRAY)(((GF_Proto_Value*)gfp_get(node, 0))->_avalue)
#define gfp_bool(node)  (GF_PROTO_BOOL)(((GF_Proto_Value*)gfp_get(node, 0))->_bvalue)
#define gfp_string(node, len)  (GF_PROTO_STRING)(((GF_Proto_Value*)gfp_get(node, len))->_svalue)
#define gfp_message(node)  (GF_PROTO_MESSAGE)(node->_value._avalue)
#endif

int main()
{
	const char *filelist[] =
	{
		"msgconfig.lua",
	};

	gfp_load_files(filelist, sizeof(filelist) / sizeof(filelist[0]));


#if 1
	gfp_node *msg = gfp_new_msg("xx2");
#endif
	
	printf("\n***************************************\n");

	gfp_unload();

	getchar();
}