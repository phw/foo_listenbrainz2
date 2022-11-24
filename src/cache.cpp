#include "stdafx.h"
#include "cache.h"

namespace lbz
{
	listen_cache::listen_cache(GUID const& guid) : cfg_var(guid) {}

	void listen_cache::add(json data)
	{
		PFC_INSYNC_WRITE(m_lock);
		m_cache.emplace_back(data);
	}

	json listen_cache::get_batch(size_t count)
	{
		PFC_INSYNC_READ(m_lock);
		if (m_cache.size() > count)
		{
			return json(m_cache.begin(), m_cache.begin() + count);
		}
		else
		{
			return json(m_cache);
		}
	}

	void listen_cache::drop(size_t count)
	{
		PFC_INSYNC_WRITE(m_lock);
		m_cache = json(m_cache.begin() + count, m_cache.end());
	}

	void listen_cache::reset()
	{
		PFC_INSYNC_WRITE(m_lock);
		m_cache = json::array();
	}

	bool listen_cache::empty() {
		PFC_INSYNC_READ(m_lock);
		return m_cache.empty();
	}

	size_t listen_cache::size() {
		PFC_INSYNC_READ(m_lock);
		return m_cache.size();
	}

	void listen_cache::get_data_raw(stream_writer* stream, abort_callback& abort)
	{
		PFC_INSYNC_READ(m_lock);
		stream->write_string(m_cache.dump(), abort);
	}

	void listen_cache::set_data_raw(stream_reader* stream, t_size sizehint, abort_callback& abort)
	{
		json cache = json::parse(stream->read_string(abort).get_ptr(), nullptr, false);
		if (cache.is_array())
		{
			PFC_INSYNC_WRITE(m_lock);
			m_cache = cache;
			spam(PFC_string_formatter() << "Loaded cache with " << m_cache.size() << " listen(s)");
		}
		else
		{
			set_default();
		}
	}

	void listen_cache::set_default()
	{
		json legacy_cache = json::parse(prefs::str_cache.get_ptr(), nullptr, false);
		prefs::str_cache = "null";  // Reset the legacy cache
		PFC_INSYNC_WRITE(m_lock);
		if (legacy_cache.is_array())
		{
			m_cache = legacy_cache;
			spam(PFC_string_formatter() << "Loaded " << m_cache.size() << " listen(s) from legacy cache");
		}
		else
		{
			m_cache = json::array();
			spam(PFC_string_formatter() << "Initialized empty listen cache");
		}
	}
}
