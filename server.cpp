

#include "jsonrpc.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Handle server.list
json HandleServerList()
{
	json result = json::array();

	// Create a list to store the servers
	ProtocolInterface::ServerList servers;
	ServerInstance->PI->GetServerList(servers);

	for (const auto& server : servers)
	{
		json obj;
		obj["name"] = server.servername;
		obj["parent"] = server.parentname;
		obj["description"] = server.description;
		obj["user_count"] = server.usercount;
		obj["op_count"] = server.opercount;
		obj["latency_ms"] = server.latencyms;
		result.push_back(obj);
	}

	return result;
}

// Handle server.get
json HandleServerGet(const json& params)
{
	if (!params.contains("server"))
		return CreateError("Missing parameter: server", JSON_RPC_ERROR_INVALID_PARAMS);

	std::string server_name = params["server"].get<std::string>();

	ProtocolInterface::ServerList servers;
	ServerInstance->PI->GetServerList(servers);

	for (const auto& server : servers)
	{
		if (server.servername == server_name)
		{
			json obj;
			obj["name"] = server.servername;
			obj["parent"] = server.parentname;
			obj["description"] = server.description;
			obj["user_count"] = server.usercount;
			obj["op_count"] = server.opercount;
			obj["latency_ms"] = server.latencyms;
			return obj;
		}
	}

	return CreateError("Server not found", JSON_RPC_ERROR_NOT_FOUND);
}

// Handle server.rehash
json HandleServerRehash()
{
	ServerInstance->Rehash();
	return {{"result", "Server rehashed successfully"}};
}

// Handle server.connect (Not supported via JSON-RPC)
json HandleServerConnect(const json& params)
{
	return CreateError("Server connection via RPC is not supported", JSON_RPC_ERROR_METHOD_NOT_FOUND);
}

// Handle server.disconnect (Not supported)
json HandleServerDisconnect(const json& params)
{
	return CreateError("Server disconnection via RPC is not supported", JSON_RPC_ERROR_METHOD_NOT_FOUND);
}
