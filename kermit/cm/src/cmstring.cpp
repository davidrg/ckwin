#include "cmstring.h"

#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#include "util.h"

// UTF-8 requires Windows NT 4.0 or Windows 98 or newer.
// Its definition doesn't appear in Visual C++ 4.0, but it is
// present in 4.2. Included here for compatibility with 4.0.
#ifndef CP_UTF8
#define CP_UTF8              65001          /* UTF-8 translation */
#endif /* CP_UTF8 */

typedef struct tagCMStringData {
	size_t length;
	LPTSTR string;
	int refCount;
	BOOL isNull;
} CMStringData;

//////////// Constructors, Destructor ////////////

CMString::CMString() {
	_data = (CMStringData*)malloc(sizeof(CMStringData));
	ZeroMemory(_data, sizeof(CMStringData));

	_data->refCount = 1;
	_data->length = 0;
	_data->isNull = TRUE;
	_data->string = NULL;

	// We'll still allocate a null terminated string just so that
	// callers of data() don't always have to check if NULL.
	_data->string = (LPTSTR)malloc(sizeof(TCHAR));
	ZeroMemory(_data->string, sizeof(TCHAR));
}


CMString::CMString(LPCTSTR str) {
	if (str == NULL) {
		InitFromTSTR(NULL, 0);
	} else {
		InitFromTSTR(str, _tcsclen(str));
	}
}

CMString::CMString(LPCTSTR str, int allocateLength) {
	InitFromTSTR(str, allocateLength);
}

CMString::CMString(const CMString &other) {
	_data = other._data;
	_data->refCount++;
}

CMString::~CMString() {
	_data->refCount -=1;

	if (_data->refCount == 0) {
		// We're the last CMString instance left holding a reference to
		// this data. Delete it.
		free(_data->string);
		free(_data);
	}
}


void CMString::InitFromTSTR(LPCTSTR str, int allocateLength) {
	_data = (CMStringData*)malloc(sizeof(CMStringData));
	ZeroMemory(_data, sizeof(CMStringData));

	_data->refCount = 1;
	
	if (str == NULL) {
		_data->length = 0;
		_data->isNull = TRUE;

		// We'll still allocate a null terminated string just so that
		// callers of data() don't always have to check if NULL.
		_data->string = (LPTSTR)malloc(sizeof(TCHAR));
		ZeroMemory(_data->string, sizeof(TCHAR));
	} else {
		LPTSTR buf;
		int len = _tcsclen(str);
		if (allocateLength < len) allocateLength = len;
		int buflen = sizeof(TCHAR) * (allocateLength+1);

		buf = (LPTSTR)malloc(buflen);
		ZeroMemory(buf, buflen);

		_tcsnccpy(buf, str, len);

		_data->isNull = FALSE;
		_data->length = len;
		_data->string = buf;
	}
}


CMString CMString::fromUtf8(char* str) {

	if (str == NULL) {
		return CMString(NULL);
	}

#ifdef UNICODE
	LPTSTR buf;

	int lengthRequired = MultiByteToWideChar(
		CP_UTF8, 
		0,			// Must be 0 for CP_UTF8
		str,
		-1,			// -1 for null terminated string
		buf,
		0);

	buf = (LPTSTR)malloc(lengthRequired * sizeof(TCHAR));

	MultiByteToWideChar(
		CP_UTF8, 
		0,			// Must be 0 for CP_UTF8
		str,
		-1,			// -1 for null terminated string
		buf,
		lengthRequired);

	CMString result(buf);

	free(buf);

	return result;
#else
	// We don't have unicode support so there isn't much we can
	// do with a UTF-8 string. Just copy the data as-is and it
	// just consists of ASCII characters.
	return CMString(str);
#endif 
}

#ifndef UNICODE
wchar_t* CMString::toUtf16() {
	wchar_t* buf = NULL;

	int lengthRequired = MultiByteToWideChar(
		CP_UTF8,
		0,			// Must be 0 for CP_UTF8
		data(),
		-1,			// -1 for null terminated string
		buf,
		0);

	buf = (wchar_t*)malloc(lengthRequired * sizeof(wchar_t));

	MultiByteToWideChar(
		CP_UTF8,
		0,			// Must be 0 for CP_UTF8
		data(),
		-1,			// -1 for null terminated string
		buf,
		lengthRequired);

	return buf;
}
#endif

