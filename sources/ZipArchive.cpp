#include "bfio.h"
#include "fsal_common.h"
#include "ZipArchive.h"
#include "FileStream.h"
#include "MemRefFile.h"
#include <cassert>
#include <zlib.h>


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

	{
		bfio::SizeCalculator s;
		s << header;
		assert(s.GetSize() == sizeof(header));
	}
	{
		bfio::SizeCalculator s;
		s << fileHeader;
		assert(s.GetSize() == sizeof(fileHeader));
	}

	for (int i = 0; file.Tell() - ecdr.offsetOfStartOfCentralDirectory < ecdr.sizeOfTheCentralDirectory;++i)
	{
		// stream >> header;
		file.Read(header);

		assert(header.centralFileHeaderSignature == ZIP_SIGNATURES::CENTRAL_DIRECTORY_FILE_HEADER);

		size_t currPos = file.Tell();

		file.Seek(header.relativeOffsetOfLocalHeader, File::Beginning);

		// stream >> fileHeader;
		file.Read(fileHeader);

		assert(fileHeader.localFileHeaderSignature == ZIP_SIGNATURES::LOCAL_HEADER);

		filename.resize(fileHeader.fileNameLength);

		file.Read((uint8_t*)&filename[0], fileHeader.fileNameLength);

		ZipEntryData entry;
		entry.compressionMethod = header.compressionMethod;
		entry.generalPurposeBitFlag = header.generalPurposBbitFlag;
		entry.sizeUncompressed = header.dataDescriptor.uncompressedSize;
		entry.sizeCompressed = header.dataDescriptor.compressedSize;
		entry.offset = file.Tell() + fileHeader.extraFieldLength;

		filelist.Add(entry, filename);

		file.Seek(currPos + header.fileNameLength + header.extraFieldLength + header.fileCommentLength, File::Beginning);
	}
	FileEntry<ZipEntryData> key("");
	filelist.GetIndex(key);

	return true;
}


File ZipReader::OpenFile(const fs::path& filepath)
{
	ZipEntryData entry = filelist.FindEntry(filepath);

	if (entry.offset != -1)
	{
		switch (entry.compressionMethod)
		{
			case ZIP_COMPRESSION::NONE:
			{
				auto* memfile = new MemRefFile();
				memfile->Resize(entry.sizeUncompressed);
				auto* data = memfile->GetDataPointer();
				fileMutex.lock();
				file.Seek(entry.offset, File::Beginning);
				file.Read((uint8_t*)data, entry.sizeUncompressed);
				fileMutex.unlock();
				return memfile;
			}

			case ZIP_COMPRESSION::DEFLATE:
			{
				auto* memfile = new MemRefFile();
				memfile->Resize(entry.sizeUncompressed);
				auto* uncompressedBuffer = memfile->GetDataPointer();
				char* compressedBuffer = new char[entry.sizeCompressed];

				fileMutex.lock();
				file.Seek(entry.offset, File::Beginning);
				file.Read((uint8_t*)compressedBuffer, entry.sizeCompressed);
				fileMutex.unlock();

				z_stream stream = {0};
				int32_t err;
				stream.next_in = (Bytef*)compressedBuffer;
				stream.avail_in = (uInt)entry.sizeCompressed;
				stream.next_out = (Bytef*)uncompressedBuffer;
				stream.avail_out = entry.sizeUncompressed;
				stream.zalloc = (alloc_func)nullptr;
				stream.zfree = (free_func)nullptr;

				err = inflateInit2(&stream, -MAX_WBITS);
				if (err == Z_OK)
				{
					err = inflate(&stream, Z_FINISH);
					inflateEnd(&stream);
					if (err == Z_STREAM_END)
					{
						err = Z_OK;
					}
					inflateEnd(&stream);
				}

				if (err != Z_OK)
				{
					delete[] compressedBuffer;
					delete memfile;
					return File();
				}
				else
				{
					delete[] compressedBuffer;
					return memfile;
				}
			}
//
//			case ZIP_ENUMS::LZ4:
//			{
//				auto* memfile = new MemRefFile();
//				memfile->Resize(entry.sizeUncompressed);
//				auto* uncompressedBuffer = memfile->GetDataPointer();
//
//				char* compressedBuffer = new char[entry.sizeCompressed];
//
//				fileMutex.lock();
//				file.Seek(entry.offset, File::Beginning);
//				file.Read((uint8_t*)compressedBuffer, entry.sizeUncompressed);
//				fileMutex.unlock();
//
//				int b = LZ4_decompress_fast(compressedBuffer, uncompressedBuffer, entry.sizeUncompressed);
//
//				if (b <= 0)
//				{
//					delete[] compressedBuffer;
//					delete memfile;
//					return File();
//				}
//				else
//				{
//					delete[] compressedBuffer;
//					return memfile;
//				}
//			}

			default:
			{
				return File();
			}
		}
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

	return filelist.Exists(key);
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
