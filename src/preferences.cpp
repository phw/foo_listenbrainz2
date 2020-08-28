#include "stdafx.h"

namespace guids
{
	static constexpr GUID preferences_page = { 0x3c92ef3b, 0xd3d7, 0x43e0, { 0xa5, 0xe0, 0x7, 0xf8, 0x14, 0xa0, 0x55, 0xfb } };

	static constexpr GUID check_enabled = { 0xf2a8a44c, 0x8a82, 0x4cf5, { 0xa3, 0x4f, 0x41, 0xef, 0xbe, 0xd8, 0x2d, 0x8e } };
	static constexpr GUID check_library = { 0x9175b5e1, 0xeea1, 0x44b2, { 0x99, 0xc3, 0x45, 0x0, 0x9d, 0xff, 0x9f, 0x7b } };

	static constexpr GUID str_user_token = { 0x3d9164f6, 0xb3b0, 0x403f, { 0xb1, 0x42, 0x44, 0x7c, 0x5d, 0xdf, 0x4f, 0x15 } };
	static constexpr GUID str_cache = { 0x1b2db62, 0x2d12, 0x4fcb, { 0xbb, 0xc3, 0x87, 0x2f, 0xd4, 0x28, 0xf7, 0x2d } };
}

namespace prefs
{
	namespace defaults
	{
		static constexpr bool check_enabled = false;
		static constexpr bool check_library = false;

		static constexpr const char* str_user_token = "";
		static constexpr const char* str_cache = "[]"; // empty JSON array
	}

	cfg_bool check_enabled(guids::check_enabled, defaults::check_enabled);
	cfg_bool check_library(guids::check_library, defaults::check_library);

	cfg_string str_user_token(guids::str_user_token, defaults::str_user_token);
	cfg_string str_cache(guids::str_cache, defaults::str_cache);
}

namespace lbz
{
	class lbz_preferences_page_instance : public CDialogImpl<lbz_preferences_page_instance>, public preferences_page_instance
	{
	public:
		lbz_preferences_page_instance(preferences_page_callback::ptr callback) : m_callback(callback) {}

		BEGIN_MSG_MAP_EX(lbz_preferences_page_instance)
			MSG_WM_INITDIALOG(OnInitDialog)
			COMMAND_RANGE_HANDLER_EX(IDC_CHECK_ENABLED, IDC_EDIT_USER_TOKEN, OnChanged)
		END_MSG_MAP()

		enum { IDD = IDD_PREFERENCES };

		BOOL OnInitDialog(CWindow, LPARAM)
		{
			const bool enabled = prefs::check_enabled.get_value();

			m_check_enabled = GetDlgItem(IDC_CHECK_ENABLED);
			m_check_enabled.SetCheck(enabled);

			m_check_library = GetDlgItem(IDC_CHECK_LIBRARY);
			m_check_library.SetCheck(prefs::check_library.get_value());
			m_check_library.EnableWindow(enabled);

			m_edit_user_token = GetDlgItem(IDC_EDIT_USER_TOKEN);
			uSetWindowText(m_edit_user_token, prefs::str_user_token);
			m_edit_user_token.EnableWindow(enabled);

			return FALSE;
		}

		bool has_changed()
		{
			if (m_check_enabled.IsChecked() != prefs::check_enabled.get_value()) return true;
			if (m_check_library.IsChecked() != prefs::check_library.get_value()) return true;

			pfc::string8_fast temp;
			uGetWindowText(m_edit_user_token, temp);
			if (prefs::str_user_token != temp) return true;

			return false;
		}

		uint32_t get_state() override
		{
			uint32_t state = preferences_state::resettable;
			if (has_changed()) state |= preferences_state::changed;
			return state;
		}

		void apply() override
		{
			prefs::check_enabled = m_check_enabled.IsChecked();
			prefs::check_library = m_check_library.IsChecked();
			uGetWindowText(m_edit_user_token, prefs::str_user_token);
		}

		void on_change()
		{
			const bool enabled = m_check_enabled.IsChecked();
			m_check_library.EnableWindow(enabled);
			m_edit_user_token.EnableWindow(enabled);

			m_callback->on_state_changed();
		}

		void reset() override
		{
			m_check_enabled.SetCheck(prefs::defaults::check_enabled);
			m_check_library.SetCheck(prefs::defaults::check_library);
			uSetWindowText(m_edit_user_token, prefs::defaults::str_user_token);

			on_change();
		}

		void OnChanged(UINT, int, CWindow)
		{
			on_change();
		}

	private:
		CCheckBox m_check_enabled;
		CCheckBox m_check_library;
		CEdit m_edit_user_token;
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

		const char* get_name() override
		{
			return component_name;
		}
	};

	FB2K_SERVICE_FACTORY(lbz_preferences_page_impl);
}
