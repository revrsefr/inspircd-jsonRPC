
#include "jsonrpc.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Handle server_ban.list
json HandleServerBanList()
{
	json response;
	response["jsonrpc"] = "2.0";
	response["result"] = json::array();

	// Get the G-line ban list
	XLineLookup* bans = ServerInstance->XLines->GetAll("G");
	if (!bans)
		return response;  // Return empty result if no bans exist

	for (const auto& entry : *bans)
	{
		XLine* ban = entry.second;
		if (!ban)
			continue;

		json b;
		b["mask"] = ban->Displayable();
		b["set_by"] = ban->source;
		b["reason"] = ban->reason;
		b["expiry"] = static_cast<int>(ban->expiry);
		response["result"].push_back(b);
	}

	return response;
}

// Handle server_ban.add
json HandleServerBanAdd(const json& params)
{
	if (!params.contains("mask") || !params.contains("reason"))
		return CreateError("Missing parameters: mask, reason", JSON_RPC_ERROR_INVALID_PARAMS);

	std::string mask = params["mask"].get<std::string>();
	std::string reason = params["reason"].get<std::string>();
	time_t duration = params.contains("duration") ? params["duration"].get<int>() : 3600;

	XLine* ban = new GLine(ServerInstance->Time(), duration, "JSON-RPC", reason, mask, "*");

	if (!ServerInstance->XLines->AddLine(ban, nullptr))
	{
		delete ban;
		return CreateError("Ban already exists", JSON_RPC_ERROR_ALREADY_EXISTS);
	}

	return {{"result", "Ban added successfully"}};
}

// Handle server_ban.del
json HandleServerBanDel(const json& params)
{
	if (!params.contains("mask"))
		return CreateError("Missing parameter: mask", JSON_RPC_ERROR_INVALID_PARAMS);

	std::string mask = params["mask"].get<std::string>();
	std::string reason;

	if (!ServerInstance->XLines->DelLine(mask, "G", reason, nullptr))
		return CreateError("Ban not found", JSON_RPC_ERROR_NOT_FOUND);

	return {{"result", "Ban removed successfully"}};
}
