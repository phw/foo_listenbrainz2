#pragma once

namespace lbz
{
	class http_task
	{
	public:
		http_task(listen_type type, json data);

		void run();

	private:
		void cache();
		void process_response(json j);
		void submit_cache();

		json m_data;
		listen_type m_listen_type;
	};
}
