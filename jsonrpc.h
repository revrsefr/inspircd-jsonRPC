#pragma once

#include <nlohmann/json.hpp>
#include "inspircd.h"
#include "protocol.h"
#include "modules.h"
#include "modules/httpd.h"
#include "server.h"
#include "channels.h"
#include "usermanager.h"
#include "mode.h"
#include "xline.h"
#include "stringutils.h"
#include "clientprotocolevent.h"


using json = nlohmann::json;

// JSON-RPC Error Codes
enum JSONRPCErrorCode
{
	JSON_RPC_ERROR_PARSE_ERROR              = -32700,
	JSON_RPC_ERROR_INVALID_REQUEST          = -32600,
	JSON_RPC_ERROR_METHOD_NOT_FOUND         = -32601,
	JSON_RPC_ERROR_INVALID_PARAMS           = -32602,
	JSON_RPC_ERROR_INTERNAL_ERROR           = -32603,
	JSON_RPC_ERROR_API_CALL_DENIED          = -32000,
	JSON_RPC_ERROR_SERVER_GONE              = -32001,
	JSON_RPC_ERROR_TIMEOUT                  = -32002,
	JSON_RPC_ERROR_REMOTE_SERVER_NO_RPC     = -32003,
	JSON_RPC_ERROR_NOT_FOUND                = -1000,
	JSON_RPC_ERROR_ALREADY_EXISTS           = -1001,
	JSON_RPC_ERROR_INVALID_NAME             = -1002,
	JSON_RPC_ERROR_USERNOTINCHANNEL         = -1003,
	JSON_RPC_ERROR_TOO_MANY_ENTRIES         = -1004,
	JSON_RPC_ERROR_DENIED                   = -1005
};

// Utility Functions
bool CheckAuth(HTTPRequest& request);
json CreateError(const std::string& message, int code);
void SendResponse(HTTPRequest& request, json& response);
void SendError(HTTPRequest& request, const std::string& message, int code);

// Server Handlers
json HandleServerList();
json HandleServerGet(const json& params);
json HandleServerRehash();
json HandleServerConnect(const json& params);
json HandleServerDisconnect(const json& params);

// Server Bans
json HandleServerBanList();
json HandleServerBanAdd(const json& params);
json HandleServerBanDel(const json& params);

// Channels
json HandleChannelList();
json HandleChannelGet(const json& params);
json HandleChannelSetMode(const json& params);
json HandleChannelKick(const json& params);

// Users
json HandleUserList();
json HandleUserGet(const json& params);
json HandleUserSetNick(const json& params);
json HandleUserSetUsername(const json& params);
json HandleUserSetRealname(const json& params);
json HandleUserSetVhost(const json& params);
json HandleUserSetMode(const json& params);
json HandleUserSetSnomask(const json& params);
json HandleUserSetOper(const json& params);
