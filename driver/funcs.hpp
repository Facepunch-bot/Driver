#pragma once

using EtwpIsProcessZombie_ = bool( NTAPI* )( PEPROCESS );
inline EtwpIsProcessZombie_ EtwpIsProcessZombie = NULL;

using PsGetNextProcess_ = PEPROCESS( NTAPI* )( PEPROCESS );
inline PsGetNextProcess_ PsGetNextProcess = NULL;

using NtGetAsyncKeyState_ = SHORT( NTAPI* )( INT );
inline NtGetAsyncKeyState_ NtGetAsyncKeyState = NULL;

using ValidateHwnd_ = uintptr_t( __stdcall* )( uintptr_t );
inline ValidateHwnd_ ValidateHwnd = NULL;

//using MiGetPteAddress_ = PMMPTE( NTAPI* )( PVOID );
//inline MiGetPteAddress_ MiGetPteAddress = NULL;

using PspThreadDelete_ = void ( NTAPI* )( void* );
inline PspThreadDelete_ PspThreadDelete = NULL;

inline RTL_AVL_TABLE* PiDDBCacheTbl = nullptr;

namespace funcs
{
	inline BOOLEAN SetupFunctions()
	{
		const auto PiDDBCacheTbl_rel = reinterpret_cast< uint8_t* >(
			utils::FindPattern( XS( "ntoskrnl.exe" ),
				XS( "PAGE" ),
				PUCHAR( "\x48\x8D\x0D\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x3D\x00\x00\x00\x00\x0F\x83" ),
				XS( "xxx????x????x????xx" ) ) );

		if ( !PiDDBCacheTbl_rel )
		{
			DbgOut( "PiDDBCacheTbl sig not found!" );
			return FALSE;
		}

		PiDDBCacheTbl = utils::resolve_mov< RTL_AVL_TABLE* >( PiDDBCacheTbl_rel );
		if ( !PiDDBCacheTbl )
		{
			DbgOut( "PiDDBCacheTbl sig not found!" );
			return FALSE;
		}

		PspThreadDelete = reinterpret_cast< PspThreadDelete_ >( utils::FindPattern( XS( "ntoskrnl.exe" ),
			XS( "PAGE" ),
			PUCHAR( "\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x56\x57\x41\x54\x41\x56\x41\x57\x48\x83\xEC\x40\x48\x8B\xF9" ),
			XS( "xxxx?xxxx?xxxxxxxxxxxxxxx" ) ) );

		if ( !PspThreadDelete )
		{
			DbgOut( "PspThreadDelete sig not found!" );
			return FALSE;
		}

		/*MiGetPteAddress = reinterpret_cast< MiGetPteAddress_ >( utils::FindPattern( XS( "ntoskrnl.exe" ),
				XS( ".text" ),
				PUCHAR( "\x48\xC1\xE9\x09\x48\xB8\xF8\xFF\xFF\xFF\x7F\x00\x00\x00\x48\x23\xC8\x48\xB8\x00\x00\x00\x00\x00\x00\x00\x00\x48\x03\xC1\xC3" ),
				XS( "xxxxxxxxxxxxxxxxxxx????????xxxx" ) ) );

		if ( !MiGetPteAddress )
		{
			DbgOut( "MiGetPteAddress sig not found!" );
			return FALSE;
		}*/

		EtwpIsProcessZombie = reinterpret_cast< EtwpIsProcessZombie_ >( utils::FindPattern( XS( "ntoskrnl.exe" ), XS( "PAGE" ), PUCHAR( "\x8B\x81\x00\x00\x00\x00\xA8\x04\x75\x00\x33\xC0" ), XS( "xx????xxx?xx" ) ) );
		if ( !EtwpIsProcessZombie )
		{
			DbgOut( "EtwpIsProcessZombie sig not found!" );
			return FALSE;
		}

		const auto PsGetNextProcess_rel = reinterpret_cast< uint8_t* >( utils::FindPattern( XS( "ntoskrnl.exe" ), XS( "PAGE" ), PUCHAR( "\x79\xDC\xE9" ), XS( "xxx" ) ) );
		if ( !PsGetNextProcess_rel )
		{
			DbgOut( "PsGetNextProcess sig not found!" );
			return FALSE;
		}

		PsGetNextProcess = utils::resolve_call< PsGetNextProcess_ >( utils::resolve_jxx( PsGetNextProcess_rel ) );
		if ( !PsGetNextProcess )
		{
			DbgOut( "PsGetNextProcess sig not found!" );
			return FALSE;
		}

		if ( auto win32k = utils::GetWinlogon() )
		{
			KAPC_STATE apc{ };
			KeStackAttachProcess( win32k, &apc );

			ValidateHwnd = reinterpret_cast< ValidateHwnd_ >( utils::FindExport( XS( "win32kbase.sys" ), XS( "ValidateHwnd" ) ) );
			if ( !ValidateHwnd )
			{
				DbgOut( "ValidateHwnd sig not found!" );
				return FALSE;
			}

			NtGetAsyncKeyState = reinterpret_cast< NtGetAsyncKeyState_ >( utils::FindExport( XS( "win32kbase.sys" ), XS( "_GetAsyncKeyState" ) ) );
			if ( !NtGetAsyncKeyState )
			{
				DbgOut( "NtGetAsyncKeyState sig not found!" );
				return FALSE;
			}

			KeUnstackDetachProcess( &apc );
			ObDereferenceObject( win32k );
		}

		//DbgOut( "MiGetPteAddress = 0x%p", MiGetPteAddress );
		DbgOut( "PspThreadDelete = 0x%p", PspThreadDelete );
		DbgOut( "PiDDBCacheTbl = 0x%p", PiDDBCacheTbl );
		DbgOut( "EtwpIsProcessZombie = 0x%p", EtwpIsProcessZombie );
		DbgOut( "PsGetNextProcess = 0x%p", PsGetNextProcess );
		DbgOut( "ValidateHwnd = 0x%p", ValidateHwnd );
		DbgOut( "NtGetAsyncKeyState = 0x%p", NtGetAsyncKeyState );
		return TRUE;
	}
}