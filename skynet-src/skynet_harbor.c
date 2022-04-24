#include "skynet.h"
#include "skynet_harbor.h"
#include "skynet_server.h"
#include "skynet_mq.h"
#include "skynet_handle.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

static struct skynet_context *REMOTE = 0;
static unsigned int HARBOR = ~0;

static inline int
invalid_type(int type)
{
	return type != PTYPE_SYSTEM && type != PTYPE_HARBOR;
}

/*
向harbor发送消息（远程消息），
本质上是把消息交给专门负责远程的REMOTE服务去做远程消息的发送
*/
void skynet_harbor_send(struct remote_message *rmsg, uint32_t source, int session)
{
	assert(invalid_type(rmsg->type) && REMOTE);
	skynet_context_send(REMOTE, rmsg, sizeof(*rmsg), source, PTYPE_SYSTEM, session);
}

/*
判断handle是否是来自于远程节点，
远程节点的标记是：最高位1字节不是当前的HARBOR且不为0
*/
int skynet_harbor_message_isremote(uint32_t handle)
{
	assert(HARBOR != ~0);
	int h = (handle & ~HANDLE_MASK);
	return h != HARBOR && h != 0;
}

void skynet_harbor_init(int harbor)
{
	HARBOR = (unsigned int)harbor << HANDLE_REMOTE_SHIFT;
}

void skynet_harbor_start(void *ctx)
{
	// the HARBOR must be reserved to ensure the pointer is valid.
	// It will be released at last by calling skynet_harbor_exit
	skynet_context_reserve(ctx);
	REMOTE = ctx;
}

void skynet_harbor_exit()
{
	struct skynet_context *ctx = REMOTE;
	REMOTE = NULL;
	if (ctx)
	{
		skynet_context_release(ctx);
	}
}
