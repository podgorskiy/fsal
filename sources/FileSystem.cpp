#include "fsal.h"
#include "StdFile.h"
#include "FastPathNormalization.h"
#include "ZipArchive.h"

#include <vector>
#include <functional>
#include <mutex>

struct fsal::FsalImplementation
{
	std::vector<Archive> archives;
	std::vector<path> searchPaths;
	std::mutex searchPathsMutex;
};

using namespace fsal;

#define FSAL_SINGLETON

#if defined FSAL_SINGLETON

FileSystem::FileSystem()
{
	static std::shared_ptr<FsalImplementation> ptr(new FsalImplementation());
	m_impl = ptr;
}
#else
FileSystem::FileSystem()
{
	m_impl.reset(new FsalImplementation());
}
#endif

void fsal::FileSystem::PushSearchPath(const Location& location)
{
	std::lock_guard<std::mutex> lock(m_impl->searchPathsMutex);
	m_impl->searchPaths.push_back(NormalizePath(location.GetFullPath()));
}

void fsal::FileSystem::PopSearchPath()
{
	std::lock_guard<std::mutex> lock(m_impl->searchPathsMutex);
	if (m_impl->searchPaths.size() > 0)
	{
		m_impl->searchPaths.pop_back();
	}
}

void fsal::FileSystem::ClearSearchPaths()
{
	std::lock_guard<std::mutex> lock(m_impl->searchPathsMutex);
	m_impl->searchPaths.clear();
}

Status fsal::FileSystem::MountArchive(const File& archive)
{
	if (archive)
	{
		Archive archiveReader(ArchiveReaderInterfacePtr((ArchiveReaderInterface*)new ZipReader()));
		if (archiveReader.OpenArchive(archive))
		{
			m_impl->archives.push_back(archiveReader);
			return true;
		}
	}
	return false;
}

static bool CheckAttributes(PathType type, LinkType link, const path& fullPath)
{
	bool is_regular_file = fs::is_regular_file(fullPath);
	bool is_directory = fs::is_directory(fullPath);
	bool is_symlink = fs::is_symlink(fullPath);

	if (0
	    || (type & kFile) && is_regular_file
	    || (type & kDirectory) && is_directory
			)
	{
		if (0
		    || (link & kSymlink) && is_symlink
		    || (link & kNotSymlink) && !is_symlink
				)
		{
			return true;
		}
	}
	return false;
}

Status fsal::FileSystem::Find(const Location& location, path& absolutePath, PathType& type, Archive& archive)
{
	archive = Archive();

	// Return failur if the path is specified as absolute, but the path is not recognized as absolute
	if (location.m_relartiveTo == Location::kAbsolute && !location.m_filepath.is_absolute())
	{
		absolutePath = "";
		archive = Archive();
		return Status::Failed();
	}

	// First check. Current directory or system path. Performed if "Archives" is not specified.
	if (location.m_relartiveTo != Location::kArchives)
	{
		path fullpath = location.GetFullPath();

		if (fs::exists(fullpath) && CheckAttributes(location.m_type, location.m_link, fullpath))
		{
			type = fs::is_directory(fullpath) ? kDirectory : kFile;
			absolutePath = fullpath;
			archive = Archive();
			return Status::Succeeded();
		}
	}

	// Second check. Search paths
	if (location.m_relartiveTo == Location::kCurrentDirectory || location.m_relartiveTo == Location::kSearchPaths || location.m_relartiveTo == Location::kSearchPathsAndArchives)
	{
		std::lock_guard<std::mutex> lock(m_impl->searchPathsMutex);
		for (std::vector<path>::iterator it = m_impl->searchPaths.begin(), end = m_impl->searchPaths.end(); it != end; ++it)
		{
			fsal::Location absoluteLocation(location);
			absoluteLocation.m_filepath = *it / location.m_filepath;
			absoluteLocation.m_relartiveTo = Location::Options::kAbsolute;

			path fullpath = absoluteLocation.GetFullPath();

			if (fs::exists(fullpath))
			{
				if (CheckAttributes(location.m_type, location.m_link, fullpath))
				{
					type = fs::is_directory(fullpath) ? kDirectory : kFile;
					absolutePath = fullpath;
					archive = Archive();
					return Status::Succeeded();
				}
			}
		}
	}

	// Third check. Archives
	if (location.m_relartiveTo == Location::kArchives || location.m_relartiveTo == Location::kSearchPathsAndArchives)
	{
		std::lock_guard<std::mutex> lock(m_impl->searchPathsMutex);
		for (std::vector<Archive>::iterator it = m_impl->archives.begin(), end = m_impl->archives.end(); it != end; ++it)
		{
			if (it->Exists(location.m_filepath, location.m_type))
			{
				type = location.m_type;
				absolutePath = location.m_filepath;
				archive = *it;
				return Status::Succeeded();
			}
		}

		absolutePath = "";
		archive = Archive();
		return Status::Failed();
	}

	return Status::Failed();
}

