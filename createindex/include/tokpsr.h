// tokpsr.h

#ifndef __tokpsr_h__
#define __tokpsr_h__

#include <stdlib.h>

#define TOKPSR_SEP		" \t\n\r"
#define TOKPSR_SINGLE	""

extern const char* TOKPSR_TOKEN[];

bool is_token(const char* token, const char* p);
const char* get_next_token(char* buff, size_t size, const char* p, const char* single = TOKPSR_SINGLE, const char* sep = TOKPSR_SEP);
const char* passby_token(const char* token, const char* p, const char* single = TOKPSR_SINGLE, const char* sep = TOKPSR_SEP);

const char* get_next_token(char* buff, size_t size, const char* p, const char* tokens[], const char* sep = TOKPSR_SEP);
const char* passby_token(const char* token, const char* p, const char* tokens[], const char* sep = TOKPSR_SEP);

#endif

