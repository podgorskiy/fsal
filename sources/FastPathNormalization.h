#pragma once
#include <string>

namespace fsal
{
	void NormalizePath(const std::string src, std::string& dst);

	void NormalizePath(const std::string src, std::string& dst, int& filenamePos, int& depth);

	// No memory allocations version. The 'char* dst' assumed to be already allocated and as large as 'const char* src'.
	void NormalizePath(const char* src, char* dst, int len, char*& filename, int& depth);
}
