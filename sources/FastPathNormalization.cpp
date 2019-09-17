#include "FastPathNormalization.h"

#include <cstring>


inline bool IsSlash(char a)
{
	return (a == '\\' || a == '/');
}

// If the char pointed by 'it' is not slash, then it is accepted
// If accepted:
//     - the char is written to 'c'
//     - true is returned
//     - 'it' is decremented
// Otherwise false is returned
// If 'it' points to the element before 'begin', 
// that means that we've already parssed the whole string, false is returned 
inline bool AcceptChar(const char* begin, const char*& it, char& c)
{
	if (it != begin - 1 && !IsSlash(*it))
	{
		c = *it;
		--it;
		return true;
	}
	return false;
}

// Same as above, but for slash characters
inline bool AcceptSlash(const char* src, const char*& p)
{
	if (p != src - 1 && IsSlash(*p))
	{
		--p;
		return true;
	}
	return false;
}

// Accepts, if next two symbols are two dots which are not followed by a character
inline bool AcceptDotDotEntry(const char* src, const char*& p)
{
	if (p > src && *p == '.' && *(p - 1) == '.')
	{
		char c;
		const char* b = p;
		b -= 2;
		if (!AcceptChar(src, b, c))
		{
			p = b;
			return true;
		}
	}
	return false;
}

// Accepts, if next symbols is a dot which is not followed by a character
inline bool AcceptDotEntry(const char* src, const char*& p)
{
	if (p >= src && *p == '.')
	{
		char c;
		const char* b = p;
		b -= 1;
		if (!AcceptChar(src, b, c))
		{
			p = b;
			return true;
		}
	}
	return false;
}

void fsal::NormalizePath(const char* src, char* dst, size_t len, char*& filename, int& depth)
{
	char* dstIt = dst + len - 1;
	const char* srcIt = src + len - 1;

	depth = 0;
	int skipping = 0;

	char c = '\x0';
	bool hasLeadingSlash = false;
	bool firstEntryWasAccepted = false;

	for(;;)
	{
		if (AcceptSlash(src, srcIt))
		{
			hasLeadingSlash |= true & !firstEntryWasAccepted;
			++depth;
			*dstIt = skipping == 0 ? '/' : '\x0';
			--dstIt;
			while (AcceptSlash(src, srcIt));
		}
		else if (AcceptDotEntry(src, srcIt))
		{
			--depth;
			++dstIt;
		}
		else if (AcceptDotDotEntry(src, srcIt))
		{
			--depth;
			--depth;
			++dstIt;
			++skipping;
		}
		else if (AcceptChar(src, srcIt, c))
		{
			*dstIt = skipping == 0 ? c : '\x0';
			--dstIt;

			while (AcceptChar(src, srcIt, c))
			{
				*dstIt = skipping == 0 ? c : '\x0';
				--dstIt;
			}
			skipping -= skipping > 0;
		}
		else
		{
			break;
		}
		firstEntryWasAccepted = true;
	}
	depth -= hasLeadingSlash ? 1 : 0;

	++dstIt;

	char* p = dst + len;

	if (dstIt != dst)
	{
		char* end = dst + len;
		while (dstIt < end)
		{
			if (*dstIt != '\x0')
			{
				*dst = *dstIt;
				++dst;
			}
			++dstIt;
		}
		*dst = '\x0';
		end = dst;
	}

	--p;

	while (AcceptSlash(dst, (const char*&)p));
	while (AcceptChar(dst, (const char*&)p, c));
	++p;
	filename = p;
}

fsal::fs::path fsal::NormalizePath(const fs::path& src)
{
	return fs::u8path(NormalizePath(src.u8string()));
}

std::string fsal::NormalizePath(const char* src)
{
	std::string dst;
	size_t len = strlen(src);
	dst.resize(len);
	char* c_dst = &dst[0];
	char* filename = nullptr;
	int depth = 0;
	NormalizePath(src, c_dst, len, filename, depth);
	dst.resize(strlen(c_dst));
	return dst;
}

std::string fsal::NormalizePath(const std::string& src)
{
	int depth = 0;
	int filenamePos = 0;
	std::string dst;
	NormalizePath(src, dst, filenamePos, depth);
	return dst;
}

void fsal::NormalizePath(const std::string src, std::string& dst, int& filenamePos, int& depth)
{
	dst.resize(src.length());
	const char* c_src = src.c_str();
	char* c_dst = &dst[0];
	char* filename = nullptr;
	NormalizePath(c_src, c_dst, src.length(), filename, depth);
	filenamePos = (int)(filename - c_dst);
	dst.resize(strlen(c_dst));
}

void fsal::NormalizePath(const fs::path _src, fs::path& _dst, int& filenamePos, int& depth)
{
	std::string dst;
	std::string src = _src.u8string();
	dst.resize(src.length());
	const char* c_src = src.c_str();
	char* c_dst = &dst[0];
	char* filename = nullptr;
	NormalizePath(c_src, c_dst, src.length(), filename, depth);
	filenamePos = (int)(filename - c_dst);
	dst.resize(strlen(c_dst));
	_dst = fs::u8path(dst);
}
