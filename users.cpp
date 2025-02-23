#include "jsonrpc.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Helper function to find a user by nickname
User* FindUserByNick(const std::string& nick)
{
	return ServerInstance->Users.FindNick(nick);
}

// Handle user.list
json HandleUserList()
{
	json result = json::array();

	for (const auto& [uuid, user] : ServerInstance->Users.GetUsers())
	{
		json obj;
		obj["nick"] = user->nick;
		obj["uuid"] = user->uuid;
		obj["realname"] = user->GetRealName();
		obj["host"] = user->GetDisplayedHost();
		obj["ip"] = user->GetAddress();
		obj["oper"] = user->IsOper();
		obj["away"] = user->IsAway();
		result.push_back(obj);
	}

	return result;
}

// Handle user.get
json HandleUserGet(const json& params)
{
	if (!params.contains("nick"))
		return CreateError("Missing parameter: nick", JSON_RPC_ERROR_INVALID_PARAMS);

	std::string nick = params["nick"].get<std::string>();
	User* user = FindUserByNick(nick);
	if (!user)
		return CreateError("User not found", JSON_RPC_ERROR_NOT_FOUND);

	json obj;
	obj["nick"] = user->nick;
	obj["uuid"] = user->uuid;
	obj["realname"] = user->GetRealName();
	obj["host"] = user->GetDisplayedHost();
	obj["ip"] = user->GetAddress();
	obj["oper"] = user->IsOper();
	obj["away"] = user->IsAway();
	return obj;
}

// Handle user.set_nick
json HandleUserSetNick(const json& params)
{
	if (!params.contains("nick") || !params.contains("newnick"))
		return CreateError("Missing parameters: nick, newnick", JSON_RPC_ERROR_INVALID_PARAMS);

	std::string old_nick = params["nick"].get<std::string>();
	std::string new_nick = params["newnick"].get<std::string>();

	User* user = FindUserByNick(old_nick);
	if (!user)
		return CreateError("User not found", JSON_RPC_ERROR_NOT_FOUND);

	if (!user->ChangeNick(new_nick))
		return CreateError("Nickname change failed", JSON_RPC_ERROR_DENIED);

	return {{"result", "Nickname changed successfully"}};
}

// Handle user.set_realname
json HandleUserSetRealname(const json& params)
{
	if (!params.contains("nick") || !params.contains("realname"))
		return CreateError("Missing parameters: nick, realname", JSON_RPC_ERROR_INVALID_PARAMS);

	std::string nick = params["nick"].get<std::string>();
	std::string realname = params["realname"].get<std::string>();

	User* user = FindUserByNick(nick);
	if (!user)
		return CreateError("User not found", JSON_RPC_ERROR_NOT_FOUND);

	user->ChangeRealName(realname);

	return {{"result", "Real name changed successfully"}};
}

// Handle user.set_vhost
json HandleUserSetVhost(const json& params)
{
	if (!params.contains("nick") || !params.contains("vhost"))
		return CreateError("Missing parameters: nick, vhost", JSON_RPC_ERROR_INVALID_PARAMS);

	std::string nick = params["nick"].get<std::string>();
	std::string vhost = params["vhost"].get<std::string>();

	User* user = FindUserByNick(nick);
	if (!user)
		return CreateError("User not found", JSON_RPC_ERROR_NOT_FOUND);

	user->ChangeDisplayedHost(vhost);

	return {{"result", "Virtual host changed successfully"}};
}

// Handle user.set_mode
json HandleUserSetMode(const json& params)
{
	if (!params.contains("nick") || !params.contains("modes"))
		return CreateError("Missing parameters: nick, modes", JSON_RPC_ERROR_INVALID_PARAMS);

	std::string nick = params["nick"].get<std::string>();
	std::string modes = params["modes"].get<std::string>();

	User* user = FindUserByNick(nick);
	if (!user)
		return CreateError("User not found", JSON_RPC_ERROR_NOT_FOUND);

	Modes::ChangeList changelist;
	ServerInstance->Modes.ModeParamsToChangeList(user, MODETYPE_USER, {modes}, changelist);
	ServerInstance->Modes.Process(user, nullptr, nullptr, changelist, ModeParser::MODE_LOCALONLY);

	return {{"result", "User mode changed successfully"}};
}
