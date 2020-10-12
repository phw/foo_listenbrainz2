#pragma once

namespace lbz
{
	class http_task
	{
	public:
		http_task(listen_type type, json data);

		void run();

	private:
		json get_cache();
		void cache();
		void process_response(json j);
		void reset_cache();
		void submit_cache();

		json m_data;
		listen_type m_listen_type;
	};
}
