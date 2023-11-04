#pragma once
#include "cache.h"

namespace lbz
{
	namespace prefs
	{
		namespace defaults
		{
			static constexpr bool check_enabled = false;
			static constexpr bool check_artist_first = true;
			static constexpr bool check_library = false;
			static constexpr bool check_skip = false;
			static constexpr bool check_client_details = true;

			static constexpr const char* str_user_token = "";
			static constexpr const char* str_api_url = "https://api.listenbrainz.org/1/submit-listens";
			static constexpr const char* str_query = "%genre% IS k-pop";

			static constexpr const char* str_cache = "null";
		}

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

	static const char* get_api_url()
	{
		if (strlen(prefs::str_api_url.get_ptr()) == 0)
		{
			return prefs::defaults::str_api_url;
		}
		else
		{
			return prefs::str_api_url;
		}
	}

	// Returns true, if the API URL is the default ListenBrainz service.
	static bool is_listenbrainz()
	{
		const char* api_url = prefs::str_api_url.get_ptr();
		if (strlen(api_url) == 0
			|| strcmp(api_url, prefs::defaults::str_api_url) == 0)
		{
			return true;
		}

		return false;
	}

	static bool is_valid_token()
	{
		// empty token
		if (prefs::str_user_token.get_ptr() == nullptr) return false;
		// Default ListenBrainz service
		if (is_listenbrainz()) return is_uuid(prefs::str_user_token.get_ptr());
		// Custom server
		if (strlen(prefs::str_user_token.get_ptr()) > 0) return true;

		return false;
	}
}
