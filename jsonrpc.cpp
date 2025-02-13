

#include "jsonrpc.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ✅ FIXED: Define `CreateError` properly
json CreateError(const std::string& message, int code)
{
	json error;
	error["jsonrpc"] = "2.0";
	error["error"]["code"] = code;
	error["error"]["message"] = message;
	return error;
}

class ModuleJsonRPC : public Module, public HTTPRequestEventListener
{
	HTTPdAPI httpd;
	std::string apiuser;
	std::string apipassword;

public:
	ModuleJsonRPC()
		: Module(VF_VENDOR, "Provides a JSON-RPC API over HTTP."),
		  HTTPRequestEventListener(this),
		  httpd(this)
	{
	}

	void ReadConfig(ConfigStatus& status) override
	{
		auto tag = ServerInstance->Config->ConfValue("jsonrpc");
		apiuser = tag->getString("apiuser", "apiuser");
		apipassword = tag->getString("rpc-user-password", "password");
	}

	ModResult OnHTTPRequest(HTTPRequest& request) override
	{
		if (request.GetPath() != "/jsonrpc")
			return MOD_RES_PASSTHRU;

		if (!CheckAuth(request))
		{
			SendError(request, "Unauthorized", JSON_RPC_ERROR_API_CALL_DENIED);
			return MOD_RES_DENY;
		}

		json req;
		try
		{
			req = json::parse(request.GetPostData());
		}
		catch (const json::parse_error&)
		{
			SendError(request, "Invalid JSON request", JSON_RPC_ERROR_INVALID_REQUEST);
			return MOD_RES_DENY;
		}

		if (!req.contains("method") || !req.contains("id"))
		{
			SendError(request, "Invalid JSON-RPC request format", JSON_RPC_ERROR_INVALID_REQUEST);
			return MOD_RES_DENY;
		}

		std::string method = req["method"];
		json params = req.contains("params") ? req["params"] : json::object();
		int id = req["id"];

		json response;
		response["jsonrpc"] = "2.0";
		response["id"] = id;

		if (method == "server.list")
			response["result"] = HandleServerList();
		else if (method == "server.get")
			response["result"] = HandleServerGet(params);
		else if (method == "server_ban.list")
			response["result"] = HandleServerBanList();
		else if (method == "server_ban.add")
			response["result"] = HandleServerBanAdd(params);
		else if (method == "server_ban.del")
			response["result"] = HandleServerBanDel(params);
		else if (method == "channel.list")
			response["result"] = HandleChannelList();
		else if (method == "channel.get")
			response["result"] = HandleChannelGet(params);
		else if (method == "channel.set_mode")
			response["result"] = HandleChannelSetMode(params);
		else if (method == "channel.kick")
			response["result"] = HandleChannelKick(params);
		else if (method == "user.list")
			response["result"] = HandleUserList();
		else if (method == "user.get")
			response["result"] = HandleUserGet(params);
		else if (method == "user.set_nick")
			response["result"] = HandleUserSetNick(params);
		else
			response["error"] = CreateError("Method not found", JSON_RPC_ERROR_METHOD_NOT_FOUND);

		SendResponse(request, response);
		return MOD_RES_DENY;
	}

	bool CheckAuth(HTTPRequest& request)
	{
		if (!request.headers)
			return false;

		std::string auth_header = request.headers->GetHeader("Authorization");
		if (auth_header.empty() || auth_header.find("Basic ") != 0)
			return false;

		std::string encoded_credentials = auth_header.substr(6);
		std::string decoded = Base64::Decode(encoded_credentials);
		size_t pos = decoded.find(':');

		if (pos == std::string::npos)
			return false;

		std::string user = decoded.substr(0, pos);
		std::string pass = decoded.substr(pos + 1);

		return (user == apiuser && pass == apipassword);
	}

	void SendResponse(HTTPRequest& request, json& response)
	{
		std::stringstream response_data;
		response_data << response.dump();

		HTTPDocumentResponse doc(this, request, &response_data, 200);
		doc.headers.SetHeader("Content-Type", "application/json");
		httpd->SendResponse(doc);
	}

	void SendError(HTTPRequest& request, const std::string& message, int code)
	{
		json response;
		response["jsonrpc"] = "2.0";
		response["error"]["code"] = code;
		response["error"]["message"] = message;

		SendResponse(request, response);
	}

	json CreateError(const std::string& message, int code)
	{
		json error;
		error["code"] = code;
		error["message"] = message;
		return error;
	}

};

MODULE_INIT(ModuleJsonRPC)
