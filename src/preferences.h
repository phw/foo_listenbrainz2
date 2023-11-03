#pragma once
#include "cache.h"

namespace lbz
{
	namespace prefs
	{
		extern cfg_bool check_enabled;
		extern cfg_bool check_artist_first;
		extern cfg_bool check_library;
		extern cfg_bool check_skip;
		extern cfg_bool check_client_details;

		extern cfg_string str_user_token;
		extern cfg_string str_api_url;
		extern cfg_string str_query;

		extern listen_cache cache;

		// Legacy cache configuration, only used for migrations
		extern cfg_string str_cache;
	}
}
