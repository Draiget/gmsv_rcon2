#ifndef ZONTWELG_GMSVRCON2_DETOUR_H
#define ZONTWELG_GMSVRCON2_DETOUR_H

struct DetourPatch_t
{
	DetourPatch_t()
	{
		patch[0] = 0;
		bytes = 0;
	}

	unsigned char patch[20];
	size_t bytes;
};

class CDetourContext 
{
public:
	static CDetourContext *CreateDetour(void *pDetourCallbackFn, void **pTrampoline, const char *pDetourName, const char *pSignature, const char *pLibName);

	bool IsEnabled() const;
	void EnableDetour();
	void DisableDetour();
	void Destroy();

	void *m_Detour_Address;
	void *m_Detour_Trampoline;
	void *m_Detour_Callback;

protected:
	CDetourContext(void *pDetourCallbackFn, void **pTrampoline, const char *pDetourName, const char *pSignature, const char *pLibName);
	bool Init();

private:
	void Rewind(char* pOutbase = nullptr, char* pOutptr = nullptr);
	bool CreateDetour();
	void DeleteDetour();

	bool m_Enabled;
	bool m_Detoured;
	DetourPatch_t m_Detour_Restore;
	void **m_Trampoline;
	const char *m_Detour_Name;
	const char *m_Detour_Signature;
	const char *m_Detour_LibName;
};

class GenericClass {};

typedef void (GenericClass::*VoidFunc)();

inline void *GetCodeAddr(VoidFunc mfp) {
	return *reinterpret_cast<void **>( &mfp );
}

#define GetCodeAddress(mfp) \
		GetCodeAddr(reinterpret_cast<VoidFunc>(mfp))

#endif
