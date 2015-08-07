#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "gfproto2.h"


//byte order
static GF_PROTO_BYTE_ORDER _byte_order = GF_LITTLE_ENDIAN;

//msg def
static GF_Proto_Message_Array _msg_def_array = { 0, 0 };

//parser keys
static const char *_gf_proto_keys[] =
{
	"int",
	"float",
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
GF_Proto_Array_Node *get_itemdef_by_str(const char *str, int len);
GF_Proto_Message *parse_msgdef_from_str(const char *str, int slen);
GF_Proto_Message *parse_file(const char *file);
void num_2_str(char **str, void *pnum, int numlen);
bool encode_node(GF_Proto_Node *node, char **str, int slen, int *len);
char *encode_msg(GF_Proto_Message *msg, char **str, int slen, int *len);
GF_Proto_Message *decode_msg(GF_Proto_Message *msgdef, const char *str, int tlen, int *len, bool order_equal);

/*********************************************************************************************************/
// parser
static GF_PROTO_BYTE_ORDER get_bit_endian()
{
	short n = 0x1;
	return (GF_PROTO_BYTE_ORDER)!*(char*)&n;
}

static int16_t get_msg_hash(GF_Proto_Message *msg)
{
	int i;
	for (i = 0; i < _msg_def_array._msg_def_len; i++)
	{
		if (strcmp(_msg_def_array._msg_defs[i], msg->_name) == 0)
		{
			return i;
		}
	}
	return -1;
}

static GF_Proto_Message *get_msg_def(const char *name)
{
	int i;
	for (i = 0; i < _msg_def_array._msg_def_len; i++)
	{
		if (strcmp(_msg_def_array._msg_defs[i]->_name, name) == 0)
		{
			return _msg_def_array._msg_defs[i];
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

static GF_Proto_Array_Node *get_itemdef_by_str(const char *str, int len)
{
	
	GF_Proto_Array_Node *typenode = 0;// (GF_Proto_Node*)GFMalloc(sizeof(GF_Proto_Node));

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
		realloc_str(&typenode, sizeof(GF_Proto_Array_Node));

		while (*p == ' ' || *p == '\r' || *p == '\t' || *p == '\n')
		{
			if (i + 1 > len)
			{
				gf_proto_destroy_node(typenode);
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
				gf_proto_destroy_node(typenode);
				return 0;
			}

			p++;
			i++;
		}
		memcpy(typenode->_nodetypename, begin, p - begin);
		typenode->_nodetypename[p - begin] = 0;

		typenode->_nodetype = get_next_itemdef_type(begin, p - begin);
	}
	else
	{
		realloc_str(&typenode, sizeof(GF_Proto_Node));
	}
	typenode->_type = nodetype;

	while (*p == ' ' || *p == '\r' || *p == '\t' || *p == '\n')
	{
		if (i + 1 > len)
		{
			gf_proto_destroy_node(typenode);
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
			gf_proto_destroy_node(typenode);
			return 0;
		}

		p++;
		i++;
	}

	memcpy(typenode->_name, begin, p - begin);
	typenode->_name[p - begin] = 0;
	
	init_node(typenode);

	return typenode;
}

static GF_Proto_Message *parse_msgdef_from_str(const char *str, int slen)
{
	GF_Proto_Message *msg = (GF_Proto_Message*)GFMalloc(sizeof(GF_Proto_Message));
	msg->_len = 0;
	msg->_nodes = 0;

	GF_Proto_Array_Node *typenode = 0;

	int i = 0;
	int len, msglen;
	const char *curr = str, *next;

	char *p = str, *begin = str, *end;
	while(*p != ' ' && *p != '\r' && *p != '\t' && *p != '\n')
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
			
			realloc_str(&(msg->_nodes), sizeof(GF_Proto_Node*)* msg->_len);
			msg->_nodes[msg->_len - 1] = typenode;
		}
	}

	return msg;
}



static GF_Proto_Message *parse_file(const char *file)
{
	int len, msglen;
	GF_Proto_Message *msg = 0;

	const char *fstr = get_str_from_file(file, &len);
	const char *curr, *next = fstr;
	while (curr = get_next_msgdef_str(next, len, &next))
	{
		msglen = next - curr;
		msg = parse_msgdef_from_str(curr, msglen);
		if (msg)
		{
			_msg_def_array._msg_def_len++;
			if (!_msg_def_array._msg_defs)
			{
				_msg_def_array._msg_defs = GFMalloc(sizeof(_msg_def_array._msg_defs) * _msg_def_array._msg_def_len);
			}
			else
			{
				_msg_def_array._msg_defs = GFRealloc(_msg_def_array._msg_defs, sizeof(_msg_def_array._msg_defs) * _msg_def_array._msg_def_len);
			}
			_msg_def_array._msg_defs[_msg_def_array._msg_def_len - 1] = msg;
		}
	}
	
	GFFree(fstr);

	return 0;
}



static void num_2_str(char **str, void *pnum, int numlen)
{

}

static bool encode_node(GF_Proto_Node *node, char **str, int slen, int *len)
{
	int llen, i;
	GF_Proto_Node **arrnodes;
	switch (node->_type)
	{
	case GF_PROTO_TYPE_INT:
		realloc_str(str, slen + sizeof(GF_PROTO_INT));
		*(GF_PROTO_INT*)(*str + slen) = node->_value._ivalue;
		slen += sizeof(GF_PROTO_INT);
		break;
	case GF_PROTO_TYPE_FLOAT:
		realloc_str(str, slen + sizeof(GF_PROTO_FLOAT));
		*(GF_PROTO_FLOAT*)(*str + slen) = node->_value._fvalue;
		slen += sizeof(GF_PROTO_FLOAT);
		break;
	case GF_PROTO_TYPE_STRING:
		realloc_str(str, slen + node->_len + 2);
		*(int16_t*)(*str + slen) = node->_len; 
		if (node->_len > 0)
		{	
			memcpy(*str + slen + 2, node->_value._svalue, node->_len);
			slen += node->_len;
		}
		slen += 2;
		break;
	case GF_PROTO_TYPE_MESSAGE:
		realloc_str(str, slen + 1);
		if (node->_value._mvalue)
		{
			*(bool*)(*str + slen) = true;
			slen += 1;
			encode_msg(node->_value._mvalue, str, slen, &llen);
			slen = llen;
		}
		else
		{
			*(bool*)(*str + slen) = false;
			slen += 1;
		}
		break;
	case GF_PROTO_TYPE_ARRAY:
		realloc_str(str, slen + 2);
		*(int16_t*)(*str + slen) = node->_len;
		slen += 2;
		if (node->_len > 0)
		{
			arrnodes = node->_value._avalue;
			for (i = 0; i < node->_len; i++)
			{
				encode_node(arrnodes[i], str, slen, &llen);
				slen = llen;
			}
		}
		break;
	case GF_PROTO_TYPE_BOOL:
		realloc_str(str, slen + sizeof(GF_PROTO_BOOL));
		*(GF_PROTO_BOOL*)(*str + slen) = node->_value._bvalue;
		slen += sizeof(GF_PROTO_BOOL);
		break;
	default:
		GF_PROTO_ERROR("wrong node type!");
		return false;
		break;
	}

	*len = slen;

	return true;
}

static char *encode_msg(GF_Proto_Message *msg, char **str, int slen, int *len)
{
	int i;
	int llen;
	for (i = 0; i < msg->_len; i++)
	{
		encode_node(msg->_nodes[i], str, slen, &llen);
		slen = llen;
	}

	*len = slen;

	return *str;
}

static GF_Proto_Array_Node *decode_node(GF_Proto_Array_Node *nodedef, GF_Proto_Array_Node *node, const char *str, int tlen, int *len, bool order_equal)
{
	//GF_Proto_Array_Node *node = 0;
	int slen = tlen, llen = 0, i;
	GF_Proto_Node **arrnodes;

	switch (nodedef->_type)
	{
	case GF_PROTO_TYPE_INT:
		//realloc_str(&node, sizeof(GF_Proto_Node));
		*(GF_Proto_Node*)node = *(GF_Proto_Node*)nodedef;
		node->_value._ivalue = *(GF_PROTO_INT*)str;
		slen -= sizeof(GF_PROTO_INT);
		break;
	case GF_PROTO_TYPE_FLOAT:
		//realloc_str(&node, sizeof(GF_Proto_Node));
		*(GF_Proto_Node*)node = *(GF_Proto_Node*)nodedef;
		node->_value._fvalue = *(GF_PROTO_FLOAT*)str;
		slen -= sizeof(GF_PROTO_FLOAT);
		break;
	case GF_PROTO_TYPE_STRING:
		//realloc_str(&node, sizeof(GF_Proto_Node));
		*(GF_Proto_Node*)node = *(GF_Proto_Node*)nodedef;
		node->_len = *(int16_t*)str;
		if (node->_len > 0)
		{
			realloc_str(&(node->_value._svalue), node->_len);
			memcpy(node->_value._svalue, str + 2, node->_len);
			slen -= node->_len;
		}
		slen -= 2;
		break;
	case GF_PROTO_TYPE_MESSAGE:
		//realloc_str(&node, sizeof(GF_Proto_Node));
		*(GF_Proto_Node*)node = *(GF_Proto_Node*)nodedef;
		bool has = *(bool*)str;
		slen -= 1;
		if (has)
		{
			decode_msg(get_msg_def(node->_name), str + 1, slen, &llen, order_equal);
			slen = llen;
		}
		
		break;
	case GF_PROTO_TYPE_ARRAY:
		//realloc_str(&node, sizeof(GF_Proto_Array_Node));
		*(GF_Proto_Array_Node*)node = *(GF_Proto_Array_Node*)nodedef;
		node->_value._avalue = 0;
		node->_len = *(int16_t*)str;
		slen -= 2;
		if (node->_len > 0)
		{
			realloc_str(&(node->_value._avalue), sizeof(GF_Proto_Node*)* node->_len);
			GF_Proto_Array_Node *subdef = 0;
			if (node->_nodetype != GF_PROTO_TYPE_ARRAY)
			{
				realloc_str(&subdef, sizeof(GF_Proto_Node));
			}
			else
			{
				realloc_str(&subdef, sizeof(GF_Proto_Array_Node));
			}
			
			subdef->_type = node->_nodetype;
			if (node->_nodetype == GF_PROTO_TYPE_MESSAGE)
			{
				strcpy(subdef->_name, node->_nodetypename);
			}
			
			
			//str += 2;
			for (i = 0; i < node->_len; i++)
			{
				GF_Proto_Array_Node *subnode = 0;
				if (node->_nodetype != GF_PROTO_TYPE_ARRAY)
				{
					realloc_str(&subnode, sizeof(GF_Proto_Node));
				}
				else
				{
					realloc_str(&subnode, sizeof(GF_Proto_Array_Node));
				}

				subnode->_type = node->_nodetype;
				if (node->_nodetype == GF_PROTO_TYPE_MESSAGE)
				{
					strcpy(subnode->_name, node->_nodetypename);
				}
				
				node->_value._avalue[i] = decode_node(subdef, subnode, str + tlen - slen, slen, &llen, order_equal);
				slen = llen;
			}
			GFFree(subdef);
		}
		break;
	case GF_PROTO_TYPE_BOOL:
		//realloc_str(&node, sizeof(GF_Proto_Node));
		*(GF_Proto_Node*)node = *(GF_Proto_Node*)nodedef;
		node->_value._bvalue = *(GF_PROTO_BOOL*)str;
		slen -= sizeof(GF_PROTO_BOOL);
		break;
	default:
		GF_PROTO_ERROR("wrong node type!");
		return 0;
		break;
	}

	*len = slen;

	return node;
}

static GF_Proto_Message *decode_msg(GF_Proto_Message *msgdef, const char *str, int tlen, int *len, bool order_equal)
{
	GF_Proto_Message *msg = gf_proto_new_msg(msgdef->_name);

	int i, slen = tlen, llen = 0;
	GF_Proto_Array_Node *nodedef;
	for (i = 0; i < msgdef->_len; i++)
	{
		nodedef = msgdef->_nodes[i];

		decode_node(nodedef, msg->_nodes[i], str + tlen - slen, slen, &llen, order_equal);
		slen = llen;
		//decode_node(GF_Proto_Array_Node *nodedef, const char *str, int slen, int *len)
	}

	if (len)
	{
		*len = slen;
	}
	return msg;
}

/*********************************************************************************************************/
// public interface

void gf_proto_load(const char **filelist, int n)
{
	_byte_order = get_bit_endian();

	GF_Proto_Message *msg, *msgxx2;
	GF_Proto_Node *xx2;
	int i;
	for (i = 0; i < n; i++)
	{
		parse_file(filelist[i]);
	}
}

void gf_proto_unload()
{
	int i;
	for (i = 0; i < _msg_def_array._msg_def_len; i++)
	{
		gf_proto_destroy_msg(_msg_def_array._msg_defs[i]);
	}
	if (_msg_def_array._msg_defs)
	{
		GFFree(_msg_def_array._msg_defs);
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
	}

	return node;
}

GF_Proto_Message *gf_proto_new_msg(const char *msgname)
{
	GF_Proto_Message *msg = 0, *msgdef;
	GF_Proto_Array_Node *node, *nodedef;
	int i, j;
	for (i = 0; i < _msg_def_array._msg_def_len; i++)
	{
		msgdef = _msg_def_array._msg_defs[i];
		if (strcmp(msgname, msgdef->_name) == 0)
		{
			realloc_str(&msg, sizeof(GF_Proto_Message));
			*msg = *msgdef;
			msg->_nodes = 0;
			realloc_str(&msg->_nodes, sizeof(GF_Proto_Array_Node*)* msg->_len);
			for (j = 0; j < msg->_len; j++)
			{
				nodedef = msgdef->_nodes[j];
				msg->_nodes[j] = 0;
				if (nodedef->_type != GF_PROTO_TYPE_ARRAY)
				{
					realloc_str(msg->_nodes + j, sizeof(GF_Proto_Node));
					*(GF_Proto_Node*)(msg->_nodes[j]) = *(GF_Proto_Node*)nodedef;
				}
				else
				{
					realloc_str(msg->_nodes + j, sizeof(GF_Proto_Array_Node));
					*msg->_nodes[j] = *nodedef;
				}
				//msg->_nodes[j] = (_msg_def_array._msg_defs[i]->_nodes[j]);
				init_node(msg->_nodes[j]);
			}

			return msg;
		}
	}

	return 0;
}

void gf_proto_destroy_node(GF_Proto_Node *node)
{
	if (!node)
	{
		return;
	}

	//int16_t len, i;
	//GF_Proto_Node **arrnodes;

	switch (node->_type)
	{
	case GF_PROTO_TYPE_INT:
	case GF_PROTO_TYPE_FLOAT:
	case GF_PROTO_TYPE_BOOL:
		break;
	case GF_PROTO_TYPE_STRING:
		if (node->_len)
		{
			GFFree(node->_value._svalue);
		}
		break;
	case GF_PROTO_TYPE_MESSAGE:
		gf_proto_destroy_msg(node->_value._mvalue);
		break;
	case GF_PROTO_TYPE_ARRAY:
		if (node->_len > 0)
		{
			//arrnodes = node->_value._avalue;
			int i;
			for (i = 0; i < node->_len; i++)
			{
				gf_proto_destroy_node(node->_value._avalue[i]);
			}
			GFFree(node->_value._avalue);
		}
		
		break;
	default:
		break;
	}

	GFFree(node);

}

void gf_proto_destroy_msg(GF_Proto_Message *msg)
{
	if (!msg)
	{
		return;
	}

	int i;
	GF_Proto_Node *node = 0;
	for (i = 0; i < msg->_len; i++)
	{
		gf_proto_destroy_node(msg->_nodes[i]);
	}
	GFFree(msg->_nodes);
	GFFree(msg);
}

void *gf_proto_get(GF_Proto_Node *node, int *len)
{
	switch (node->_type)
	{
	case GF_PROTO_TYPE_INT:
	case GF_PROTO_TYPE_FLOAT:
	case GF_PROTO_TYPE_MESSAGE:
	case GF_PROTO_TYPE_ARRAY:
	case GF_PROTO_TYPE_BOOL:
		return (void*)&node->_value;
		break;
	case GF_PROTO_TYPE_STRING:
		if (len != 0)
		{
			*len = node->_len;
		}
		return (void*)&node->_value;
		break;
	default:
		GF_PROTO_ERROR("wrong node type!");
		break;
	}

	return 0;
}

bool gf_proto_set(GF_Proto_Node *node, void *value, int len)
{
	switch (node->_type)
	{
	case GF_PROTO_TYPE_INT:
		node->_value._ivalue = *(GF_PROTO_INT*)value;
		break;
	case GF_PROTO_TYPE_FLOAT:
		node->_value._fvalue = *(GF_PROTO_FLOAT*)value;
		break;
	case GF_PROTO_TYPE_STRING:
		node->_value._svalue = (GF_PROTO_STRING)GFMalloc(len);
		memcpy(node->_value._svalue, *(GF_PROTO_STRING*)value, len);
		node->_len = len;
		break;
	case GF_PROTO_TYPE_MESSAGE:
		node->_value._mvalue = *(GF_PROTO_MESSAGE*)value;
		break;
	case GF_PROTO_TYPE_ARRAY:
		//node->_value._avalue = (GF_PROTO_ARRAY)value;
		//node->_len = len;
		GF_PROTO_ERROR("array shouldn't be set!"); 
		return false;
		break;
	case GF_PROTO_TYPE_BOOL:
		node->_value._bvalue = *(GF_PROTO_BOOL*)value;;
		break;
	default:
		GF_PROTO_ERROR("wrong node type!");
		return false;
		break;
	}

	return true;
}


void *gf_proto_get_filed(GF_Proto_Message *msg, const char *key)
{
	int i;
	for (i = 0; i < msg->_len; i++)
	{
		if (strcmp(key, msg->_nodes[i]->_name) == 0)
		{
			if (msg->_nodes[i]->_type != GF_PROTO_TYPE_MESSAGE)
			{
				return msg->_nodes[i];
			}
			else
			{
				return msg->_nodes[i]->_value._mvalue;
			}
			
		}
	}

	GF_PROTO_ERROR("no such filed: %s!", key);
	return 0;
}

bool gf_proto_set_filed(GF_Proto_Message *msg, const char *key, void *value, int len)
{
	int i;
	for (i = 0; i < msg->_len; i++)
	{
		if (strcmp(key, msg->_nodes[i]->_name) == 0)
		{
			return gf_proto_set(msg->_nodes[i], value, len);
		}
	}

	GF_PROTO_ERROR("no such filed: %s!", key);
	return false;
}


bool gf_proto_push(GF_Proto_Array_Node *arr, GF_Proto_Node *node)
{
	if (arr->_type != GF_PROTO_TYPE_ARRAY)
	{
		GF_PROTO_ERROR("wrong type: is not array!");
		return false;
	}

	if (arr->_nodetype != node->_type)
	{
		GF_PROTO_ERROR("wrong node to push: is a %s, need %s!", node->_name, arr->_nodetypename);
		return false;
	}
	else if (GF_PROTO_TYPE_MESSAGE == arr->_nodetype && (strcmp(arr->_nodetypename, node->_name) != 0))
	{
		GF_PROTO_ERROR("wrong node to push: is a %s, need %s!", node->_name, arr->_nodetypename);
		return false;
	}

	if (arr->_len == 0)
	{
		realloc_str(&(arr->_value._avalue), sizeof(GF_Proto_Node*));
		arr->_len++;
		(arr->_value._avalue)[0] = node;
	}
	else
	{
		realloc_str(&(arr->_value._avalue), sizeof(GF_Proto_Node*)* (arr->_len + 1));
		arr->_len = arr->_len + 1;
		arr->_value._avalue[arr->_len] = node;
	}

	return true;
}

bool gf_proto_pop(GF_Proto_Array_Node *arr)
{
	int16_t arrlen;
	if (arr->_type != GF_PROTO_TYPE_ARRAY)
	{
		GF_PROTO_ERROR("wrong type: is not array!");
		return false;
	}

	if (arr->_len > 0)
	{
		arr->_len = arr->_len - 1;
		gf_proto_destroy_node(arr->_value._avalue[arr->_len]);
		if (arr->_len == 0)
		{
			GFFree(arr->_value._avalue);
			arr->_value._avalue = 0;
		}
		else
		{
			realloc_str(&(arr->_value._avalue), sizeof(GF_Proto_Node)* arr->_len);
		}
	}	

	return true;
}

bool gf_proto_erase(GF_Proto_Array_Node *arr, int idx)
{
	int16_t arrlen;
	GF_Proto_Node *targetpos;
	arrlen = arr->_len;


	if (arr->_type != GF_PROTO_TYPE_ARRAY)
	{
		GF_PROTO_ERROR("wrong type: is not array!");
		return false;
	}

	if (idx > arr->_len)
	{
		GF_PROTO_ERROR("wrong idx for erase: out of range!");
		return false;
	}

	if (arr->_len <= 0)
	{
		GF_PROTO_ERROR("array is empty, no node to be erased!");
		return false;
	}
	else
	{
		gf_proto_destroy_node(arr->_value._avalue[idx]);
		arr->_len--;
		memmove(arr->_value._avalue + idx, arr->_value._avalue + idx + 1, sizeof(GF_Proto_Node*)* (arr->_len - idx));
		realloc_str(arr->_value._avalue, sizeof(GF_Proto_Node) * arr->_len);
	}

	return true;
}

char *gf_proto_encode(GF_Proto_Message *msg, int *len)
{
	uint16_t hash = get_msg_hash(msg);
	if (hash < 0)
	{
		return 0;
	}

	hash |= (_byte_order << (15 - 1));

	char *str = 0;
	realloc_str(&str, 2);
	*(uint16_t*)str = hash;

	if (encode_msg(msg, &str, 2, len))
	{
		return str;
	}

	return 0;
}

/*******************************************************************/

GF_Proto_Message *gf_proto_decode(const char * str, int slen)
{
	//realloc_str(&str, 2);
	uint16_t hash = *(uint16_t*)str;

	GF_PROTO_BYTE_ORDER border = (hash >> 15);
	hash ^= (border << 15);

	if (hash >= 0 && hash < _msg_def_array._msg_def_len)
	{
		GF_Proto_Message *msgdef = _msg_def_array._msg_defs[hash];
		
		return decode_msg(msgdef, str + 2, slen - 2, 0, border == _byte_order);
	}

	GF_PROTO_ERROR("no such msg defined!");

	return 0;
}