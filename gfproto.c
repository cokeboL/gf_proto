#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "gfproto.h"


//byte order
static GF_PROTO_BYTE_ORDER _byte_order = GF_LITTLE_ENDIAN;

//msg def
static GF_Proto_Msgdef _msg_defs = { 0, 0 };

//parser keys
static const char *_gf_proto_keys[] =
{
	"int",
	"float",
	"double",
	"array",
	"bool",
	"string",
};

void realloc_str(void **str, int len);
const char *get_str_from_file(const char *file, int *filelen);
const char *get_next_msgdef_str(const char *str, int len, const char **next);
const char *get_next_itemdef_str(const char *str, int len, const char **next);
GF_PROTO_TYPE get_next_itemdef_type(const char *typestr, int len);
bool init_node(GF_Proto_Node *node);

#if 0
GF_Proto_Array_Node *get_itemdef_by_str(const char *str, int len);
GF_Proto_Message *parse_msgdef_from_str(const char *str, int slen);
GF_Proto_Message *parse_file(const char *file);
void num_2_str(char **str, void *pnum, int numlen);
bool encode_node(GF_Proto_Node *node, char **str, int slen, int *len);
char *encode_msg(GF_Proto_Message *msg, char **str, int slen, int *len);
GF_Proto_Message *decode_msg(GF_Proto_Message *msgdef, const char *str, int tlen, int *len, bool order_equal);
#endif

/*********************************************************************************************************/
// parser
static GF_PROTO_BYTE_ORDER get_bit_endian()
{
	short n = 0x1;
	return (GF_PROTO_BYTE_ORDER)!*(char*)&n;
}

static int16_t get_msg_hash(GF_Proto_Node *msg)
{
	int i;
	for (i = 0; i < _msg_defs._len; i++)
	{
		if (strcmp(_msg_defs._nodes[i]->_name, msg->_name) == 0)
		{
			return i;
		}
	}
	return -1;
}

static GF_Proto_Node *get_msg_def(const char *name)
{
	int i;
	for (i = 0; i < _msg_defs._len; i++)
	{
		if (strcmp(_msg_defs._nodes[i]->_name, name) == 0)
		{
			return _msg_defs._nodes[i];
		}
	}
	return 0;
}

static void realloc_str(void **str, int len)
{
	if (!*str)
	{
		*str = GFMalloc(len);
	}
	else
	{
		*str = GFRealloc(*str, len);
	}
}

