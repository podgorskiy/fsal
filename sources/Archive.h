#pragma once
#include "ArchiveInterface.h"
#include <memory>

namespace fsal
{
	class Archive
	{
	public:
		bool Valid() const;

		File Open(const fs::path& filepath);

		bool Exists(const fs::path& filepath, PathType type = kFile | kDirectory);

		Status AddFile(const fs::path& path, File file, int compression);

		Status CreateDirectory(const fs::path& path);

	private:
		std::shared_ptr<ArchiveReaderInterface> m_impl;
	};
}
