#include "stdafx.h"
#include "http_task.h"

namespace lbz
{
	http_task::http_task(listen_type type, json data) : m_listen_type(type), m_data(data) {}

	json http_task::get_cache()
	{
		json cache = json::parse(prefs::str_cache.get_ptr(), nullptr, false);
		return cache.is_array() ? cache : json::array();
	}

	void http_task::cache()
	{
		json cache = get_cache();
		cache.emplace_back(m_data["payload"][0]);
		spam(PFC_string_formatter() << "The cache now contains " << cache.size() << " listen(s)");
		prefs::str_cache = cache.dump().c_str();
	}

	void http_task::process_response(json j)
	{
		auto status = j["status"];
		auto code = j["code"];
		auto error = j["error"];
		const bool ok = status.is_string() && status.get_ref<const std::string&>() == "ok";
		const bool bad = code.is_number_unsigned() && code.get<size_t>() == 400U;

		if (m_listen_type == listen_type::single)
		{
			if (ok)
			{
				spam("Listen submitted OK!");
				submit_cache();
			}
			else if (bad)
			{
				spam("Server responded with error code 400 meaning submission was not accepted and it cannot be retried because it was malformed.");
			}
			else
			{
				if (error.is_string()) spam(error.get_ref<const std::string&>().c_str());
				else spam(j.dump().c_str());
				cache();
			}
		}
		else if (m_listen_type == listen_type::import)
		{
			if (ok)
			{
				spam("Cached listens submitted OK!");

				json cache = get_cache();
				if (cache.size() <= cache_max)
				{
					reset_cache();
				}
				else
				{
					json tmp(cache.begin() + cache_max, cache.end());
					prefs::str_cache = tmp.dump().c_str();
					submit_cache();
				}
			}
			else if (bad)
			{
				spam("Submission of the cache was refused because one or more 'listens' were malformed. The cache will now be reset.");
				reset_cache();
			}
		}
	}

	void http_task::reset_cache()
	{
		prefs::str_cache = "[]";
	}

	void http_task::run()
	{
		pfc::string8_fast buffer;
		std::string str = m_data.dump();

		http_request_post_v2::ptr request;
		http_client::get()->create_request("POST")->cast(request);
		request->add_header("Authorization", PFC_string_formatter() << "Token " << prefs::str_user_token);
		request->set_post_data(reinterpret_cast<const void*>(str.c_str()), str.length(), "application/json");

		try
		{
			auto response = request->run_ex(api_url, fb2k::noAbort);
			response->read_string_raw(buffer, fb2k::noAbort);

			json j = json::parse(buffer.get_ptr(), nullptr, false);
			if (j.is_object())
			{
				process_response(j);
				return;
			}

			http_reply::ptr ptr;
			if (response->cast(ptr))
			{
				ptr->get_status(buffer);
			}
		}
		catch (const std::exception& e)
		{
			buffer = e.what();
		}

		if (m_listen_type != listen_type::playing_now)
		{
			spam(buffer);
			if (m_listen_type == listen_type::single) cache();
		}
	}

	void http_task::submit_cache()
	{
		json cache = get_cache();
		if (cache.empty()) return;

		if (cache.size() > cache_max)
		{
			cache.get_ref<json::array_t&>().resize(cache_max);
		}

		json j;
		j["listen_type"] = "import";
		j["payload"] = cache;

		spam(PFC_string_formatter() << "Now submitting " << cache.size() << " listen(s) from the cache.");

		http_task task(listen_type::import, j);
		task.run();
	}
}
