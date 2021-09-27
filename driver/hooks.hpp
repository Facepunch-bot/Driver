#pragma once

namespace hooks
{
	using NtUserWindowFromPoint_ = HWND( NTAPI* )( LONG, LONG );
	extern NtUserWindowFromPoint_ oNtUserWindowFromPoint;
	extern HWND NTAPI hkNtUserWindowFromPoint( LONG x, LONG y );

	using NtUserQueryWindow_ = HANDLE( NTAPI* )( HWND, WINDOWINFOCLASS );
	extern NtUserQueryWindow_ oNtUserQueryWindow;
	extern HANDLE NTAPI hkNtUserQueryWindow( HWND WindowHandle, WINDOWINFOCLASS WindowInfo );

	using NtUserFindWindowEx_ = HWND( NTAPI* )( HWND, HWND, PUNICODE_STRING, PUNICODE_STRING, DWORD );
	extern NtUserFindWindowEx_ oNtUserFindWindowEx;
	extern HWND NTAPI hkNtUserFindWindowEx( HWND hWndParent, HWND hWndChildAfter, PUNICODE_STRING lpszClass, PUNICODE_STRING lpszWindow, DWORD dwType );

	using NtUserBuildHwndList_ = NTSTATUS( NTAPI* )( HDESK, HWND, BOOLEAN, BOOLEAN, ULONG, ULONG, HWND*, PULONG );
	extern NtUserBuildHwndList_ oNtUserBuildHwndList;
	extern NTSTATUS NTAPI hkNtUserBuildHwndList( HDESK hDesktop, HWND hwndParent, BOOLEAN bChildren, BOOLEAN bUnknownFlag, ULONG dwThreadId, ULONG lParam, HWND* pWnd, PULONG pBufSize );

	using NtUserGetForegroundWindow_ = HWND( NTAPI* )( );
	extern NtUserGetForegroundWindow_ oNtUserGetForegroundWindow;
	extern HWND NTAPI hkNtUserGetForegroundWindow( );

	using NtQueryInformationAtom_ = NTSTATUS( NTAPI* )( IN RTL_ATOM, IN ATOM_INFORMATION_CLASS, OUT PVOID, IN ULONG, OUT PULONG OPTIONAL );
	extern NtQueryInformationAtom_ oNtQueryInformationAtom;
	extern NTSTATUS NTAPI hkNtQueryInformationAtom( IN RTL_ATOM Atom, IN ATOM_INFORMATION_CLASS AtomInformationClass, OUT PVOID AtomInformation, IN ULONG AtomInformationLength, OUT PULONG ReturnLength OPTIONAL );

	inline HANDLE hMyHwndPid = nullptr;
	inline HWND hMyHwnd = nullptr;
}