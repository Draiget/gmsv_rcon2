#pragma once

#ifdef _PLATFORM_LINUX
#define SIG_C_SERVERREMOTEACCESS_IS_PASSWORD \
		"\\x55\\x89\\x2A\\x83\\x2A\\x2A\\x8B\\x2A\\x2A\\x8B\\x2A\\x2A\\x85\\x2A\\x74\\x2A\\x80\\x2A\\x2A\\x75\\x2A\\xC9\\x31\\x2A\\xC3\\x8D\\x2A\\x2A\\x2A\\x2A\\x2A\\x2A\\x83"

// Raw: 55 89 ?? 83 ?? ?? 8B ?? ?? 8B ?? ?? 85 ?? 74 ?? 80 ?? ?? 75 ?? C9 31 ?? C3 8D ?? ?? ?? ?? ?? ?? 83
// In: engine.so

#define SIG_C_SERVERREMOTEACCESS_WIRTE_DATA_REQUEST \
		"\\x55\\x89\\x2A\\x57\\x56\\x53\\x81\\x2A\\x2A\\x2A\\x2A\\x2A\\x65\\x2A\\x2A\\x2A\\x2A\\x2A\\x2A\\x89\\x2A\\x2A\\x31\\x2A\\x8B\\x2A\\x2A\\x8B\\x2A\\x2A\\x89\\x2A\\x89\\x2A\\x2A\\x2A\\x2A\\x2A\\x8B"

// Raw: 55 89 ?? 57 56 53 81 ?? ?? ?? ?? ?? 65 ?? ?? ?? ?? ?? ?? 89 ?? ?? 31 ?? 8B ?? ?? 8B ?? ?? 89 ?? 89 ?? ?? ?? ?? ?? 8B
// In: engine_srv.so (engine.so contains different op-code)

#define SIG_C_SERVERREMOTEACCESS_LOG_COMMAND \
		"\\x55\\x89\\x2A\\x53\\x83\\x2A\\x2A\\x8B\\x2A\\x2A\\x2A\\x2A\\x2A\\x8B\\x2A\\x2A\\x8B\\x2A\\x2A\\x8B\\x2A\\x2A\\x8B\\x2A\\x2A\\x85"

// Raw: 55 89 ?? 53 83 ?? ?? 8B ?? ?? ?? ?? ?? 8B ?? ?? 8B ?? ?? 8B ?? ?? 8B ?? ?? 85
// In: engine.so
#else
#define SIG_C_SERVERREMOTEACCESS_IS_PASSWORD \
		"\\x55\\x8B\\x2A\\x8B\\x2A\\x2A\\x83\\x2A\\x2A\\x85\\x2A\\x74\\x2A\\x80\\x2A\\x2A\\x74\\x2A\\xE8\\x2A\\x2A\\x2A\\x2A"

#define SIG_C_SERVERREMOTEACCESS_WIRTE_DATA_REQUEST \
		"\\x55\\x8B\\x2A\\x81\\x2A\\x2A\\x2A\\x2A\\x2A\\x8B\\x2A\\x2A\\x56\\x8B\\x2A\\x89\\x2A\\x2A\\x01\\x2A\\x2A"

#define SIG_C_SERVERREMOTEACCESS_LOG_COMMAND \
		"\\x55\\x8B\\x2A\\xA1\\x2A\\x2A\\x2A\\x2A\\x8B\\x2A\\x83\\x2A\\x2A\\x2A\\x74\\x2A\\x8B\\x2A\\x2A\\x3B\\x2A\\x2A\\x73"

// Raw: 55 8B ?? A1 ?? ?? ?? ?? 8B ?? 83 ?? ?? ?? 74 ?? 8B ?? ?? 3B ?? ?? 73
// NOTE: While function is still exists in code, WriteDataRequest didn't use it,
// looks like it was force-inlined. Therefore need to insert jump at switch case.
// NOTE: This issue is only on Windows engine.dll

#define SIG_C_SERVERREMOTEACCESS_LOG_COMMAND_IN \
		"\\x8B\\x2A\\x2A\\x2A\\x2A\\x2A\\x83\\x2A\\x2A\\x83\\x2A\\x2A\\x2A\\x74\\x2A\\x3B\\x2A\\x2A\\x73\\x2A\\x8B\\x2A\\x2A\\x80"

// Raw: 8B ?? ?? ?? ?? ?? 83 ?? ?? 83 ?? ?? ?? 74 ?? 3B ?? ?? 73 ?? 8B ?? ?? 80

#define SIG_C_SERVERREMOTEACCESS_LOG_COMMAND_OUT \
		"\\x68\\x2A\\x2A\\x2A\\x2A\\x8D\\x2A\\x2A\\x2A\\x2A\\x2A\\xE9\\x2A\\x2A\\x2A\\x2A\\xE8"

// Raw: 68 ?? ?? ?? ?? 8D ?? ?? ?? ?? ?? E9 ?? ?? ?? ?? E8
// NOTE: Ends of LogCommand block `if ( listener != <REG> )`

#endif
