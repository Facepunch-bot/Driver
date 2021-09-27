#include "includes.hpp"

namespace comms
{
	PEPROCESS process = nullptr;
	uintptr_t local_pid;
	void ProcessRequest(const comms::PMEMORY_OPERATION& pData )
	{
		switch ( pData->control_code )
		{

		case comms::control_codes::status:
		{
			pData->status = STATUS_SUCCESS;
			break;
		}

		case comms::control_codes::hide_window:
		{
			auto Ptr = reinterpret_cast< c_hide_window_request* >( pData->Data );
			if ( Ptr )
			{
				hooks::hMyHwndPid = Ptr->pid;
				hooks::hMyHwnd = Ptr->hwnd;

				/*if ( hooks::hMyHwnd )
				{
					const auto wnd = ValidateHwnd( reinterpret_cast< uintptr_t >( hooks::hMyHwnd ) );
					if ( wnd )
						*reinterpret_cast< uintptr_t* >( wnd + 0x68 ) = 0;
				}*/
				pData->status = STATUS_SUCCESS;
			}
			break;
		}

		case comms::control_codes::attach_process:
		{
			auto pAttachProcess = reinterpret_cast< comms::c_attach_process_request* >( pData->Data );
			if ( pAttachProcess )
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "c_attach_process_request->processhash %lu\n", pAttachProcess->hash);
				process = process::find(pAttachProcess->hash);
				local_pid = FindProcess(L"RustClient.exe");
				pData->status = STATUS_SUCCESS;
			}
			break;
		}

		case comms::control_codes::detach_process:
		{
			process = nullptr;
			pData->status = STATUS_SUCCESS;
			break;
		}

		case comms::control_codes::find_module:
		{
			auto pFindModule = reinterpret_cast< comms::c_find_module_request* >( pData->Data );
			if (!pFindModule)
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "pFindModule = he is the nigger \n");

			if (!(process = process::find(h("RustClient.exe"))))
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "Invalid Process.\n");
			
			if ( process && pFindModule )
			{
				const auto peb = PsGetProcessPeb( process );

				if (process::is_x86(process))
				{
					pFindModule->res = process::find_module_x86(process, peb, pFindModule->hash);
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "pFindModule->res %X\n", pFindModule->res);
				}
				else
				{
					pFindModule->res = process::find_module_x64(process, peb, pFindModule->hash);
					DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "pFindModule->res %X\n", pFindModule->res);
					
				}
			}
			else
			{
				DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "pFindModule = nigger \n");
			}
			break;
		}

		case comms::control_codes::key_state:
		{
			auto pGetKey = reinterpret_cast< comms::c_key_state_request* >( pData->Data );
			if ( pGetKey )
			{
				bool res = false;
				auto key = pGetKey->key;

				PEPROCESS Proc = nullptr;
				if ( NT_SUCCESS( PsLookupProcessByProcessId( HANDLE(local_pid), &Proc ) ) )
				{
					KAPC_STATE apc{ };
					KeStackAttachProcess( Proc, &apc );
					{
						res = NtGetAsyncKeyState( key );
					}
					KeUnstackDetachProcess( &apc );
					ObDereferenceObject( Proc );
				}

				pGetKey->res = res;
			}
			break;
		}

		case comms::control_codes::read:
		{
			auto pRead = reinterpret_cast< comms::c_read_write_request* >( pData->Data );
			if ( pRead )
			{
				pData->status = process::Read(local_pid, process, pRead->address, pRead->buffer, pRead->size);
			}
			break;
		}

		case comms::control_codes::write:
		{
			auto pWrite = reinterpret_cast< comms::c_read_write_request* >( pData->Data );
			if ( pWrite )
			{
				pData->status = process::Write(local_pid, process, pWrite->address, pWrite->buffer, pWrite->size );
			}
			break;
		}

		case comms::control_codes::read_injected:
		{
			auto pRead = reinterpret_cast< comms::c_read_write_request* >( pData->Data );
			if ( pRead )
			{
				pData->status = process::ReadInjected(local_pid, process, reinterpret_cast< uintptr_t >( pRead->address ), pRead->buffer );
			}
			break;
		}

		/*case comms::control_codes::write_read_only:
		{
			const auto request = reinterpret_cast < comms::c_change_prot_request* >( pData->Data );
			if ( request )
			{
				pData->status = process::ProtectVirtualMemory( reinterpret_cast< UINT_PTR >(process::find(h("RustClient.exe")), ( PVOID )request->address, sizeof( uintptr_t ), 0 );
			}
			break;
		}*/
		}
	}
}