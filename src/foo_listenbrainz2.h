#pragma once

namespace lbz
{
	static constexpr const char* component_name = "ListenBrainz 2";
	static constexpr const char* component_dll_name = "foo_listenbrainz2.dll";
	static constexpr const char* component_version = "1.1.1";
	static constexpr const char* component_info = "Copyright (C) 2020 marc2003\n\nBuild: " __TIME__ ", " __DATE__;
	static constexpr size_t cache_max = 50;

	enum class listen_type
	{
		playing_now,
		single,
		import
	};

	static void spam(pfc::stringp str)
	{
		FB2K_console_formatter() << component_name << ": " << str;
	}
}
