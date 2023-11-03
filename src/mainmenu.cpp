#include "stdafx.h"

namespace lbz
{
	class lbz_mainmenu_commands: public mainmenu_commands
	{
	public:
		GUID get_command(t_uint32 index) override
		{
			if (index == 0) return guids::mainmenu_command;
			else uBugCheck();
		}

		GUID get_parent() override
		{
			return mainmenu_groups::playback;
		}

		bool get_description(t_uint32 index, pfc::string_base& out) override
		{
			out = "Toggle ListenBrainz submissions on/off";
			return true;
		}

		bool get_display(t_uint32 index, pfc::string_base& out, t_uint32& flags) override
		{
			if (!is_valid_token(prefs::str_user_token.get_ptr())) flags = flag_disabled;
			else if (prefs::check_enabled.get_value()) flags = flag_checked;
			get_name(index, out);
			return true;
		}

		t_uint32 get_command_count() override
		{
			return 1;
		}

		void execute(t_uint32 index, service_ptr_t<service_base> callback) override
		{
			if (is_valid_token(prefs::str_user_token.get_ptr()))
			{
				prefs::check_enabled = !prefs::check_enabled.get_value();
			}
		}

		void get_name(t_uint32 index, pfc::string_base& out) override
		{
			if (strcmp(prefs::str_api_url.get_ptr(), "https://api.listenbrainz.org/1/submit-listens") == 0) {
				out = "Submit to ListenBrainz";
			} else {
				out = "Submit to Scrobble Server";
			}
		}
	};

	FB2K_SERVICE_FACTORY(lbz_mainmenu_commands);
}
