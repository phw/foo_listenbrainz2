#include "stdafx.h"

namespace lbz
{
	class lbz_mainmenu_commands: public mainmenu_commands
	{
	public:
		GUID get_command(size_t index) override
		{
			if (index == 0) return guids::mainmenu_command;
			else uBugCheck();
		}

		GUID get_parent() override
		{
			return mainmenu_groups::playback;
		}

		bool get_description(size_t index, pfc::string_base& out) override
		{
			out = "Toggle ListenBrainz submissions on/off";
			return true;
		}

		bool get_display(size_t index, pfc::string_base& out, size_t& flags) override
		{
			if (!is_uuid(prefs::str_user_token.get_ptr())) flags = flag_disabled;
			else if (prefs::check_enabled.get_value()) flags = flag_checked;
			get_name(index, out);
			return true;
		}

		size_t get_command_count() override
		{
			return 1;
		}

		void execute(size_t index, service_ptr_t<service_base> callback) override
		{
			if (is_uuid(prefs::str_user_token.get_ptr()))
			{
				prefs::check_enabled = !prefs::check_enabled.get_value();
			}
		}

		void get_name(size_t index, pfc::string_base& out) override
		{
			out = "Submit to ListenBrainz";
		}
	};

	FB2K_SERVICE_FACTORY(lbz_mainmenu_commands);
}