CMString CMString::number(int i, int base) {
	LPTSTR buffer[33];
	ZeroMemory(buffer, 33 * sizeof(TCHAR));

	_itot(i, (LPTSTR)buffer, base);

	return CMString((LPTSTR)buffer);
}

//////////// Operators ////////////

CMString &CMString::operator=(const CMString &rhs) {
	_data->refCount -= 1;
	if (_data->refCount == 0) {
		free(_data->string);
		free(_data);
	}

	_data = rhs._data;
	_data->refCount++;

	return *this;
}

/*CMString &operator+=(const CMString &other) {
	int len_required = length() + other.length() + 1;

}*/

CMString operator+(CMString lhs, const CMString &rhs) {
	CMString rhsCopy = CMString(rhs);

	int len = lhs.length() + rhsCopy.length();
	
	
	CMString result = CMString(lhs.data(), len);
	lstrcat(result._data->string, rhsCopy.data());
	result._data->length = _tcsclen(result._data->string);

	return result;
}

BOOL operator==(const CMString &lhs, const CMString &rhs) {
	if (lhs.isNull() && rhs.isNull()) return TRUE;
	if (lhs.isNull() || rhs.isNull()) return FALSE;

	return lstrcmp(lhs._data->string, rhs._data->string) == 0;
}

BOOL operator!=(const CMString &lhs, const CMString &rhs) {
	return !(lhs == rhs);
}

//////////// Utility functions ////////////

size_t CMString::length() const {
	return _data->length;
}

LPTSTR CMString::data() {
	return _data->string;
}

/*CMString CMString::copy() {
	return CMString(data());
}*/

char* CMString::toUtf8(size_t *lengthBytes) const {
	char* buf;

	if (isNull()) return NULL;

#ifdef UNICODE
	int lengthReqd = WideCharToMultiByte(
		CP_UTF8,
		0,			// Must be 0 for CP_UTF8
		_data->string,
		-1,			// -1 for null terminated string
		buf,
		0,			// 0 to find out how long the output will be
		NULL,		// Must be NULL for CP_UTF8
		NULL		// Must be NULL for CP_UTF8
		);

	buf = (char*)malloc(lengthReqd * sizeof(char));

	WideCharToMultiByte(
		CP_UTF8,
		0,			// Must be 0 for CP_UTF8
		_data->string,
		-1,			// -1 for null terminated string
		buf,
		lengthReqd,	// 0 to find out how long the output will be
		NULL,		// Must be NULL for CP_UTF8
		NULL		// Must be NULL for CP_UTF8
		);

	if (lengthBytes != NULL) {
		*lengthBytes = lengthReqd;
	}
	return buf;
#else
	// We're not dealing with unicode data - we've got either ASCII
	// or some other DBCS encoding. No conversion available - just
	// return a copy of the data we're holding.

 	buf = (char*)malloc((_data->length + 1) * sizeof(char));
	_tcsnccpy(buf, _data->string, _data->length + 1);
	
	if (lengthBytes != NULL) {
		*lengthBytes = (_data->length + 1) * sizeof(char);
	}
	return buf;
#endif
}

BOOL CMString::contains(TCHAR c) const {
	if (isNull()) return FALSE;

	return _tcschr(_data->string, c) != 0;
}

BOOL CMString::isNull() const {
	return _data->isNull;
}

BOOL CMString::isNullOrWhiteSpace() const {
	if (isNull()) return TRUE;

	for (unsigned int i = 0; i < _data->length; i++) {
		switch(_data->string[i]) {
		case _T(' '):
		case _T('\n'):
		case _T('\r'):
		case _T('\f'):
		case _T('\t'):
		case _T('\v'):
			continue;
		}
		return FALSE;
	}

	return TRUE;
}