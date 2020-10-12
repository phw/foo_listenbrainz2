#pragma once
#include <regex>
using std::regex;

namespace lbz
{
	static constexpr const char* component_name = "ListenBrainz 2";
	static constexpr const char* component_dll_name = "foo_listenbrainz2.dll";
	static constexpr const char* component_version = "1.1.3";
	static constexpr const char* component_info = "Copyright (C) 2020 marc2003\n\nBuild: " __TIME__ ", " __DATE__;
	static constexpr const char* api_url = "https://api.listenbrainz.org/1/submit-listens";
	static constexpr size_t cache_max = 50;

	namespace guids
	{
		static constexpr GUID mainmenu_command = { 0x5fc6dbfb, 0xbf19, 0x474d, { 0x8d, 0xad, 0x97, 0x66, 0x10, 0x9e, 0x52, 0x7d } };

		static constexpr GUID preferences_page = { 0x3c92ef3b, 0xd3d7, 0x43e0, { 0xa5, 0xe0, 0x7, 0xf8, 0x14, 0xa0, 0x55, 0xfb } };

		static constexpr GUID check_enabled = { 0xf2a8a44c, 0x8a82, 0x4cf5, { 0xa3, 0x4f, 0x41, 0xef, 0xbe, 0xd8, 0x2d, 0x8e } };
		static constexpr GUID check_artist_first = { 0xc9f4c44d, 0x799b, 0x40f6, { 0xb2, 0xbd, 0x4b, 0x53, 0xb4, 0xea, 0x16, 0x4c } };
		static constexpr GUID check_library = { 0x9175b5e1, 0xeea1, 0x44b2, { 0x99, 0xc3, 0x45, 0x0, 0x9d, 0xff, 0x9f, 0x7b } };
		static constexpr GUID check_skip = { 0x1c79e372, 0x4304, 0x45f4, { 0x81, 0xab, 0x72, 0x97, 0x49, 0xce, 0x5e, 0xed } };

		static constexpr GUID str_user_token = { 0x3d9164f6, 0xb3b0, 0x403f, { 0xb1, 0x42, 0x44, 0x7c, 0x5d, 0xdf, 0x4f, 0x15 } };
		static constexpr GUID str_query = { 0xc026551f, 0xfa92, 0x4917, { 0xa5, 0xbe, 0x28, 0x4f, 0x9b, 0x6d, 0x80, 0xdf } };

		static constexpr GUID str_cache = { 0x1b2db62, 0x2d12, 0x4fcb, { 0xbb, 0xc3, 0x87, 0x2f, 0xd4, 0x28, 0xf7, 0x2d } };
	}

	enum class listen_type
	{
		playing_now,
		single,
		import
	};

	static bool is_uuid(const char* mbid)
	{
		if (mbid == nullptr) return false;
		regex rx("^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$");
		return regex_search(mbid, rx);
	}

	static void spam(pfc::stringp str)
	{
		FB2K_console_formatter() << component_name << ": " << str;
	}
}
