#include "game_loop.h"

#if defined(__WINOS__)

namespace gb
{
	class game_loop_win32 : public game_loop
	{
	private:

	protected:

		static std::shared_ptr<game_loop> m_shared_instance;

	public:

		game_loop_win32() = default;
		~game_loop_win32() = default;

		static std::shared_ptr<game_loop> shared_instance();
	};

	std::shared_ptr<game_loop> game_loop_win32::m_shared_instance = nullptr;

	std::shared_ptr<game_loop> game_loop_win32::shared_instance()
	{
		if (!m_shared_instance)
		{
			m_shared_instance = std::make_shared<game_loop_win32>();
		}
		return m_shared_instance;
	}

	void assign_hwnd_to_game_loop(const void* hwnd)
	{

	}

	void add_listener_to_game_loop(const std::shared_ptr<i_game_loop>& listener)
	{
		game_loop_win32::shared_instance()->add_listener(listener);
	};

	void remove_listener_from_game_loop(const std::shared_ptr<i_game_loop>& listener)
	{
		game_loop_win32::shared_instance()->remove_listener(listener);
	};

	void execute_runloop()
	{
		MSG msg;
		memset(&msg, 0, sizeof(msg));
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				game_loop_win32::shared_instance()->on_update();
			}
		}
	}

	void pause_run_loop()
	{
		game_loop_win32::shared_instance()->pause_run_loop();
	}

	void resume_run_loop()
	{
		game_loop_win32::shared_instance()->resume_run_loop();
	}
}

#endif
