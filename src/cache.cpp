#include "stdafx.h"
#include "cache.h"

namespace lbz
{
	listen_cache::listen_cache(GUID const& guid) : cfg_var(guid) {}

	void listen_cache::add(json data)
	{
		m_lock.enterWrite();
		m_cache.emplace_back(data);
		m_lock.leaveWrite();
	}

	json listen_cache::get_batch(size_t count)
	{
		m_lock.enterRead();
		if (m_cache.size() > count)
		{
			return json(m_cache.begin(), m_cache.begin() + count);
		}
		else
		{
			return json(m_cache);
		}

		m_lock.leaveRead();
	}

	void listen_cache::drop(size_t count)
	{
		m_lock.enterWrite();
		m_cache = json(m_cache.begin() + count, m_cache.end());
		m_lock.leaveWrite();
	}

	void listen_cache::reset()
	{
		m_lock.enterWrite();
		m_cache = json::array();
		m_lock.leaveWrite();
	}

	bool listen_cache::empty() {
		return m_cache.empty();
	}

	size_t listen_cache::size() {
		return m_cache.size();
	}

	void listen_cache::get_data_raw(stream_writer* stream, abort_callback& abort)
	{
		m_lock.enterRead();
		stream->write_string(m_cache.dump(), abort);
		m_lock.leaveRead();
	}

	void listen_cache::set_data_raw(stream_reader* stream, t_size sizehint, abort_callback& abort)
	{
		json cache = json::parse(stream->read_string(abort).get_ptr(), nullptr, false);
		if (cache.is_array())
		{
			m_lock.enterWrite();
			m_cache = cache;
			m_lock.leaveWrite();
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
		m_lock.enterWrite();
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

		m_lock.leaveWrite();
	}
}
