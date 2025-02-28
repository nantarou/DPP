/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * Copyright 2021 Craig Edwards and D++ contributors 
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#include <dpp/discord.h>
#include <dpp/event.h>
#include <string>
#include <iostream>
#include <fstream>
#include <dpp/discordclient.h>
#include <dpp/discord.h>
#include <dpp/cache.h>
#include <dpp/stringops.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/discordevents.h>

using json = nlohmann::json;

namespace dpp { namespace events {

using namespace dpp;

/**
 * @brief Handle event
 * 
 * @param client Websocket client (current shard)
 * @param j JSON data for the event
 * @param raw Raw JSON string
 */
void message_reaction_remove_emoji::handle(discord_client* client, json &j, const std::string &raw) {
	if (!client->creator->on_message_reaction_remove_emoji.empty()) {
		json &d = j["d"];
		dpp::message_reaction_remove_emoji_t mrre(client, raw);
		mrre.reacting_guild = dpp::find_guild(snowflake_not_null(&d, "guild_id"));
		mrre.reacting_channel = dpp::find_channel(snowflake_not_null(&d, "channel_id"));
		mrre.message_id = snowflake_not_null(&d, "message_id");
		mrre.reacting_emoji = dpp::emoji().fill_from_json(&(d["emoji"]));
		if (mrre.reacting_channel && mrre.message_id) {
			client->creator->on_message_reaction_remove_emoji.call(mrre);
		}
	}

}

}};