#pragma once
#include "ArchiveInterface.h"
#include <memory>

namespace fsal
{
	class Archive
	{
	public:
		Archive();

		Archive(ArchiveReaderInterfacePtr reader);

		bool Valid() const;

		File OpenFile(const fs::path& filepath);

		bool Exists(const fs::path& filepath, PathType type = kFile | kDirectory);

		Status AddFile(const fs::path& path, File file, int compression);

		Status CreateDirectory(const fs::path& path);

		std::vector<std::string> ListDirectory(const fs::path& path);
	private:
		ArchiveReaderInterfacePtr m_impl;
	};
}
