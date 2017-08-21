#include <array>
#include <string>

#include "utils/Utils.h"

#include "FireRenderIES_Profiles.h"

FIRERENDER_NAMESPACE_BEGIN

namespace
{
	bool DoFindSession(const TCHAR* searchRequest,
		std::function<bool(WIN32_FIND_DATA&)> f)
	{
		WIN32_FIND_DATA ffd;
		auto hFind = FindFirstFile(searchRequest, &ffd);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		do
		{
			if (f(ffd))
			{
				return true;
			}
		} while (FindNextFile(hFind, &ffd) != 0);

		auto closeRes = FindClose(hFind);
		FASSERT(closeRes);

		return false;
	}
}

std::wstring FireRenderIES_Profiles::GetIESProfilesDirectory()
{
	return GetDataStoreFolder() + L"IES Profiles\\";
}

bool FireRenderIES_Profiles::GetIESFileName(std::wstring& filename,
	size_t* nFileOffset, size_t* nExtOffset)
{
	constexpr size_t fileBufferSize = MAX_PATH;
	std::array<TCHAR, fileBufferSize> fileBuffer;
	std::fill(fileBuffer.begin(), fileBuffer.end(), 0);

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetCOREInterface()->GetMAXHWnd();
	ofn.lpstrFilter = _T("IES Files(*.IES)\0*.IES\0All files(*.*)\0*.*\0\0");
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = &fileBuffer[0];
	ofn.nMaxFile = fileBufferSize;
	ofn.lpstrTitle = _T("Select IES file");
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	ofn.lpstrDefExt = _T("IES");

	if (GetOpenFileName(&ofn) == FALSE)
	{
		auto errorCode = CommDlgExtendedError();
		FASSERT(errorCode == 0);
		return false;
	}

	filename = ofn.lpstrFile;

	if (nFileOffset != nullptr) *nFileOffset = ofn.nFileOffset;
	if (nExtOffset != nullptr) *nExtOffset = ofn.nFileExtension;

	return true;
}

bool FireRenderIES_Profiles::CopyIES_File(const TCHAR* from, size_t nameOffset)
{
	auto profilesDir = GetIESProfilesDirectory();
	auto newName = profilesDir + (from + nameOffset);
	auto rawNewName = newName.c_str();

	bool replace = false;

	// Ask user to replace an existing profile
	if (FileExists(rawNewName))
	{
		replace = true;

		auto result = MessageBox(
			GetCOREInterface()->GetMAXHWnd(),
			_T("You are going to replace an existing profile. Continue?"),
			_T("Warning"),
			MB_ICONQUESTION | MB_YESNO);

		FASSERT(result != 0);

		if (result != IDYES)
		{
			return false;
		}
	}

	// Make sure that profiles directory exists
	if (!FolderExists(profilesDir.c_str()))
	{
		auto res = CreateDirectory(profilesDir.c_str(), NULL);
		FASSERT(res && "Failed to create profiles directory");
	}

	// Copy profile file
	auto res = CopyFile(from, rawNewName, FALSE);
	FASSERT(res);

	return replace ? false : res;
}

void FireRenderIES_Profiles::ForEachProfile(std::function<void(const TCHAR* name)> f)
{
	auto profilesFolder = GetIESProfilesDirectory() + L'*';

	DoFindSession(profilesFolder.c_str(), [&](WIN32_FIND_DATA& ffd)
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			// should it be recursive?
		}
		else
		{
			f(ffd.cFileName);
		}

		return false;
	});
}

std::wstring FireRenderIES_Profiles::ProfileNameToPath(const TCHAR* profileName)
{
	return GetIESProfilesDirectory() + profileName;
}

FIRERENDER_NAMESPACE_END
