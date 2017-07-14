#ifndef ZONTWELG_GMSVRCON2_MACRO_H
#define ZONTWELG_GMSVRCON2_MACRO_H

#define DETOUR_MEMBER_CALL(name) \
		(this->*name##_Actual)

#define DETOUR_DECLARE_A1( name, ret, arg1type, arg1name) \
		class name##Class {\
		public:\
			ret DETOUR_##name(arg1type arg1name);\
			static ret (name##Class::* name##_Actual)(arg1type);\
		};\
		extern CDetourContext *g_pDETOUR_##name;\
		extern ret (*name##_Actual)(arg1type)

#define DETOUR_DECLARE_MEMBER_A1(name, ret, arg1type, arg1name) \
		CDetourContext *g_pDETOUR_##name = nullptr;\
		ret (name##Class::* name##Class::name##_Actual)(arg1type) = nullptr;\
		ret name##Class::DETOUR_##name(arg1type arg1name)

#define DETOUR_DECLARE_A2( name, ret, arg1type, arg1name, arg2type, arg2name) \
		class name##Class {\
		public:\
			ret DETOUR_##name(arg1type arg1name, arg2type arg2name);\
			static ret (name##Class::* name##_Actual)(arg1type, arg2type);\
		};\
		extern CDetourContext *g_pDETOUR_##name;\
		extern ret (*name##_Actual)(arg1type, arg2type)

#define DETOUR_DECLARE_MEMBER_A2(name, ret, arg1type, arg1name, arg2type, arg2name) \
		CDetourContext *g_pDETOUR_##name = nullptr;\
		ret (name##Class::* name##Class::name##_Actual)(arg1type, arg2type) = nullptr;\
		ret name##Class::DETOUR_##name(arg1type arg1name, arg2type arg2name)

#define DETOUR_DECLARE_A4( name, ret, arg1type, arg1name, arg2type, arg2name, arg3type, arg3name, arg4type, arg4name) \
		class name##Class {\
		public:\
			ret DETOUR_##name(arg1type arg1name, arg2type arg2name, arg3type arg3name, arg4type arg4name);\
			static ret (name##Class::* name##_Actual)(arg1type, arg2type, arg3type, arg4type);\
		};\
		extern CDetourContext *g_pDETOUR_##name;\
		extern ret (*name##_Actual)(arg1type, arg2type, arg3type, arg4type)

#define DETOUR_DECLARE_MEMBER_A4(name, ret, arg1type, arg1name, arg2type, arg2name, arg3type, arg3name, arg4type, arg4name) \
		CDetourContext *g_pDETOUR_##name = nullptr;\
		ret (name##Class::* name##Class::name##_Actual)(arg1type, arg2type, arg3type, arg4type) = nullptr;\
		ret name##Class::DETOUR_##name(arg1type arg1name, arg2type arg2name, arg3type arg3name, arg4type arg4name)

#define GET_STATIC_CALLBACK(name) (void *)&DETOUR_##name
#define GET_STATIC_TRAMPOLINE(name) (void **)&name##_Actual
#define GET_MEMBER_CALLBACK(name) (void *)GetCodeAddress(&name##Class::DETOUR_##name)
#define GET_MEMBER_TRAMPOLINE(name) (void **)(&name##Class::name##_Actual)

#define DETOUR_CREATE_MEMBER(name, detourname, signature, file)\
		g_pDETOUR_##name = CDetourContext::CreateDetour(GET_MEMBER_CALLBACK(name), GET_MEMBER_TRAMPOLINE(name), detourname, signature, file);\
		if (!g_pDETOUR_##name) return false;\
		g_pDETOUR_##name->EnableDetour();

#define DETOUR_DESTROY(name)\
		if (g_pDETOUR_##name) g_pDETOUR_##name->Destroy();\
		g_pDETOUR_##name = nullptr;

#endif
