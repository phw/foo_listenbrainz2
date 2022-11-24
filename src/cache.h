#pragma once
#include "stdafx.h"

namespace lbz
{
	class listen_cache : cfg_var
	{
	public:
		listen_cache(GUID const& guid);
		void add(json data);
		json get_batch(size_t count);
		void drop(size_t count);
		void reset();
		bool empty();
		size_t size();

	private:
		void get_data_raw(stream_writer* stream, abort_callback& abort) override;
		void set_data_raw(stream_reader* stream, t_size sizehint, abort_callback& abort) override;
		void set_default();

		json m_cache;
		pfc::readWriteLock m_lock;
	};
}
