#include "bfio.h"
#include "fsal_common.h"
#include "ZipArchive.h"
#include "FileStream.h"
#include "MemRefFile.h"
#include <cassert>

using namespace fsal;

namespace bfio
{
	template<class RW>
	inline void Serialize(RW& io, DataDescriptor& x)
	{
		io & x.CRC32;
		io & x.compressedSize;
		io & x.uncompressedSize;
	}
	template<class RW>
	inline void Serialize(RW& io, CentralDirectoryHeader& x)
	{
		io & x.centralFileHeaderSignature;
		io & x.versionMadeBy;
		io & x.versionNeededToExtract;
		io & x.generalPurposBbitFlag;
		io & x.compressionMethod;
		io & x.lastModFileTime;
		io & x.lastModFileDate;
		io & x.dataDescriptor;
		io & x.fileNameLength;
		io & x.extraFieldLength;
		io & x.fileCommentLength;
		io & x.diskNumberStart;
		io & x.internalFileAttributes;
		io & x.externalFileAttributes;
		io & x.relativeOffsetOfLocalHeader;
	}
	template<class RW>
	inline void Serialize(RW& io, EndOfCentralDirectoryRecord& x)
	{
		io & x.endOfCentralDirSignature;
		io & x.numberOfThisDisk;
		io & x.numberOfTheDiskWithTheStartOfTheCentralDirectory;
		io & x.totalNumberOfEntriesInTheCentralDirectoryOnThisDisk;
		io & x.totalNumberOfEntriesInTheCentralDirectory;
		io & x.sizeOfTheCentralDirectory;
		io & x.offsetOfStartOfCentralDirectory;
		io & x.ZIPFileCommentLength;
	}
	template<class RW>
	inline void Serialize(RW& io, LocalFileHeader& x)
	{
		io & x.localFileHeaderSignature;
		io & x.versionNeededToExtract;
		io & x.generalPurposeBitFlag;
		io & x.compressionMethod;
		io & x.lastModFileTime;
		io & x.lastModFileDate;
		io & x.dataDescriptor;
		io & x.fileNameLength;
		io & x.extraFieldLength;
	}
}

Status ZipReader::OpenArchive(File file_)
{
	file = file_;

	FileStream stream(file);

	ptrdiff_t sizeOfCDEND = bfio::SizeOf<EndOfCentralDirectoryRecord>();

	file.Seek(-sizeOfCDEND, File::End);

	EndOfCentralDirectoryRecord ecdr;
	stream >> ecdr;

	if (ecdr.endOfCentralDirSignature != ZIP_SIGNATURES::END_OF_CENTRAL_DIRECTORY_SIGN)
	{
		return false;
	}

	file.Seek(ecdr.offsetOfStartOfCentralDirectory, File::Beginning);

	std::string filename;

	CentralDirectoryHeader header;
	LocalFileHeader fileHeader;

	for (int i = 0;; ++i)
	{
		stream >> header;

		assert(header.centralFileHeaderSignature == ZIP_SIGNATURES::CENTRAL_DIRECTORY_FILE_HEADER);

		size_t currPos = file.Tell();

		file.Seek(header.relativeOffsetOfLocalHeader, File::Beginning);

		stream >> fileHeader;

		assert(fileHeader.localFileHeaderSignature == ZIP_SIGNATURES::LOCAL_HEADER);

		filename.resize(fileHeader.fileNameLength);

		file.Read((uint8_t*)&filename[0], fileHeader.fileNameLength);

		//printf("%s\n", filename.data());

		ZipEntryData entry;
		entry.compressionMethod = fileHeader.compressionMethod;
		entry.generalPurposeBitFlag = fileHeader.generalPurposeBitFlag;
		entry.sizeUncompressed = fileHeader.dataDescriptor.uncompressedSize;
		entry.sizeCompressed = fileHeader.dataDescriptor.compressedSize;
		entry.offset = file.Tell();

		filelist.Add(entry, filename);

		file.Seek(currPos + header.fileNameLength + header.extraFieldLength + header.fileCommentLength, File::Beginning);

		if (ecdr.totalNumberOfEntriesInTheCentralDirectory >= 0)
		{
			if (i >= ecdr.totalNumberOfEntriesInTheCentralDirectory)
			{
				break;
			}
		}
		else
		{
			if (file.Tell() - ecdr.offsetOfStartOfCentralDirectory >= ecdr.sizeOfTheCentralDirectory)
			{
				break;
			}
		}
	}

	return true;
}


File ZipReader::OpenFile(const fs::path& filepath)
{
	ZipEntryData entry = filelist.FindEntry(filepath);

	if (entry.offset != -1)
	{
		auto* memfile = new MemRefFile();
		memfile->Resize(entry.sizeUncompressed);
		auto* data = memfile->GetDataPointer();
		file.Seek(entry.offset, File::Beginning);
		file.Read((uint8_t*)data, entry.sizeUncompressed);
		return memfile;
	}

	return File();
}

bool ZipReader::Exists(const fs::path& filepath, PathType type)
{
	std::string path = filepath.u8string();
	if (path.back() != '/' && type == PathType::kDirectory)
	{
		path += "/";
	}

	FileEntry<ZipEntryData> key(path);

	int index = filelist.GetIndex(key);

	return index != -1;
}

std::vector<std::string> ZipReader::ListDirectory(const fs::path& path)
{
	return filelist.ListDirectory(path);
}

Status ZipWriter::CreateArchive(File file)
{
	return false;
}

Status ZipWriter::AddFile(const fs::path& path, File file, int compression)
{
	return false;
}

Status ZipWriter::CreateDirectory(const fs::path& path)
{
	return false;
}