File fsal::FileSystem::Open(const Location& location, Mode mode)
{
	PathType type;
	path absolutePath;
	Archive archive;

	if (!Find(location, absolutePath, type, archive).ok())
	{
		if (mode == Mode::kWrite || mode == Mode::kWriteUpdate)
		{
			Location locationOfParentFolder(location.m_filepath.parent_path(), location.m_relartiveTo, kDirectory);

			if (Find(locationOfParentFolder, absolutePath, type, archive).ok())
			{
				absolutePath = absolutePath / location.m_filepath.filename();
				type = location.m_type;
			}
			else
			{
				return File();
			}
		}
		else
		{
			return File();
		}

	}

	if (type == kDirectory)
	{
		return File();
	}

	if (archive.Valid())
	{
		return archive.OpenFile(absolutePath);
	}
	else
	{
		StdFile* stdf = new StdFile();
		stdf->Open(absolutePath, mode);
		if (stdf->ok())
		{
			return File(stdf);
		}
		else
		{
			return File();
		}
	}
}


bool fsal::FileSystem::Exists(const Location& location)
{
	PathType type;
	path absolutePath;
	Archive archive;
	return Find(location, absolutePath, type, archive).ok();
}


Status fsal::FileSystem::Rename(const Location& srcLocation, const Location& dstLocation)
{
	if (srcLocation.m_relartiveTo == Location::kAbsolute && !srcLocation.m_filepath.is_absolute())
	{
		return Status::Failed();
	}
	if (dstLocation.m_relartiveTo == Location::kAbsolute && !dstLocation.m_filepath.is_absolute())
	{
		return Status::Failed();
	}

	PathType type;
	path absolutePathSrc;
	Archive archiveSrc;
	path absolutePathDst;
	Archive archiveDst;

	if (Find(srcLocation, absolutePathSrc, type, archiveSrc).ok())
	{
		Location locationOfParentFolder(dstLocation.m_filepath.parent_path(), dstLocation.m_relartiveTo, kDirectory);

		if (Find(locationOfParentFolder, absolutePathDst, type, archiveDst).ok())
		{
			if (archiveSrc.Valid() || archiveDst.Valid())
			{
				return Status::Failed();
			}
			else
			{
				std::error_code error_code;
				fs::rename(absolutePathSrc, absolutePathDst / dstLocation.m_filepath.filename(), error_code);
				return error_code.value() == 0;
			}
		}
	}

	return Status::Failed();
}


Status fsal::FileSystem::Remove(const Location& location)
{
	PathType type;
	path absolutePath;
	Archive archive;
	if (Find(location, absolutePath, type, archive).ok())
	{
		if (archive.Valid())
		{
			return Status::Failed();
		}
		else
		{
			if (type == kDirectory)
			{
				return fs::remove_all(absolutePath) != 0;
			}
			else
			{
				return fs::remove(absolutePath);
			}
		}
	}

	return Status::Failed();
}


Status fsal::FileSystem::CreateDirectory(const Location& location)
{
	PathType type;
	path absolutePath;
	Archive archive;
	if (Find(location, absolutePath, type, archive).ok())
	{
		return Status::Failed();
	}
	else
	{
		Location locationOfParentFolder(location.m_filepath.parent_path(), location.m_relartiveTo, kDirectory);

		if (Find(locationOfParentFolder, absolutePath, type, archive).ok())
		{
			if (archive.Valid())
			{
				return Status::Failed();
			}
			else
			{
				return fs::create_directory(absolutePath / location.m_filepath.filename());
			}
		}
		else
		{
			return Status::Failed();
		}
	}
}