static const char *get_str_from_file(const char *file, int *filelen)
{
	char *head, *begin, *end;
	FILE *fp = fopen(file, "rb");
	if (!fp)
	{
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	char *buf = (char*)GFMalloc(len+1);
	int nread = fread(buf, len, 1, fp);
	buf[len] = 0;

	fclose(fp);

	head = buf;
	while (begin = strstr(head, "//"))
	{
		end = strstr(begin, "\n");
		if (end)
		{
			memmove(begin, end + 1, len - (end - buf + 1));
			len = len - (end - begin + 1);
		}
		else
		{
			len = begin - buf;
		}
		head = begin;
		buf[len] = 0;
	}

	head = buf;
	while (begin = strstr(head, "/*"))
	{
		end = strstr(begin, "*/");
		if (end)
		{
			memmove(begin, end + 2, len - (end - buf + 2));
			len = len - (end - begin + 2);
		}
		else
		{
			len = begin - buf;
		}
		head = begin;
		buf[len] = 0;
	}
	GF_PROTO_INFO("protofile %s: %s\n", file, buf);

	*filelen = len;

	return buf;
}

static const char *get_next_msgdef_str(const char *str, int len, const char **next)
{
	int i = 0;
	char *p = str, *begin, *end;
	
	while (*p == ' ' || *p == '\r'  || *p == '\t' || *p == '\n')
	{
		if (i + 1 > len)
		{
			return 0;
		}

		p++;
		i++;
	}
	begin = p;

	while (*p != '}')
	{
		if (i + 1 > len)
		{
			return 0;
		}

		p++;
		i++;
	}
	end = p;

	*next = p + 1;
	
	return begin;
}

static const char *get_next_itemdef_str(const char *str, int len, const char **next)
{
	int i = 0;
	char *p = str, *begin, *end;

	while (*p == '{' || *p == '\r' || *p == '\t' || *p == '\n' || *p == ' ' || *p == '=')
	{
		if (i + 1 > len)
		{
			return 0;
		}

		p++;
		i++;
	}
	if (*p == "}")
	{
		return 0;
	}

	begin = p;

	while (*p != ';')
	{
		if (i + 1 > len)
		{
			return 0;
		}

		p++;
		i++;
	}
	end = p;

	*next = p + 1;

	return begin;
}

static GF_PROTO_TYPE get_next_itemdef_type(const char *typestr, int len)
{
	int i;
	for (i = 0; i < (sizeof(_gf_proto_keys) / sizeof(_gf_proto_keys[0])); i++)
	{
		if (memcmp(typestr, _gf_proto_keys[i], len) == 0)
		{
			return i;
		}
	}

	return i;
}

static bool init_node(GF_Proto_Node *node)
{
	memset(&node->_value, 0, sizeof(node->_value));
	node->_len = 0;
	return true;
}

static bool init_array(GF_Proto_Node *node)
{
	//gfp_array(node)._type = get_next_itemdef_type(begin, p - begin);
	gfp_array(node)->_len = 0;
	gfp_array(node)->_nodes = 0;

	return true;
}

static GF_Proto_Node *get_itemdef_by_str(const char *str, int len)
{

	GF_Proto_Node *typenode = 0;
	realloc_str(&typenode, sizeof(GF_Proto_Node));

	init_node(typenode);

	int i = 0;
	char *p = str, *begin, *end;

	while (*p != ' ' && *p != '\r' && *p != '\t' && *p != '\n' && *p != '=')
	{
		if (i + 1 > len)
		{
			GFFree(typenode);
			return 0;
		}

		p++;
		i++;
	}
	GF_PROTO_TYPE nodetype = get_next_itemdef_type(str, p - str);
	if (GF_PROTO_TYPE_ARRAY == nodetype)
	{
		while (*p == ' ' || *p == '\r' || *p == '\t' || *p == '\n')
		{
			if (i + 1 > len)
			{
				GFFree(typenode);
				return 0;
			}

			p++;
			i++;
		}
		begin = p;
		while (*p != ';' && *p != ' ' && *p != '\r' && *p != '\t' && *p != '\n')
		{
			if (i + 1 > len)
			{
				GFFree(typenode);
				return 0;
			}

			p++;
			i++;
		}
		GF_PROTO_TYPE arrtype = get_next_itemdef_type(begin, p - begin);
		if (arrtype == GF_PROTO_TYPE_MSG)
		{
			memcpy(gfp_array(typenode)->_name, begin, p - begin);
			gfp_array(typenode)->_name[p - begin] = 0;
		}
		
		gfp_array(typenode)->_type = get_next_itemdef_type(begin, p - begin);
		//gfp_array(node)._len = 0;
		//gfp_array(node)._nodes = 0;
		init_array(typenode);
	}
	
	typenode->_type = nodetype;

	while (*p == ' ' || *p == '\r' || *p == '\t' || *p == '\n')
	{
		if (i + 1 > len)
		{
			gfp_destroy(typenode);
			return 0;
		}

		p++;
		i++;
	}
	begin = p;
	while (*p != ';' && *p != ' ' && *p != '\r' && *p != '\t' && *p != '\n')
	{
		if (i + 1 > len)
		{
			gfp_destroy(typenode);
			return 0;
		}

		p++;
		i++;
	}

	memcpy(typenode->_name, begin, p - begin);
	typenode->_name[p - begin] = 0;

	

	return typenode;
}

static GF_Proto_Node *parse_msgdef_from_str(const char *str, int slen)
{
	GF_Proto_Node *msg = (GF_Proto_Node*)GFMalloc(sizeof(GF_Proto_Node));
	msg->_len = 0;
	//msg->_value._avalue._nodes
	gfp_nodes(msg) = 0;
	

	GF_Proto_Node *typenode = 0;

	int i = 0;
	int len, msglen;
	const char *curr = str, *next;

	char *p = str, *begin = str, *end;
	while (*p != ' ' && *p != '\r' && *p != '\t' && *p != '\n')
	{
		if (i + 1 > slen)
		{
			GFFree(msg);
			return 0;
		}

		p++;
		i++;
	}
	memcpy(msg->_name, begin, i);
	msg->_name[i] = 0;

	next = p;
	while (curr = get_next_itemdef_str(next, slen - (next - str), &next))
	{
		msglen = next - curr;
		typenode = get_itemdef_by_str(curr, msglen);
		if (typenode)
		{
			msg->_len++;

			realloc_str(&gfp_nodes(msg), sizeof(GF_Proto_Node*)* msg->_len);
			gfp_nodes(msg)[msg->_len - 1] = typenode;
		}
	}

	return msg;
}


static GF_Proto_Node *parse_file(const char *file)
{
	int len, msglen;
	GF_Proto_Node *msg = 0;

	const char *fstr = get_str_from_file(file, &len);
	const char *curr, *next = fstr;
	while (curr = get_next_msgdef_str(next, len, &next))
	{
		msglen = next - curr;
		msg = parse_msgdef_from_str(curr, msglen);
		if (msg)
		{
			_msg_defs._len++;
			realloc_str(&_msg_defs._nodes, _msg_defs._len * sizeof(GF_Proto_Node*));

			_msg_defs._nodes[_msg_defs._len - 1] = msg;
		}
	}

	GFFree(fstr);

	return 0;
}

void gfp_load_files(const char **filelist, int n)
{
	_byte_order = get_bit_endian();

	for (int i = 0; i < n; i++)
	{
		parse_file(filelist[i]);
	}
}

void gfp_unload()
{
	int i;
	for (i = 0; i < _msg_defs._len; i++)
	{
		//gfp_destroy(_msg_defs._nodes[i]);
	}
	if (_msg_defs._nodes)
	{
		GFFree(_msg_defs._nodes);
	}
}

GF_Proto_Node *gf_proto_new_node(GF_PROTO_TYPE t, const char *msgname)
{
	GF_Proto_Node *node = 0;
	realloc_str(&node, sizeof(GF_Proto_Node));
	node->_type = t;
	init_node(node);
	if (t == GF_PROTO_TYPE_MESSAGE)
	{
		strcpy(node->_name, msgname);
		GF_Proto_Node *msgdef = get_msg_def(msgname);
		GF_Proto_Node **nodes = gfp_nodes(node); 
		GF_Proto_Node **defnodes = gfp_nodes(msgdef);
		int16_t len = gfp_len(gfp_array(msgdef));
		for (int i = 0; i < len; i++)
		{
			realloc_str(&nodes[i], sizeof(GF_Proto_Node));
		}
	}
	else if (t == GF_PROTO_TYPE_ARRAY)
	{
		strcpy(gfp_array(node)->_name, msgname)
		init_array(node);
	}

	return node;
}


void gfp_destroy(GF_Proto_Node* node)
{

}

void gfp_push(GF_Proto_Array* arr, GF_Proto_Node *value)
{

}

void gfp_pop(GF_Proto_Array* arr)
{

}

void gfp_insert(GF_Proto_Array* arr, int16_t idx, GF_Proto_Node *value)
{

}

void gfp_erase(GF_Proto_Array* arr, int16_t idx)
{

}