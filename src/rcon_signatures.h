#ifndef ZONTWELG_GMSVRCON2_RCON_SIGNATURES_H
#define ZONTWELG_GMSVRCON2_RCON_SIGNATURES_H

#define SIG_CSERVERREMOTEACCESS_CHECKPASSWORD \
		"\\x55\\x8B\\x2A\\x56\\x8B\\x2A\\x2A\\x57\\xFF\\x2A\\x2A\\x8B\\x2A\\x8B\\x2A\\xE8\\x2A\\x2A\\x2A\\x2A\\x84\\x2A\\x75\\x2A\\xFF"

#define SIG_CSERVERREMOTEACCESS_ISPASSWORD \
		"\\x55\\x8B\\x2A\\x8B\\x2A\\x2A\\x83\\x2A\\x2A\\x85\\x2A\\x74\\x2A\\x80\\x2A\\x2A\\x74\\x2A\\xE8\\x2A\\x2A\\x2A\\x2A\\x8B\\x2A\\x2A\\x8D"

#define SIG_CSERVERREMOTEACCESS_WRITEDATAREQUEST \
		"\\x55\\x8B\\x2A\\x8B\\x2A\\x2A\\x81\\x2A\\x2A\\x2A\\x2A\\x2A\\x53\\x8B\\x2A\\x01\\x2A\\x2A\\x83\\x2A\\x2A\\x0F"

#define SIG_CRCONSERVER_SETREQUESTID \
		"\\x55\\x8B\\x2A\\x51\\x8D\\x2A\\x2A\\x56\\x57\\x8B\\x2A\\x89\\x2A\\x2A\\xE8\\x2A\\x2A\\x2A\\x2A\\x8B\\x2A\\x33"

#endif