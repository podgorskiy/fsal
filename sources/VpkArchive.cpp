#include "bfio.h"
#include "fsal_common.h"
#include "VpkArchive.h"
#include "FileStream.h"
#include "MemRefFile.h"
#include <cassert>
#include <zlib.h>


using namespace fsal;


static std::string read_string(fsal::File file)
{
	std::string buf;

	char chunk[64];
	while (true)
	{
		file.Read((uint8_t*)chunk, 64);
		buf += chunk;
		int i = 0;
		for (; i < 64 && chunk[i] != 0; ++i);
		if (chunk[i] == 0)
		{
			file.Seek(file.Tell() - (64 - (i + 1)));
			break;
		}
	}
	return buf;
}


Status VPKReader::OpenArchive(FileSystem fs, Location directory, const std::string& formatString)
{
	m_formatString = formatString;
	m_directory = std::move(directory);
	char buff[2048];
	sprintf(buff, m_formatString.c_str(), "dir");
	m_index = fs.Open(m_directory / buff);

	VPKHeader_v2 header;

	m_index.Read((uint8_t*)&header, sizeof(uint32_t) * 3);
	assert(header.Signature == VPK_SIGNATURES::HEADER);
	assert(header.Version == 1 || header.Version == 2);

	if (header.Version == 2)
	{
		m_index.Seek(0);
		m_index.Read(header);
	}

	printf( "Signature: 0x%08x\n", header.Signature);
	printf( "Version: %d\n", header.Version);
	printf( "Directory length: %d\n", header.TreeSize);

	size_t tree_begin_ptr = m_index.Tell();

	while (true)
	{
		assert(m_index.Tell() - tree_begin_ptr < header.TreeSize);

		auto ext = read_string(m_index);

		if (ext.empty())
		{
			break;
		}

		while (true)
		{
			auto path = read_string(m_index);

			if (path.empty())
			{
				break;
			}
			if (path == " ")
				path = "";

			while (true)
			{
				auto name = read_string(m_index);

				if (name.empty())
				{
					break;
				}
				VPKDirectoryEntry entryHeader;
				m_index.Read(entryHeader);

				assert(entryHeader.Terminator == VPK_SIGNATURES::DIRECTORY_ENTRY_TERMINATOR);

				VpkEntryData entry = {0};
				entry.PreloadBytes = entryHeader.PreloadBytes;
				entry.ArchiveIndex = entryHeader.ArchiveIndex;
				entry.EntryOffset = entryHeader.EntryOffset;
				entry.EntryLength = entryHeader.EntryLength;
				entry.preloadData = nullptr;

				if (entryHeader.PreloadBytes > 0)
				{
					entry.preloadData = (uint8_t*) malloc(entryHeader.PreloadBytes);
					m_index.Read(entry.preloadData, entryHeader.PreloadBytes);
				}
				//printf("%s/%s.%s\n", path.c_str(), name.c_str(), ext.c_str());

				sprintf(buff, "%s/%s.%s", path.c_str(), name.c_str(), ext.c_str());
				filelist.Add(entry, buff);
			}
		}
	}

	FileEntry<VpkEntryData> key("");
	filelist.GetIndex(key);
	return true;
}

File VPKReader::OpenPak(int index)
{
	auto it = m_pak_files.find(index);
	if (it != m_pak_files.end())
	{
		return it->second;
	}
	char buff[2048];
	char buff2[32];
	sprintf(buff2, "%03d", index);
	sprintf(buff, m_formatString.c_str(), buff2);
	File file = m_fs.Open(m_directory / buff);
	m_pak_files[index] = file;
	return file;
}

File VPKReader::OpenFile(const fs::path& filepath)
{
	VpkEntryData entry = filelist.FindEntry(filepath);

	File file;
	uint32_t offset = entry.EntryOffset;

	if (entry.ArchiveIndex == 0x7fff)
	{
		file = m_index;
	}
	else
	{
		file = OpenPak(entry.ArchiveIndex);
	}

	if (entry.PreloadBytes != 0 || entry.EntryOffset != 0)
	{
		//if (entry.PreloadBytes != 0 || entry.PreloadBytes + entry.EntryLength < 1024 * 1024)
		{
			auto* memfile = new MemRefFile();
			memfile->Resize(entry.PreloadBytes + entry.EntryLength);
			auto* data = memfile->GetDataPointer();
			memcpy(data, entry.preloadData, entry.PreloadBytes);

			if (entry.EntryLength > 0)
			{
				m_fileMutex.lock();
				file.Seek(offset, File::Beginning);
				file.Read((uint8_t*) data + entry.PreloadBytes, entry.EntryLength);
				m_fileMutex.unlock();
			}
			return memfile;
		}
		//else
		//{

		//}
	}

	return File();
}


bool VPKReader::Exists(const fs::path& filepath, PathType type)
{
	std::string path = filepath.u8string();
	if (path.back() != '/' && type == PathType::kDirectory)
	{
		path += "/";
	}

	FileEntry<VpkEntryData> key(path);

	return filelist.Exists(key);
}

std::vector<std::string> VPKReader::ListDirectory(const fs::path& path)
{
	return filelist.ListDirectory(path);
}
