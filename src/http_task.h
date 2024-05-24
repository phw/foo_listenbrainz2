#pragma once

namespace lbz
{
	class http_task
	{
	public:
		http_task(listen_type type, json data);

		void run();
        bool test_connection(); // Add the member function "test_connection"

	private:
		void cache();
		void process_response(json j);

		json m_data;
		listen_type m_listen_type;
	};

	void submit_cache();
	void submit_cache_async();
}
