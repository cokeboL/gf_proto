#if _WIN32
#include "vld.h"
#endif

#include "gfproto2.h"

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


#if 0
	printf("\n***************************************\n");
	gfp_msg *msg = gfp_new_msg("xx2");
	
	gfp_node *ss = gfp_get_field(msg, "ss");
	gfp_set_string(ss, "hello", strlen("hello") + 1);


	gfp_node *ii = gfp_get_field(msg, "ii");
	gfp_set_int(ii, 369);

	gfp_array_node *axx = gfp_get_field(msg, "axx");
	gfp_node *xx1 = gfp_new_mes_node("xx");
	gfp_msg *axxmsg1 = gfp_new_msg("xx");
	gfp_set_msg(xx1, axxmsg1);
	gfp_push(axx, xx1); 
	gfp_node *axxii1 = gfp_get_field(xx1, "ii");
	gfp_set_int(axxii1, 1);
	

	gfp_node *bb = gfp_get_field(msg, "bb");
	gfp_set_bool(bb, true);

	gfp_node *ff = gfp_get_field(msg, "ff");
	gfp_set_float(ff, 78.654);

	gfp_array_node *aii = gfp_get_field(msg, "aii");
	gfp_node *aiinode = gfp_new_int();
	gfp_set_int(aiinode, 480);
	gfp_push(aii, aiinode);

	int len;
	char *buf = gfp_encode(msg, &len);

	printf("len: %d\n", len);
	for (int i = 0; i < len; i++)
	{
		printf("%c", buf[i]);
	}
	printf("\n");
#endif
	
	printf("\n***************************************\n");

	gfp_unload();

	getchar();
}