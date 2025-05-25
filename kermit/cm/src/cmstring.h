#ifndef CMSTRING_H
#define CMSTRING_H

#include <windows.h>

// A *simple* string class with support for unicode on Windows NT
// including translation to-from wchar and UTF-8. Used because
// The STL string class isn't necessarily available on the compilers
// required to target RISC NT (Visual C++ 4.0)

typedef struct tagCMStringData CMStringData;

class CMString {
public:
	CMString();

	CMString(LPCTSTR str);

	CMString(const CMString &other);

	CMString &operator=(const CMString &rhs);

	~CMString();

	static CMString fromUtf8(char* str);

	static CMString number(int i, int base=10);

	// Returns the strings length in characters, excluding null termination.
	size_t length() const;

	// Returns the raw data held by the string. The caller can modify it
	// directly if required.
	LPTSTR data();

	// Returns a deep copy fo the string.
	//CMString copy();

	// Returns the string in UTF-8 format with the length of the allocated
	// string (in bytes) returned via lengthBytes. The caller is responsible
	// for deleting the returned buffer when finished with it. If this string
	// is NULL (isNull() returns TRUE), NULL is returned.
	char* toUtf8(size_t *lengthBytes) const;
	
	BOOL isNull() const;

	BOOL isNullOrWhiteSpace() const;

	BOOL contains(TCHAR c) const;

	// TODO: return a copy of this string with any UNIX line endings
	//       replaced with CR-LF
	CMString toCRLF() const { return *this; }

	// todo: void strip();

	//CMString &operator+=(const CMString &other);

	//friend CMString operator+(CMString lhs, const CMString &rhs);
	friend BOOL operator==(const CMString &lhs, const CMString &rhs);
	friend BOOL operator!=(const CMString &lhs, const CMString &rhs);

private:
	CMStringData *_data;
};

#endif /* CMSTRING_H */