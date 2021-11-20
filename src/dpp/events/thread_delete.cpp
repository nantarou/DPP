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
#include <dpp/discordevents.h>
#include <dpp/discord.h>
#include <dpp/cache.h>
#include <dpp/stringops.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/fmt/format.h>

using json = nlohmann::json;

namespace dpp { namespace events {

using namespace dpp;
void thread_delete::handle(discord_client* client, json& j, const std::string& raw) {
	json& d = j["d"];

	dpp::channel* c = dpp::find_channel(SnowflakeNotNull(&d, "id"));
	if (c) {
		dpp::guild* g = dpp::find_guild(c->guild_id);
		if (g) {
			auto gt = std::find(g->threads.begin(), g->threads.end(), c->id);
			if (gt != g->threads.end()) {
				g->threads.erase(gt);
			}
			if (!client->creator->dispatch.thread_delete.empty()) {
				dpp::thread_delete_t td(client, raw);
				td.deleted = *static_cast<dpp::thread*>(c);
				td.deleting_guild = g;
				call_event(client->creator->dispatch.thread_delete, td);
			}
		}
		dpp::get_channel_cache()->remove(c);
	}
}
}};
