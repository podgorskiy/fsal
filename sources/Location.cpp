#include "fsal.h"

#include <stdio.h>  /* defines FILENAME_MAX */
#ifdef _WIN32
#include <direct.h>
#include <Shlobj.h>
#define GetCurrentDir _getcwd
#else
#include <unistd.h>
#define GetCurrentDir getcwd
#endif

#undef GetCurrentDirectory

using namespace fsal;

path Location::GetCurrentDirectory()
{
	static char cCurrentPath[FILENAME_MAX];
	GetCurrentDir(cCurrentPath, sizeof(cCurrentPath));
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	return cCurrentPath;
}

Location::Location(const char* filepath, Options location, PathType type, LinkType link) : m_filepath(filepath), m_relartiveTo(location), m_type(type), m_link(link)
{
}

Location::Location(const std::string& filepath, Options location, PathType type, LinkType link) : m_filepath(filepath), m_relartiveTo(location), m_type(type), m_link(link)
{
}

Location::Location(const path& filepath, Options location, PathType type, LinkType link) : m_filepath(filepath), m_relartiveTo(location), m_type(type), m_link(link)
{
}

Location::Location(Options location) : m_relartiveTo(location)
{
}

path Location::GetFullPath() const
{
	Options relartiveTo = m_relartiveTo;

	switch (m_relartiveTo)
	{
	case kAbsolute:
	{
		return m_filepath;
	}
	case kCurrentDirectory:
	{
		if (m_filepath.is_absolute())
		{
			return m_filepath;
		}
		return GetCurrentDirectory() / m_filepath;
	}
	case kTemp:
#ifdef WIN32
		char buff[1024];
		GetTempPath(1024, buff);
		return path(buff) / m_filepath;
#endif
	}

#ifdef WIN32
	KNOWNFOLDERID id;

#define WINCASE(X) case kWin_##X: id = FOLDERID_##X; break;
	switch (relartiveTo)
	{
		WINCASE(RoamingAppData);
		WINCASE(LocalAppData);
		WINCASE(LocalAppDataLow);
		WINCASE(ProgramData);
		WINCASE(ProgramFiles);
		WINCASE(ProgramFilesX86);
		WINCASE(System);
		WINCASE(Windows);
		WINCASE(RecycleBinFolder);
		WINCASE(AccountPictures);
		WINCASE(ApplicationShortcuts);
		WINCASE(Documents);
		WINCASE(Downloads);
		WINCASE(UserProfiles);
		WINCASE(Fonts);
		WINCASE(InternetCache);
		WINCASE(Pictures);
		WINCASE(AdminTools);
		WINCASE(CDBurning);
		WINCASE(CommonAdminTools);
		WINCASE(CommonPrograms);
		WINCASE(CommonStartMenu);
		WINCASE(CommonStartup);
		WINCASE(CommonTemplates);
		WINCASE(Contacts);
		WINCASE(Cookies);
		WINCASE(Desktop);
		WINCASE(DeviceMetadataStore);
		WINCASE(DocumentsLibrary);
		WINCASE(History);
		WINCASE(Libraries);
		WINCASE(Links);
		WINCASE(LocalizedResourcesDir);
		WINCASE(Music);
		WINCASE(Videos);
		WINCASE(MusicLibrary);
		WINCASE(NetHood);
		WINCASE(PhotoAlbums);
		WINCASE(Profile);
		WINCASE(ProgramFilesCommon);
		WINCASE(ProgramFilesCommonX86);
		WINCASE(Public);
		WINCASE(PublicDesktop);
		WINCASE(PublicDocuments);
		WINCASE(PublicDownloads);
		WINCASE(PublicLibraries);
		WINCASE(PublicMusic);
		WINCASE(PublicPictures);
		WINCASE(PublicVideos);

	default:
		return m_filepath;
	}
#undef WINCASE

	LPWSTR wszPath = NULL;
	HRESULT result = SHGetKnownFolderPath(id, 0, NULL, &wszPath);
	if (result == S_OK)
	{
		path relativepath(wszPath);

		CoTaskMemFree(wszPath);
		return relativepath / m_filepath;
	}
#endif

	return m_filepath;
}
