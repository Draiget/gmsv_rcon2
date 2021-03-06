#pragma once

#define DETOUR_MEMBER_CALL(name) \
		(this->*name##_Actual)

#define DETOUR_DECLARE_A1( name, ret, arg1type, arg1name) \
		class name##Class {\
		public:\
			ret DETOUR_##name(arg1type arg1name);\
			static ret (name##Class::* name##_Actual)(arg1type);\
		};\
		extern zontwelg::c_detour_context *g_pDETOUR_##name;\
		extern ret (*name##_Actual)(arg1type)

#define DETOUR_DECLARE_MEMBER_A1(name, ret, arg1type, arg1name) \
		zontwelg::c_detour_context *g_pDETOUR_##name = nullptr;\
		ret (name##Class::* name##Class::name##_Actual)(arg1type) = nullptr;\
		ret name##Class::DETOUR_##name(arg1type arg1name)

#define DETOUR_DECLARE_A2( name, ret, arg1type, arg1name, arg2type, arg2name) \
		class name##Class {\
		public:\
			ret DETOUR_##name(arg1type arg1name, arg2type arg2name);\
			static ret (name##Class::* name##_Actual)(arg1type, arg2type);\
		};\
		extern zontwelg::c_detour_context *g_pDETOUR_##name;\
		extern ret (*name##_Actual)(arg1type, arg2type)

#define DETOUR_DECLARE_MEMBER_A2(name, ret, arg1type, arg1name, arg2type, arg2name) \
		zontwelg::c_detour_context *g_pDETOUR_##name = nullptr;\
		ret (name##Class::* name##Class::name##_Actual)(arg1type, arg2type) = nullptr;\
		ret name##Class::DETOUR_##name(arg1type arg1name, arg2type arg2name)

#define DETOUR_DECLARE_A4( name, ret, arg1type, arg1name, arg2type, arg2name, arg3type, arg3name, arg4type, arg4name) \
		class name##Class {\
		public:\
			ret DETOUR_##name(arg1type arg1name, arg2type arg2name, arg3type arg3name, arg4type arg4name);\
			static ret (name##Class::* name##_Actual)(arg1type, arg2type, arg3type, arg4type);\
		};\
		extern zontwelg::c_detour_context *g_pDETOUR_##name;\
		extern ret (*name##_Actual)(arg1type, arg2type, arg3type, arg4type)

#define DETOUR_DECLARE_MEMBER_A4(name, ret, arg1type, arg1name, arg2type, arg2name, arg3type, arg3name, arg4type, arg4name) \
		zontwelg::c_detour_context *g_pDETOUR_##name = nullptr;\
		ret (name##Class::* name##Class::name##_Actual)(arg1type, arg2type, arg3type, arg4type) = nullptr;\
		ret name##Class::DETOUR_##name(arg1type arg1name, arg2type arg2name, arg3type arg3name, arg4type arg4name)

#define GET_STATIC_CALLBACK(name) (void *)&DETOUR_##name
#define GET_STATIC_TRAMPOLINE(name) (void **)&name##_Actual
#define GET_MEMBER_CALLBACK(name) (void *)GetCodeAddress(&name##Class::DETOUR_##name)
#define GET_MEMBER_TRAMPOLINE(name) (void **)(&name##Class::name##_Actual)
#define GET_ORIGINAL_ADDRESS(name) g_pDETOUR_##name->m_detour_address

#define DETOUR_CREATE_MEMBER(name, detourname, signature, file)\
		g_pDETOUR_##name = zontwelg::c_detour_context::create(GET_MEMBER_CALLBACK(name), GET_MEMBER_TRAMPOLINE(name), detourname, signature, file);\
		if (!g_pDETOUR_##name) return false;\
		g_pDETOUR_##name->enable_detour();

#define DETOUR_CREATE_MEMBER_EX(name, detourname, signature, file, err_msg)\
		g_pDETOUR_##name = zontwelg::c_detour_context::create(GET_MEMBER_CALLBACK(name), GET_MEMBER_TRAMPOLINE(name), detourname, signature, file);\
		if (!g_pDETOUR_##name){ Msg(err_msg); return false; }\
		g_pDETOUR_##name->enable_detour();

#define DETOUR_DESTROY(name)\
		if (g_pDETOUR_##name) g_pDETOUR_##name->destroy();\
		g_pDETOUR_##name = nullptr;
