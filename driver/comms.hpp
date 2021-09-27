#pragma once

namespace comms
{
	enum class control_codes : uint8_t
	{
		none,
		status,
		attach_process,
		detach_process,
		get_window_thread,
		set_window_thread,
		find_module,
		key_state,
		read,
		write,
		read_injected,
		hide_window,
		write_read_only
	};

	typedef struct _INOUT_SHAREDDATA
	{
		HANDLE ProcessId;
		HANDLE hEventUser;
		HANDLE hEventDriver;
		bool   Finish;
		bool   DataWritten;
	} INOUT_SHAREDDATA, * PINOUT_SHAREDDATA;

	typedef struct _MEMORY_OPERATION
	{
		uint32_t			  cookie;
		control_codes		  control_code;
		NTSTATUS              status;
		char                  Data[0x1000];
	} MEMORY_OPERATION, * PMEMORY_OPERATION;

	struct c_hide_window_request
	{
		HANDLE pid;
		HWND hwnd;

		c_hide_window_request()
		{
			this->pid = nullptr;
			this->hwnd = nullptr;
		}
	};

	struct c_attach_process_request
	{
		uint32_t hash;

		c_attach_process_request( const uint32_t hash ) : c_attach_process_request()
		{
			this->hash = hash;
		}
		c_attach_process_request()
		{
			this->hash = 0;
		}
	};

	struct c_detach_process_request
	{
		c_detach_process_request() = default;
	};

	struct c_get_window_thread_request
	{
		uintptr_t window_handle;
		uintptr_t res;

		c_get_window_thread_request( const uintptr_t window_handle ) : c_get_window_thread_request()
		{
			this->window_handle = window_handle;
		}
		c_get_window_thread_request()
		{
			this->window_handle = 0;
			this->res = 0;
		}
	};

	struct c_set_window_thread_request
	{
		uintptr_t target_window_handle;
		uintptr_t thread_pointer;

		c_set_window_thread_request( const uintptr_t window_handle, const uintptr_t thread_pointer ) : c_set_window_thread_request()
		{
			this->target_window_handle = window_handle;
			this->thread_pointer = thread_pointer;
		}
		c_set_window_thread_request()
		{
			this->target_window_handle = 0;
			this->thread_pointer = 0;
		}
	};

	struct c_find_module_request
	{
		uint32_t hash;
		uintptr_t res;

		c_find_module_request( const uint32_t hash ) : c_find_module_request()
		{
			this->hash = hash;
		}
		c_find_module_request()
		{
			this->hash = 0;
			this->res = 0;
		}
	};

	struct c_key_state_request
	{
		int key;
		bool res;

		c_key_state_request( const int key ) : c_key_state_request()
		{
			this->key = key;
		}

		c_key_state_request()
		{
			this->key = 0;
			this->res = false;
		}
	};

	struct c_read_write_request
	{
		void* address;
		void* buffer;
		size_t size;

		c_read_write_request(
			void* address,
			void* buffer,
			const size_t size ) : c_read_write_request()
		{
			this->address = address;
			this->buffer = buffer;
			this->size = size;
		}

		c_read_write_request()
		{
			this->address = 0;
			this->buffer = 0;
			this->size = 0;
		}
	};

	struct c_change_prot_request
	{
		uintptr_t address;

		c_change_prot_request(
			uintptr_t address
		) : c_change_prot_request()
		{
			this->address = address;
		}

		c_change_prot_request()
		{
			this->address = 0;
		}
	};

	extern void ProcessRequest(const comms::PMEMORY_OPERATION& pData );
}