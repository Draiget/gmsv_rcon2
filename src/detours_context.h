#pragma once

#ifdef _PLATFORM_LINUX
#include <sys/types.h>
#endif

namespace zontwelg 
{
	struct detour_patch_t
	{
		detour_patch_t() {
			patch[0] = 0;
			bytes = 0;
		}

		unsigned char patch[20]{};
		size_t bytes{};
	};

	class c_detour_context
	{
	public:
		static c_detour_context* create(void* p_detour_callback_fn, void** p_trampoline, const char* p_detour_name, const char* p_signature, const char* p_lib_name);
		static void copy_function(detour_patch_t* reference, void* src_address);

		bool is_enabled() const;
		void enable_detour();
		void disable_detour();
		void destroy();

		void* m_detour_address;
		void* m_detour_trampoline;
		void* m_detour_callback;

	protected:
		c_detour_context(void* p_detour_callback_fn, void** p_trampoline, const char* p_detour_name, const char* p_signature, const char* p_lib_name);
		bool init();

	private:
		void rewind(char* p_out_base = nullptr, char* p_out_ptr = nullptr);
		bool create_detour();
		void delete_detour();

		bool m_enabled_;
		bool m_detoured_;
		detour_patch_t m_detour_restore_;
		void** m_trampoline_;
		const char* m_detour_name_;
		const char* m_detour_signature_;
		const char* m_detour_lib_name_;
	};

	class generic_class {};
	typedef void (generic_class::* void_func)();

	inline void* get_code_address(void_func mfp) {
		return *reinterpret_cast<void**>(&mfp);
	}
}

#define GetCodeAddress(mfp) \
		zontwelg::get_code_address(reinterpret_cast<zontwelg::void_func>(mfp))
