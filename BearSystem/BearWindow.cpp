#include "BearGraphics.hpp"
#ifdef WINDOWS
BEARTOOL_API BearMap<int32, int32>* LToWinowsKey;
BEARTOOL_API BearMap<int32, int32>* GFromWinowsKey;
static bsize LCount = 0;
LRESULT CALLBACK GlobalOnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams));
		break;
	}
	{
		auto viewport = reinterpret_cast<BearWindow*>(GetWindowLongPtrW(handle, GWLP_USERDATA));
		if (viewport)
		{
			viewport->OnEvent(handle, message, wParam, lParam);
		}
	}
	return DefWindowProc(handle, message, wParam, lParam);
}
LRESULT CALLBACK GlobalOnEventNoClosed(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CLOSE:
		return 0;
		break;
	case WM_CREATE:
		SetWindowLongPtrW(handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams));
		break;
	}
	{
		auto viewport = reinterpret_cast<BearWindow*>(GetWindowLongPtrW(handle, GWLP_USERDATA));
		if (viewport)
		{
			viewport->OnEvent(handle, message, wParam, lParam);
		}
	}
	return DefWindowProc(handle, message, wParam, lParam);
}


static bool LBWindowsClass = false;
static bool LBWindowsClassNC = false;
static void RegisterWindowsClass(HINSTANCE hInstance, bool closed)
{
	if (closed)
	{
		if (LBWindowsClass)return;
		LBWindowsClass = true;
	}
	else
	{
		if (LBWindowsClassNC)return;
		LBWindowsClassNC = true;
	}


	WNDCLASSEX wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	if (closed)
	{
		wc.lpfnWndProc = GlobalOnEvent;
	}
	else
	{
		wc.lpfnWndProc = GlobalOnEventNoClosed;
	}
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
#ifdef DEBUG
	wc.hbrBackground = CreateSolidBrush(RGB(69, 22, 28));
#else
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
#endif
	wc.lpszMenuName = NULL;
	if (closed)
	{
		wc.lpszClassName = TEXT("BEAR");
	}
	else
	{
		wc.lpszClassName = TEXT("BEARNC");
	}
	wc.cbSize = sizeof(WNDCLASSEX);
	RegisterClassEx(&wc);
}
BearWindow::BearWindow(bsize width, bsize height, bool fullscreen, BearFlags<int32> flags) :m_MouseShow(1),m_width(width), m_height(height), m_fullscreen(false), m_mouse_enter(false)
{
	m_events_item = m_events.end();
	HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
	RegisterWindowsClass(hInstance, !flags.test(TW_WIHTOUT_CLOSED));



	m_Style = WS_POPUP;
	if (!flags.test(TW_POPUP))
	{
		if (flags.test(TW_ONLY_CLOSED)) m_Style = WS_OVERLAPPED | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_DLGFRAME;
		else m_Style = WS_OVERLAPPEDWINDOW;
	}

	{
		RECT rectangle = { 0, 0, static_cast<long>(width), static_cast<long>(height) };

		if (!flags.test(TW_WIHTOUT_CLOSED))
		{
			m_window = CreateWindowEx(0, TEXT("BEAR"), TEXT(""), m_Style, 0, 0, 1, 1, NULL, NULL, hInstance, this);
		}
		else
		{
			m_window = CreateWindowEx(0, TEXT("BEARNC"), TEXT(""), m_Style, 0, 0, 1, 1, NULL, NULL, hInstance, this);
		}

		AdjustWindowRect(&rectangle, GetWindowLong((HWND)m_window, GWL_STYLE), false);
		SetWindowPos((HWND)m_window, NULL, 0, 0, rectangle.right - rectangle.left, rectangle.bottom - rectangle.top, SWP_NOMOVE | SWP_NOZORDER);

		uint32 xpos = static_cast<int32>(((uint32)GetSystemMetrics(SM_CXSCREEN) / 2) - (width / 2));
		uint32 ypos = static_cast<int32>(((uint32)GetSystemMetrics(SM_CYSCREEN) / 2) - (height / 2));

		SetWindowPos((HWND)m_window, NULL, xpos, ypos, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}

	ShowWindow((HWND)m_window, SW_SHOW);
	if (fullscreen)SetFullscreen(fullscreen);

}

BearWindow::~BearWindow()
{
	DestroyWindow(GetWindowHandle());

}

void BearWindow::Resize(bsize width, bsize height)
{
	m_width = width;
	m_height = height;

	if (!m_fullscreen)
	{




		SetWindowLong((HWND)m_window, GWL_STYLE, m_Style);
		//AdjustWindowRect(&rectangle, GetWindowLong((HWND)m_window, GWL_STYLE), false);

		RECT			m_rcWindowBounds;
		RECT				DesktopRect;

		GetClientRect(GetDesktopWindow(), &DesktopRect);

		SetRect(&m_rcWindowBounds,
			(DesktopRect.right -static_cast<LONG>( m_width)) / 2,
			(DesktopRect.bottom - static_cast<LONG>(m_height)) / 2,
			(DesktopRect.right + static_cast<LONG>(m_width)) / 2,
			(DesktopRect.bottom + static_cast<LONG>(m_height)) / 2);

		AdjustWindowRect(&m_rcWindowBounds, m_Style, FALSE);

		SetWindowPos((HWND)m_window,
			HWND_NOTOPMOST,
			m_rcWindowBounds.left,
			m_rcWindowBounds.top,
			(m_rcWindowBounds.right - m_rcWindowBounds.left),
			(m_rcWindowBounds.bottom - m_rcWindowBounds.top),
			SWP_SHOWWINDOW | SWP_NOCOPYBITS | SWP_DRAWFRAME);
	}
	else
	{
		SetWindowPos((HWND)m_window, HWND_TOP, 0, 0, static_cast<int32>(m_width), static_cast<int32>(m_height), SWP_FRAMECHANGED);
		SetWindowLong((HWND)m_window, GWL_STYLE, WS_EX_TOPMOST | WS_POPUP | WS_VISIBLE);
		ShowWindow((HWND)m_window, SW_MAXIMIZE);
		SetWindowLong(m_window, GWL_EXSTYLE, WS_EX_TOPMOST);

		SetForegroundWindow(m_window);
	}
}
void BearWindow::SetFullscreen(bool fullscreen)
{
	m_fullscreen = fullscreen;

	if (m_fullscreen)
	{
		SetWindowLong((HWND)m_window, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		SetWindowLong((HWND)m_window, GWL_EXSTYLE, WS_EX_TOPMOST);
		SetFocus(m_window);


	}
	else
	{
		SetWindowLong((HWND)m_window, GWL_STYLE, WS_DLGFRAME | WS_SYSMENU | WS_MINIMIZEBOX);


	}
	Resize(m_width, m_height);
	SetForegroundWindow((HWND)m_window);
}

bool BearWindow::Update()
{
	m_events.clear_not_free();
	m_events_item = m_events.end();
	MSG msg;
	while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	if (msg.message == WM_QUIT)
		return false;
	m_events_item = m_events.begin();
	return true;
}

void BearWindow::ShowCursor(bool show)
{
	if (m_MouseShow != show)
		::ShowCursor(show);
	m_MouseShow = show;
}

BearVector2<float> BearWindow::GetMousePosition()
{
	POINT point;
	GetCursorPos(&point);
	if (!IsFullScreen())	ScreenToClient(GetWindowHandle(), &point);
	return BearFVector2(static_cast<float>(point.x), static_cast<float>(point.y));
}

void BearWindow::SetMousePosition(const BearVector2<float>& position)
{
	POINT point = { static_cast<LONG>(position.x),static_cast<LONG>(position.y) };
	if (!IsFullScreen())
		ClientToScreen(GetWindowHandle(), &point);
	SetCursorPos(point.x, point.y);
}

bool BearWindow::GetEvent(BearEventWindows& e)
{
	/*if (m_event_resize.Type == WET_Resize && m_resize_timer.get_elapsed_time().asmiliseconds() > 100)
	{
		e = m_event_resize;
		m_event_resize.Type = WET_None;
	}
	else
	{*/
	if (m_events_item == m_events.end())
		return false;
	while (m_events_item + 1 != m_events.end() && (m_events_item + 1)->Type == WET_Resize) { m_events_item++; }
	e = *m_events_item;
	m_events_item++;
	//}
	return true;
}

bool BearWindow::Empty() const
{
	return m_window == 0;
}

void BearWindow::OnEvent(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	bint i = 0;
	if (m_events_item == m_events.end()) i = -1;
	else i = m_events.end() - m_events_item;
	BearEventWindows ev;
	switch (message)
	{
	case WM_SIZE:
	{

		RECT rect;
		GetClientRect(GetWindowHandle(), &rect);

		BearVector2<bsize> new_size(rect.right - rect.left, rect.bottom - rect.top);

		if (m_fullscreen == false &&wParam != SIZE_MINIMIZED && new_size != GetSize())
		{
			if(new_size.x + 8 == m_width && new_size.y + 31 == m_height ) break;
			m_width = new_size.x;
			m_height = new_size.y;
			ev.Size.width = GetSizeFloat().x;
			ev.Size.height = GetSizeFloat().y;
			ev.Type = WET_Resize;
			/*m_event_resize = ev;
			m_resize_timer.restart();*/
			m_events.push_back(ev);
		}
		break;
	}
	case WM_MOUSEMOVE:
	{
		int32 x = static_cast<int32>(LOWORD(lParam));
		int32 y = static_cast<int32>(HIWORD(lParam));

		RECT zone;
		GetClientRect(GetWindowHandle(), &zone);
		if (m_mouse_enter)
		{
			if ((zone.left > x || zone.right < x) ||
				(zone.top > y || zone.bottom < y))
			{
				ev.Type = WET_MouseLevae;
				m_events.push_back(ev);
				m_mouse_enter = false;
			}

		}
		else
		{
			if ((zone.left <x && zone.right > x) &&
				(zone.top < y && zone.bottom > y))
			{
				ev.Type = WET_MouseEnter;
				m_events.push_back(ev);
				m_mouse_enter = true;
			}
		}
		ev.Type = WET_MouseMove;
		ev.Position.x = static_cast<float>(x);
		ev.Position.y = static_cast<float>(y);
		m_events.push_back(ev);
		break;
	}
	case WM_SETFOCUS:
	{
		ev.Type = WET_Active;
		m_events.push_back(ev);
		break;
	}
	case WM_KILLFOCUS:
	{
		ev.Type = WET_Deactive;
		m_events.push_back(ev);
		break;
	}
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_MENU:
			if (lParam & (1 << 24))
				wParam = VK_RMENU;
			else
				wParam = VK_LMENU;
			break;
		case VK_CONTROL:
			if (lParam & (1 << 24))
				wParam = VK_RCONTROL;
			else
				wParam = VK_LCONTROL;
			break;
		case VK_SHIFT:
			if (lParam & (1 << 24))
				wParam = VK_RSHIFT;
			else
				wParam = VK_LSHIFT;
			break;

		};
		auto item = GFromWinowsKey->find(DWORD(wParam));
		if (item == GFromWinowsKey->end())
			break;
		ev.Type = WET_KeyDown;
		ev.Key = static_cast<BearInput::Key>(item->second);
		m_events.push_back(ev);
		break;
	}
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		switch (wParam)
		{
		case VK_MENU:
			if (lParam & (1 << 24))
				wParam = VK_RMENU;
			else
				wParam = VK_LMENU;
			break;
		case VK_CONTROL:
			if (lParam & (1 << 24))
				wParam = VK_RCONTROL;
			else
				wParam = VK_LCONTROL;
			break;
		case VK_SHIFT:
			if (lParam & (1 << 24))
				wParam = VK_RSHIFT;
			else
				wParam = VK_LSHIFT;
			break;

		};
		auto item = GFromWinowsKey->find(DWORD(wParam));
		if (item == GFromWinowsKey->end())
			break;
		ev.Type = WET_KeyUp;
		ev.Key = static_cast<BearInput::Key>(item->second);
		m_events.push_back(ev);
		break;
	}
	case WM_MOUSEWHEEL:
	{
		int16 delta = static_cast<int16>(HIWORD(wParam));
		if (delta > 0)
		{
			ev.Type = WET_KeyDown;
			ev.Change = delta / 120.f;
			ev.Key = BearInput::KeyMouseScrollUp;
		}
		else if (delta < 0)
		{
			ev.Type = WET_KeyDown;
			ev.Change = delta / 120.f;
			ev.Key = BearInput::KeyMouseScrollDown;
		}
		m_events.push_back(ev);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		ev.Type = WET_KeyDown;
		ev.Key = BearInput::KeyMouseLeft;
		m_events.push_back(ev);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		ev.Type = WET_KeyDown;
		ev.Key = BearInput::KeyMouseRight;
		m_events.push_back(ev);
		break;
	}
	case WM_MBUTTONDOWN:
	{
		ev.Type = WET_KeyDown;
		ev.Key = BearInput::KeyMouseMiddle;
		m_events.push_back(ev);
		break;
	}
	case WM_LBUTTONUP:
	{
		ev.Type = WET_KeyUp;
		ev.Key = BearInput::KeyMouseLeft;
		m_events.push_back(ev);
		break;
	}
	case WM_RBUTTONUP:
	{
		ev.Type = WET_KeyUp;
		ev.Key = BearInput::KeyMouseRight;
		m_events.push_back(ev);
		break;
	}
	case WM_MBUTTONUP:
	{
		ev.Type = WET_KeyUp;
		ev.Key = BearInput::KeyMouseMiddle;
		m_events.push_back(ev);
		break;
	}
	case WM_CHAR:
	{
		TCHAR ch = TCHAR(wParam);
		ev.Type = WET_Char;
		ev.Char = ch;
		m_events.push_back(ev);
		break;
	}
	default:
		return;
	}

	if (i < 0)
	{
		m_events_item = m_events.end();
	}
	else if (i)
	{
		m_events_item = m_events.begin() + i;
	}
	return;
}
#endif