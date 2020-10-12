#include "stdafx.h"

namespace lbz
{
	namespace prefs
	{
		namespace defaults
		{
			static constexpr bool check_enabled = false;
			static constexpr bool check_artist_first = true;
			static constexpr bool check_library = false;
			static constexpr bool check_skip = false;

			static constexpr const char* str_user_token = "";
			static constexpr const char* str_query = "%genre% IS k-pop";

			static constexpr const char* str_cache = "[]"; // empty JSON array
		}

		cfg_bool check_enabled(guids::check_enabled, defaults::check_enabled);
		cfg_bool check_artist_first(guids::check_artist_first, defaults::check_artist_first);
		cfg_bool check_library(guids::check_library, defaults::check_library);
		cfg_bool check_skip(guids::check_skip, defaults::check_skip);

		cfg_string str_user_token(guids::str_user_token, defaults::str_user_token);
		cfg_string str_query(guids::str_query, defaults::str_query);

		cfg_string str_cache(guids::str_cache, defaults::str_cache);
	}

	class lbz_preferences_page_instance : public CDialogImpl<lbz_preferences_page_instance>, public preferences_page_instance
	{
	public:
		lbz_preferences_page_instance(preferences_page_callback::ptr callback) : m_callback(callback) {}

		BEGIN_MSG_MAP_EX(lbz_preferences_page_instance)
			MSG_WM_INITDIALOG(OnInitDialog)
			COMMAND_RANGE_HANDLER_EX(IDC_CHECK_ENABLED, IDC_EDIT_QUERY, OnChanged)
		END_MSG_MAP()

		enum { IDD = IDD_PREFERENCES };

		BOOL OnInitDialog(CWindow, LPARAM)
		{
			const bool enabled = prefs::check_enabled.get_value();

			m_check_enabled = GetDlgItem(IDC_CHECK_ENABLED);
			m_check_enabled.SetCheck(enabled);

			m_check_artist_first = GetDlgItem(IDC_CHECK_ARTIST_FIRST);
			m_check_artist_first.SetCheck(prefs::check_artist_first.get_value());
			m_check_artist_first.EnableWindow(enabled);

			m_check_library = GetDlgItem(IDC_CHECK_LIBRARY);
			m_check_library.SetCheck(prefs::check_library.get_value());
			m_check_library.EnableWindow(enabled);

			m_check_skip = GetDlgItem(IDC_CHECK_SKIP);
			m_check_skip.SetCheck(prefs::check_skip.get_value());
			m_check_skip.EnableWindow(enabled);

			m_edit_user_token = GetDlgItem(IDC_EDIT_USER_TOKEN);
			pfc::setWindowText(m_edit_user_token, prefs::str_user_token);
			m_edit_user_token.EnableWindow(enabled);

			m_edit_query = GetDlgItem(IDC_EDIT_QUERY);
			pfc::setWindowText(m_edit_query, prefs::str_query);
			m_edit_query.EnableWindow(enabled && prefs::check_skip.get_value());

			return FALSE;
		}

		uint32_t get_state() override
		{
			auto has_changed = [&]()
			{
				if (m_check_enabled.IsChecked() != prefs::check_enabled.get_value()) return true;
				if (m_check_artist_first.IsChecked() != prefs::check_artist_first.get_value()) return true;
				if (m_check_library.IsChecked() != prefs::check_library.get_value()) return true;
				if (m_check_skip.IsChecked() != prefs::check_skip.get_value()) return true;
				if (!pfc::getWindowText(m_edit_user_token).equals(prefs::str_user_token)) return true;
				if (!pfc::getWindowText(m_edit_query).equals(prefs::str_query)) return true;
				return false;
			}();

			uint32_t state = preferences_state::resettable;
			if (has_changed) state |= preferences_state::changed;
			return state;
		}

		void apply() override
		{
			prefs::check_enabled = m_check_enabled.IsChecked();
			prefs::check_artist_first = m_check_artist_first.IsChecked();
			prefs::check_library = m_check_library.IsChecked();
			prefs::check_skip = m_check_skip.IsChecked();
			prefs::str_user_token = pfc::getWindowText(m_edit_user_token);
			prefs::str_query = pfc::getWindowText(m_edit_query);
		}

		void on_change()
		{
			const bool enabled = m_check_enabled.IsChecked();
			m_check_artist_first.EnableWindow(enabled);
			m_check_library.EnableWindow(enabled);
			m_check_skip.EnableWindow(enabled);
			m_edit_user_token.EnableWindow(enabled);
			m_edit_query.EnableWindow(enabled && m_check_skip.IsChecked());

			m_callback->on_state_changed();
		}

		void reset() override
		{
			m_check_enabled.SetCheck(prefs::defaults::check_enabled);
			m_check_artist_first.SetCheck(prefs::defaults::check_artist_first);
			m_check_library.SetCheck(prefs::defaults::check_library);
			m_check_skip.SetCheck(prefs::defaults::check_skip);
			pfc::setWindowText(m_edit_user_token, prefs::defaults::str_user_token);
			pfc::setWindowText(m_edit_query, prefs::defaults::str_query);

			on_change();
		}

		void OnChanged(UINT, int, CWindow)
		{
			on_change();
		}

	private:
		CCheckBox m_check_enabled, m_check_library, m_check_skip, m_check_artist_first;
		CEdit m_edit_user_token, m_edit_query;
		preferences_page_callback::ptr m_callback;
	};

	class lbz_preferences_page_impl : public preferences_page_impl<lbz_preferences_page_instance>
	{
	public:
		GUID get_guid() override
		{
			return guids::preferences_page;
		}

		GUID get_parent_guid() override
		{
			return preferences_page::guid_tools;
		}

		bool get_help_url(pfc::string_base& out) override
		{
			out.set_string("https://github.com/EliasAlucard/foo_listenbrainz2");
			return true;
		}

		const char* get_name() override
		{
			return component_name;
		}
	};

	FB2K_SERVICE_FACTORY(lbz_preferences_page_impl);
}
