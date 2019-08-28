#pragma once
#include "ArchiveInterface.h"
#include "FileListBinarySearch.h"
// #include "FileListHashMap.h"

namespace fsal
{
	namespace ZIP_SIGNATURES
	{
		enum
		{
			CENTRAL_DIRECTORY_FILE_HEADER = 0x02014b50,
			END_OF_CENTRAL_DIRECTORY_SIGN = 0x06054b50,
			LOCAL_HEADER = 0x04034b50,
		};
	}

#pragma pack(push,1)

	struct DataDescriptor
	{
		int32_t CRC32;
		int32_t compressedSize;
		int32_t uncompressedSize;
	};

	struct CentralDirectoryHeader
	{
		int32_t centralFileHeaderSignature;
		int16_t versionMadeBy;
		int16_t	versionNeededToExtract;
		int16_t	generalPurposBbitFlag;
		int16_t	compressionMethod;
		int16_t	lastModFileTime;
		int16_t	lastModFileDate;
		DataDescriptor dataDescriptor;
		int16_t	fileNameLength;
		int16_t	extraFieldLength;
		int16_t	fileCommentLength;
		int16_t	diskNumberStart;
		int16_t	internalFileAttributes;
		int32_t	externalFileAttributes;
		int32_t	relativeOffsetOfLocalHeader;
	};

	struct EndOfCentralDirectoryRecord
	{
		int32_t endOfCentralDirSignature;
		int16_t	numberOfThisDisk;
		int16_t	numberOfTheDiskWithTheStartOfTheCentralDirectory;
		int16_t	totalNumberOfEntriesInTheCentralDirectoryOnThisDisk;
		int16_t	totalNumberOfEntriesInTheCentralDirectory;
		int32_t	sizeOfTheCentralDirectory;
		int32_t	offsetOfStartOfCentralDirectory;
		int16_t ZIPFileCommentLength;
	};

	struct LocalFileHeader
	{
		int32_t localFileHeaderSignature;
		int16_t	versionNeededToExtract;
		int16_t	generalPurposeBitFlag;
		int16_t	compressionMethod;
		int16_t	lastModFileTime;
		int16_t	lastModFileDate;
		DataDescriptor dataDescriptor;
		int16_t	fileNameLength;
		int16_t	extraFieldLength;
	};

	struct ZipEntryData
	{
		size_t sizeCompressed = 0;
		size_t sizeUncompressed = 0;
		ptrdiff_t offset = -1;
		int16_t	compressionMethod = 0;
		int16_t	generalPurposeBitFlag = 0;
	};

#pragma pack(pop)

	class ZipReader: ArchiveReaderInterface
	{
	public:
		virtual Status OpenArchive(File file) override;

		virtual File OpenFile(const fs::path& filepath) override;

		virtual bool Exists(const fs::path& filepath, PathType type = kFile | kDirectory) override;

		virtual std::vector<std::string> ListDirectory(const fs::path& path) override;
	private:
		FileList<ZipEntryData> filelist;
		File file;
	};

	class ZipWriter : ArchiveWriterInterface
	{
	public:
		virtual Status CreateArchive(File file) override;

		virtual Status AddFile(const fs::path& path, File file, int compression) override;

		virtual Status CreateDirectory(const fs::path& path) override;

	private:
		FileList<ZipEntryData> filelist;
		File file;
	};
}
