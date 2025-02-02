#include "AssetRegistry/FileId.h"
#include <corecrt_io.h>
#include <combaseapi.h>

using namespace Sailor;
using namespace nlohmann;

const FileId FileId::Invalid = FileId();

YAML::Node FileId::Serialize() const
{
	YAML::Node outData;
	outData = m_fileId;
	return outData;
}

void FileId::Deserialize(const YAML::Node& inData)
{
	m_fileId = inData.as<std::string>();
}

void FileId::Serialize(nlohmann::json& outData) const
{
	outData = json{ {"fileId", m_fileId} };
}

void FileId::Deserialize(const nlohmann::json& inData)
{
	inData.at("fileId").get_to<std::string>(m_fileId);
}

const std::string& FileId::ToString() const
{
	return m_fileId;
}

bool FileId::operator==(const FileId& rhs) const
{
	return m_fileId == rhs.m_fileId;
}

FileId FileId::CreateNewFileId()
{
	FileId newuid;

	::GUID win32;
	CoCreateGuid(&win32);

	char buffer[128];
	sprintf_s(buffer, "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
		win32.Data1, win32.Data2, win32.Data3,
		win32.Data4[0], win32.Data4[1], win32.Data4[2], win32.Data4[3],
		win32.Data4[4], win32.Data4[5], win32.Data4[6], win32.Data4[7]);

	newuid.m_fileId = std::string(buffer);
	return newuid;
}