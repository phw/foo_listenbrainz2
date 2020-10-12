#include "stdafx.h"
#include "http_task.h"

#include <thread>

namespace lbz
{
	struct mbid
	{
		std::string primary, secondary, name;
	};

	static const std::vector<mbid> mbids =
	{
		{ "MUSICBRAINZ_ARTISTID", "MUSICBRAINZ ARTIST ID", "artist_mbids" },
		{ "MUSICBRAINZ_ALBUMID", "MUSICBRAINZ ALBUM ID", "release_mbid" },
		{ "MUSICBRAINZ_RELEASEGROUPID", "MUSICBRAINZ RELEASE GROUP ID", "release_group_mbid" },
		{ "MUSICBRAINZ_RELEASETRACKID", "MUSICBRAINZ RELEASE TRACK ID", "track_mbid" },
		{ "MUSICBRAINZ_TRACKID", "MUSICBRAINZ TRACK ID", "recording_mbid" },
		{ "MUSICBRAINZ_WORKID", "MUSICBRAINZ WORK ID", "work_mbids"},
	};

	static const std::vector<std::string> standard_tags =
	{
		"album artist",
		"date",
		"discnumber",
		"isrc",
		"totaldiscs",
		"totaltracks",
		"tracknumber",
	};

	class lbz_play_callback_static : public play_callback_static
	{
	public:
		uint32_t get_flags() override
		{
			return flag_on_playback_new_track | flag_on_playback_time;
		}

		void on_playback_new_track(metadb_handle_ptr handle) override
		{
			m_counter = 0;
			const double length = handle->get_length();
			if (length >= 5.0 && prefs::check_enabled.get_value())
			{
				const size_t half = static_cast<size_t>(std::round(length / 2));
				m_target = std::min<size_t>(half, 240U);
			}
			else
			{
				m_target = SIZE_MAX;
			}
		}

		void on_playback_time(double) override
		{
			if (m_target == SIZE_MAX) return;

			m_counter++;

			if (m_counter == 2)
			{
				submit(listen_type::playing_now);
			}
			else if (m_counter == m_target)
			{
				submit(listen_type::single);
			}
		}

		void on_playback_dynamic_info(const file_info&) override {}
		void on_playback_dynamic_info_track(const file_info&) override {}
		void on_playback_edited(metadb_handle_ptr) override {}
		void on_playback_pause(bool) override {}
		void on_playback_seek(double) override {}
		void on_playback_starting(play_control::t_track_command, bool) override {}
		void on_playback_stop(play_control::t_stop_reason) override {}
		void on_volume_change(float) override {}

	private:
		bool check_preferences(const metadb_handle_ptr& handle, listen_type type)
		{
			const bool report = type == listen_type::single;

			if (!prefs::check_enabled.get_value()) return false;

			if (!is_uuid(prefs::str_user_token))
			{
				if (report) spam("Token not set. Not submitting.");
				return false;
			}

			if (prefs::check_library.get_value() && !library_manager::get()->is_item_in_library(handle))
			{
				if (report) spam("Track not in Media Library. Not submitting.");
				return false;
			}

			if (prefs::check_skip.get_value() && !prefs::str_query.is_empty())
			{
				search_filter_v2::ptr filter;

				try
				{
					filter = search_filter_manager_v2::get()->create_ex(prefs::str_query.get_ptr(), fb2k::service_new<completion_notify_dummy>(), search_filter_manager_v2::KFlagSuppressNotify);
				}
				catch (...) {}

				if (filter.is_valid())
				{
					metadb_handle_list items = pfc::list_single_ref_t<metadb_handle_ptr>(handle);

					pfc::array_t<bool> mask;
					mask.set_size(items.get_count());
					filter->test_multi(items, mask.get_ptr());
					items.filter_mask(mask.get_ptr());

					if (items.get_count() == 1)
					{
						if (report) spam("Track matches skip query. Not submitting.");
						return false;
					}
				}
			}

			return true;
		}

		json get_additional_info(const file_info& info)
		{
			json additional_info = json::object();

			for (const std::string& name : standard_tags)
			{
				std::string value = get_tag(info, name);
				if (!value.empty())
				{
					if (name == "album artist")
					{
						additional_info["albumartist"] = value;
					}
					else
					{
						additional_info[name] = value;
					}
				}
			}

			for (const auto& [primary, secondary, name] : mbids)
			{
				if (name == "artist_mbids" || name == "work_mbids")
				{
					size_t idx = info.meta_find(primary.c_str());
					if (idx == SIZE_MAX) idx = info.meta_find(secondary.c_str());
					if (idx != SIZE_MAX)
					{
						const size_t count = info.meta_enum_value_count(idx);
						std::vector<std::string> values;
						for (size_t i = 0; i < count; ++i)
						{
							const char* value = info.meta_enum_value(idx, i);
							if (is_uuid(value))
							{
								values.emplace_back(value);
							}
						}
						if (values.size() > 0)
						{
							additional_info[name] = values;
						}
					}
				}
				else
				{
					const char* value = info.meta_get(primary.c_str(), 0);
					if (!is_uuid(value)) value = info.meta_get(secondary.c_str(), 0);
					if (is_uuid(value))
					{
						additional_info[name] = value;
					}
				}
			}

			additional_info["listening_from"] = "foobar2000";
			return additional_info;
		}

		std::string get_tag(const file_info& info, const std::string& name)
		{
			if (name == "artist" && !prefs::check_artist_first.get_value())
			{
				if (m_obj.is_empty())
				{
					titleformat_compiler::get()->compile_force(m_obj, "[%artist%]");
				}
				pfc::string8 artist;
				playback_control::get()->playback_format_title(nullptr, artist, m_obj, nullptr, playback_control::display_level_all);
				return artist.get_ptr();
			}
			else
			{
				const char* value = info.meta_get(name.c_str(), 0);
				if (value == nullptr) return std::string();
				return value;
			}
		}

		void submit(listen_type type)
		{
			metadb_handle_ptr handle;
			if (!playback_control::get()->get_now_playing(handle)) return;
			const file_info_impl info = handle->get_info_ref()->info();

			std::string artist = get_tag(info, "artist");
			std::string title = get_tag(info, "title");
			if (artist.empty() || title.empty()) return;

			if (!check_preferences(handle, type)) return;

			json j = json::object();
			j["payload"] = json::array();
			json track_metadata = json::object();

			track_metadata["artist_name"] = artist;
			track_metadata["track_name"] = title;

			if (type == listen_type::playing_now)
			{
				j["listen_type"] = "playing_now";
			}
			else if (type == listen_type::single)
			{
				j["listen_type"] = "single";
				j["payload"][0]["listened_at"] = pfc::fileTimeWtoU(pfc::fileTimeNow());

				const char* album = info.meta_get("album", 0);
				if (album != nullptr)
				{
					track_metadata["release_name"] = album;
				}

				track_metadata["additional_info"] = get_additional_info(info);
			}

			j["payload"][0]["track_metadata"] = track_metadata;

			auto t = std::thread([=]()
				{
					http_task task(type, j);
					task.run();
				});

			t.detach();
		}

		size_t m_counter = 0;
		size_t m_target = SIZE_MAX;
		titleformat_object::ptr m_obj;
	};

	FB2K_SERVICE_FACTORY(lbz_play_callback_static);
}
