
#include "jsonrpc.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Handle channel.list
json HandleChannelList()
{
	json response;
	response["jsonrpc"] = "2.0";
	response["result"] = json::array();

	for (const auto& [_, channel] : ServerInstance->Channels.GetChans())
	{
		json ch;
		ch["name"] = channel->name;
		ch["num_users"] = static_cast<int>(channel->GetUsers().size());
		ch["topic"] = channel->topic;
		ch["modes"] = channel->ChanModes(true);
		response["result"].push_back(ch);
	}

	return response;
}

// Handle channel.get
json HandleChannelGet(const json& params)
{
	if (!params.contains("channel"))
		return CreateError("Missing parameter: channel", JSON_RPC_ERROR_INVALID_PARAMS);

	std::string channel_name = params["channel"].get<std::string>();
	Channel* channel = ServerInstance->Channels.Find(channel_name);
	if (!channel)
		return CreateError("Channel not found", JSON_RPC_ERROR_NOT_FOUND);

	json obj;
	obj["name"] = channel->name;
	obj["num_users"] = static_cast<int>(channel->GetUsers().size());
	obj["topic"] = channel->topic;
	obj["modes"] = channel->ChanModes(true);

	return obj;
}

// Handle channel.set_mode
json HandleChannelSetMode(const json& params)
{
	if (!params.contains("channel") || !params.contains("mode") || !params.contains("param"))
		return CreateError("Missing parameters: channel, mode, param", JSON_RPC_ERROR_INVALID_PARAMS);

	std::string channel_name = params["channel"].get<std::string>();
	std::string mode = params["mode"].get<std::string>();
	std::string mode_param = params["param"].get<std::string>();

	Channel* channel = ServerInstance->Channels.Find(channel_name);
	if (!channel)
		return CreateError("Channel not found", JSON_RPC_ERROR_NOT_FOUND);

	ModeHandler* mh = ServerInstance->Modes.FindMode(mode, MODETYPE_CHANNEL);
	if (!mh)
		return CreateError("Invalid mode", JSON_RPC_ERROR_INVALID_PARAMS);

	Modes::ChangeList changelist;
	changelist.push_add(mh, mode_param);
	ServerInstance->Modes.ProcessSingle(nullptr, channel, nullptr, changelist, ModeParser::MODE_LOCALONLY);

	return {{"result", "Mode set successfully"}};
}

json HandleChannelKick(const json& params)
{
    if (!params.contains("channel") || !params.contains("nick") || !params.contains("reason"))
        return CreateError("Missing parameters: channel, nick, reason", JSON_RPC_ERROR_INVALID_PARAMS);

    std::string channel_name = params["channel"].get<std::string>();
    std::string nick = params["nick"].get<std::string>();
    std::string reason = params["reason"].get<std::string>();

    Channel* channel = ServerInstance->Channels.Find(channel_name);
    if (!channel)
        return CreateError("Channel not found", JSON_RPC_ERROR_NOT_FOUND);

    User* user = ServerInstance->Users.FindNick(nick);
    if (!user)
        return CreateError("User not found", JSON_RPC_ERROR_NOT_FOUND);

    // Use the FakeClient to represent the server itself.
    User* server_user = ServerInstance->FakeClient;

    // Perform the kick with the FakeClient acting as the server
    if (!channel->KickUser(server_user, user, reason))
        return CreateError("Failed to kick user", JSON_RPC_ERROR_DENIED);

    return {{"result", "User kicked successfully by server"}};
}
