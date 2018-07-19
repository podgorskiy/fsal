#include "Archive.h"

using namespace fsal;

bool Archive::Valid() const
{
	return m_impl != nullptr;
}

File Archive::Open(const fs::path& filepath)
{
	return false;
}

bool Archive::Exists(const fs::path& filepath, PathType type)
{
	return false;
}

Status Archive::AddFile(const fs::path& path, File file, int compression)
{
	return false;
}

Status Archive::CreateDirectory(const fs::path& path)
{
	return false;
}
