// tokpsr.cpp
//#include "stdafx.h"
#include "tokpsr.h"
#include <string.h>

const char* TOKPSR_TOKEN[] = {  NULL };


bool is_token(const char* token, const char* p)
{
	return strcmp(token, p)==0;
}


const char* is_defined_token(const char* buff, const char* tokens[])
{
	register size_t i = 0;
	while (tokens[i]) {
		if (strncmp(tokens[i], buff, strlen(tokens[i]))==0) {
			return tokens[i];
		}
		i++;
	}
	return NULL;
}

const char* get_next_token(char* buff, size_t size, const char* p, const char* tokens[], const char* sep)
{
	// Skip prefix blanks
	// Notes:
	//		The char '\0' will be find by strchr!
	while (strchr(sep, *p) && *p) {
		p++;
	}
	if (!(*p)) {
		return NULL;
	}

	// Get the token
	size_t i = 0;
	const char* tok = is_defined_token(p, tokens);
	if (tok) {	// Single char token
		while (tok[i]) {
			if (i<size-1) {
				buff[i] = tok[i];
				i++;
			}
			p++;
		}
		buff[i] = 0;
	} else {
		while (!strchr(sep, *p) && !is_defined_token(p, tokens) && *p) {
			if (i<size-1) {
				buff[i] = *p;
				i++;
			}
			p++;
		}
	}
	buff[i] = 0;

	// Skip sufix blanks
	while (strchr(sep, *p) && *p) {
		p++;
	}

	return p;
}


const char* get_next_token(char* buff, size_t size, const char* p, const char* single, const char* sep)
{
	// Skip prefix blanks
	// Notes:
	//		The char '\0' will be find by strchr!
	while (strchr(sep, *p) && *p) {
		p++;
	}
	if (!(*p)) {
		return NULL;
	}

	// Get the token
	size_t i = 0;
	if (strchr(single, *p) && *p) {	// Single char token
		buff[i] = *p;
		i++;
		p++;
	} else {
		while (!strchr(sep, *p) && !strchr(single, *p) && *p) {
			if (i<size-1) {
				buff[i] = *p;
				i++;
			}
			p++;
		}
	}
	buff[i] = 0;

	// Skip sufix blanks
	while (strchr(sep, *p) && *p) {
		p++;
	}

	return p;
}

const char* passby_token(const char* token, const char* p, const char* tokens[], const char* sep)
{
	size_t size = strlen(token)+1;
	char* buff = new char[size];

	if ((p = get_next_token(buff, size, p, tokens, sep))==NULL) {
		delete []buff;
		return NULL;
	}

	if (is_token(token, buff)) {
		delete []buff;
		return p;
	} else {
		delete []buff;
		return NULL;
	}
}

const char* passby_token(const char* token, const char* p, const char* single, const char* sep)
{
	size_t size = strlen(token)+1;
	char* buff = new char[size];

	if ((p = get_next_token(buff, size, p, single, sep))==NULL) {
		delete []buff;
		return NULL;
	}

	if (is_token(token, buff)) {
		delete []buff;
		return p;
	} else {
		delete []buff;
		return NULL;
	}
}

