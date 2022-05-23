#include "pch.h"
#include "VersionUpdater.h"

#pragma comment(linker, "/defaultlib:version.lib")


///
static WCHAR* SZKEY_VS_VERSION_INFO = L"VS_VERSION_INFO";
static WCHAR* SZKEY_STRINGFILEINFO = L"StringFileInfo";
static WCHAR* SZKEY_VARFILEINFO = L"VarFileInfo";
static WCHAR* SZKEY_TRANSLATION = L"Translation";

enum enumWType
{
	WTYPE_TEXT = 1,
	WTYPE_BINARY = 0,
};

static LPCTSTR StringKeys[] =
{
	_T("Comments"),
	_T("CompanyName"),
	_T("FileDescription"),
	_T("FileVersion"),
	_T("InternalName"),
	_T("LegalCopyright"),
	_T("LegalTrademarks"),
	_T("OriginalFilename"),
	_T("PrivateBuild"),
	_T("ProductName"),
	_T("ProductVersion"),
	_T("SpecialBuild")
};
int IndexInStringKeys(WCHAR* key)
{
	int length = wcslen(key);
	for (int i = 0; i < 12; ++i)
	{
		//if (_wcsnicmp(StringKeys[i], key, length) == 0)
		if (_wcsnicmp((LPWSTR)StringKeys[i], key, length) == 0)
			return i;
	}
	return -1;
}
typedef CString* PCString;
typedef CString** PPCString;


DWORD WStringValueLength(const CString& value_)
{
	return value_.GetLength();
}
void SetLength(BYTE* resource_, WORD length_)
{
	*reinterpret_cast<WORD*>(resource_) = length_;
}
WORD GetLength(const BYTE* resource_)
{
	return *reinterpret_cast<const WORD*>(resource_);
}

///
void CVersionUpdater::CResourcePacker::Reset()
{
	if (this->Resource)
	{
		delete[]this->Resource;
		this->Resource = NULL;
		this->Size = 0;
	}
}

void CVersionUpdater::CResourcePacker::AppendWord(WORD value_) { this->Append(&value_, sizeof(WORD)); }
void CVersionUpdater::CResourcePacker::AppendWords(const WORD* value_, DWORD count_) { this->Append(value_, sizeof(WORD)*count_); }
void CVersionUpdater::CResourcePacker::AppendWChar(WCHAR value_) { this->Append(&value_, sizeof(WCHAR)); }
void CVersionUpdater::CResourcePacker::AppendWString(const WCHAR* value_) { this->Append(value_, sizeof(WCHAR)*(wcslen(value_) + 1)); }
void CVersionUpdater::CResourcePacker::AppendAlignment()
{
	if (this->Size % 4 == 0) return;
	static BYTE aligmentsValues[3] = { 0,0,0 };
	DWORD aligments = 4 - (this->Size % 4);
	this->Append(aligmentsValues, aligments);
}

void CVersionUpdater::CResourcePacker::Append(const void* value_, DWORD size_)
{
	DWORD newSize = this->Size + size_;
	BYTE* newResource = new BYTE[newSize];
	LPTSTR strTmp = (LPTSTR)value_;
	LPTSTR* pStringBuffer = new LPTSTR[newSize];
	memcpy(pStringBuffer, this->Resource, this->Size);

	if (this->Resource)
	{
		memcpy(newResource, this->Resource, this->Size);
	}

	memcpy(newResource + this->Size, value_, size_);
	delete[]this->Resource;
	this->Resource = newResource;
	this->Size = newSize;
}

///
WORD CVersionUpdater::CResourceParser::Word()
{
	return *reinterpret_cast<const WORD*>(this->Move(sizeof(WORD)));
}
WCHAR CVersionUpdater::CResourceParser::WChar()
{
	return *reinterpret_cast<const WORD*>(this->Move(sizeof(WCHAR)));
}
void CVersionUpdater::CResourceParser::Alignment()
{
	if (this->ParsedSize % 4 == 0) return;
	DWORD alignments = 4 - (this->ParsedSize % 4);
	this->ParsedSize += alignments;
}
const BYTE* CVersionUpdater::CResourceParser::Move(long size_)
{
	const BYTE* v = (this->Data + this->ParsedSize);
	this->ParsedSize += size_;
	return v;
}

///
CVersionUpdater::CStringTable::CStringTable()
	: Comments(NULL)
	, CompanyName(NULL)
	, FileDescription(NULL)
	, FileVersion(NULL)
	, InternalName(NULL)
	, LegalCopyright(NULL)
	, LegalTrademarks(NULL)
	, OriginalFilename(NULL)
	, PrivateBuild(NULL)
	, ProductName(NULL)
	, ProductVersion(NULL)
	, SpecialBuild(NULL)
{
	Language = 0;
	CodePage = 0;
}
CVersionUpdater::CStringTable::~CStringTable() { Reset(); }

void CVersionUpdater::CStringTable::Set(CString*& which_, LPCTSTR value_)
{
	if (which_) delete which_;
	which_ = value_ ? (new CString(value_)) : NULL;
}

void CVersionUpdater::CStringTable::Reset()
{
	if (this->Comments) { delete this->Comments; this->Comments = NULL; }
	if (this->CompanyName) { delete this->CompanyName; this->CompanyName = NULL; }
	if (this->FileDescription) { delete this->FileDescription; this->FileDescription = NULL; }
	if (this->FileVersion) { delete this->FileVersion; this->FileVersion = NULL; }
	if (this->InternalName) { delete this->InternalName; this->InternalName = NULL; }
	if (this->LegalCopyright) { delete this->LegalCopyright; this->LegalCopyright = NULL; }
	if (this->LegalTrademarks) { delete this->LegalTrademarks; this->LegalTrademarks = NULL; }
	if (this->OriginalFilename) { delete this->OriginalFilename; this->OriginalFilename = NULL; }
	if (this->PrivateBuild) { delete this->PrivateBuild; this->PrivateBuild = NULL; }
	if (this->ProductName) { delete this->ProductName; this->ProductName = NULL; }
	if (this->ProductVersion) { delete this->ProductVersion; this->ProductVersion = NULL; }
	if (this->SpecialBuild) { delete this->SpecialBuild; this->SpecialBuild = NULL; }
}
void CStringTable_PackChild(CVersionUpdater::CResourcePacker& packer_, const CString& which_, const CString& value_)
{
	packer_.Reset();
	packer_.AppendWord(0);
	packer_.AppendWord(WStringValueLength(value_));
	packer_.AppendWord(WTYPE_TEXT);
	//packer_.AppendWString((LPCTSTR)which_);
	packer_.AppendWString(which_.AllocSysString());
	packer_.AppendAlignment();
	//packer_.AppendWString(value_);
	packer_.AppendWString(value_.AllocSysString());
	packer_.AppendAlignment();
	SetLength(packer_.GetResource(), packer_.GetSize());
}
void CStringTable_ParseChild(const BYTE* data_, PPCString* values)
{
	CVersionUpdater::CResourceParser parser(data_);
	parser.Word();
	WORD valueLength = parser.Word();
	enumWType wType = (enumWType)parser.Word();

	//ASSERT(wType == WTYPE_TEXT);

	WCHAR* key = (WCHAR*)parser.Move(0);
	parser.Move(sizeof(WCHAR)*(wcslen(key) + 1));
	parser.Alignment();

#if 0
	* values[IndexInStringKeys(key)] = new CString((WCHAR*)parser.Move(0));
#endif
}
void CVersionUpdater::CStringTable::Pack(CResourcePacker& packer_) const
{
	packer_.Reset();
	packer_.AppendWord(0);
	packer_.AppendWord(0);
	packer_.AppendWord(WTYPE_TEXT);
	CString translate;
	translate.Format(_T("%04x%04x"), this->Language, this->CodePage);
	packer_.AppendWString(translate.AllocSysString());
	packer_.AppendAlignment();

	PCString values[] =
	{
		this->Comments,
		this->CompanyName,
		this->FileDescription,
		this->FileVersion,
		this->InternalName,
		this->LegalCopyright,
		this->LegalTrademarks,
		this->OriginalFilename,
		this->PrivateBuild,
		this->ProductName,
		this->ProductVersion,
		this->SpecialBuild
	};
	for (int i = 0; i < 12; ++i)
	{
		if (!values[i]) continue;
		CResourcePacker childPacker;
		CStringTable_PackChild(childPacker, StringKeys[i], *values[i]);
		packer_.Append(childPacker.GetResource(), childPacker.GetSize());
		packer_.AppendAlignment();
	}
	packer_.AppendAlignment();
	SetLength(packer_.GetResource(), packer_.GetSize());
}
void CVersionUpdater::CStringTable::Parse(const BYTE* data_)
{
	Reset();

	CResourceParser parser(data_);
	parser.Word();
	parser.Word();
	enumWType wType = (enumWType)parser.Word(); ASSERT(wType == WTYPE_TEXT);
	int iLanguage = 0, iCodePage = 0;
	//swscanf((WCHAR*)parser.Move(8 * sizeof(WCHAR)), L"%04x%04x", &iLanguage, &iCodePage);
	swscanf_s((WCHAR*)parser.Move(8 * sizeof(WCHAR)), L"%04x%04x", &iLanguage, &iCodePage);
	this->Language = iLanguage;
	this->CodePage = iCodePage;
	parser.Alignment();

	PPCString values[] =
	{
		&this->Comments,
		&this->CompanyName,
		&this->FileDescription,
		&this->FileVersion,
		&this->InternalName,
		&this->LegalCopyright,
		&this->LegalTrademarks,
		&this->OriginalFilename,
		&this->PrivateBuild,
		&this->ProductName,
		&this->ProductVersion,
		&this->SpecialBuild
	};

	long structLength = GetLength(data_);
	long parsedLength = parser.GetParsedSize();
	while (parsedLength < structLength)
	{
		CStringTable_ParseChild(data_ + parsedLength, values);
		parsedLength += GetLength(data_ + parsedLength);
		if (parsedLength % 4 != 0) parsedLength += 4 - (parsedLength % 4);
	}
}
///
CVersionUpdater::CStringFileInfo::~CStringFileInfo() { Reset(); }

void CVersionUpdater::CStringFileInfo::Reset()
{
	for (int i = 0; i <= this->Children.GetUpperBound(); ++i)
	{
		delete this->Children.GetAt(i);
	}
	this->Children.RemoveAll();
}
void CVersionUpdater::CStringFileInfo::Pack(CResourcePacker& packer_) const
{
	packer_.Reset();
	packer_.AppendWord(0);
	packer_.AppendWord(0);
	packer_.AppendWord(WTYPE_TEXT);
	packer_.AppendWString(SZKEY_STRINGFILEINFO);
	packer_.AppendAlignment();

	for (int i = 0; i <= this->Children.GetUpperBound(); ++i)
	{
		CStringTable* st = this->Children.GetAt(i);
		if (!st) continue;
		CResourcePacker childPacker;
		st->Pack(childPacker);
		packer_.Append(childPacker.GetResource(), childPacker.GetSize());
		packer_.AppendAlignment();
	}
	packer_.AppendAlignment();
	SetLength(packer_.GetResource(), packer_.GetSize());
}
void CVersionUpdater::CStringFileInfo::Parse(const BYTE* data_)
{
	Reset();

	CResourceParser parser(data_);
	parser.Word();
	parser.Word();
	enumWType wType = (enumWType)parser.Word(); ASSERT(wType == WTYPE_TEXT);
	parser.Move(sizeof(WCHAR)*(wcslen(SZKEY_STRINGFILEINFO) + 1));
	parser.Alignment();

	long structLength = GetLength(data_);
	long parsedLength = parser.GetParsedSize();
	while (parsedLength < structLength)
	{
		CStringTable* st = new CStringTable;
		st->Parse(data_ + parsedLength);
		this->Children.Add(st);
		parsedLength += GetLength(data_ + parsedLength);
		if (parsedLength % 4 != 0) parsedLength += 4 - (parsedLength % 4);
	}
}
///
void CVersionUpdater::CVar::Pack(CResourcePacker& packer_) const
{
	packer_.Reset();
	packer_.AppendWord(0);
	packer_.AppendWord(sizeof(structLangageAndCodePage)*(this->Value.GetCount()));
	packer_.AppendWord(WTYPE_BINARY);
	packer_.AppendWString(SZKEY_TRANSLATION);
	packer_.AppendAlignment();

	for (int i = 0; i <= this->Value.GetUpperBound(); ++i)
	{
		const structLangageAndCodePage& v = this->Value.GetAt(i);
		packer_.Append(&v, sizeof(structLangageAndCodePage));
		packer_.AppendAlignment();
	}
	packer_.AppendAlignment();
	SetLength(packer_.GetResource(), packer_.GetSize());
}
void CVersionUpdater::CVar::Parse(const BYTE* data_)
{
	CResourceParser parser(data_);
	parser.Word();
	WORD valueLength = parser.Word();
	enumWType wType = (enumWType)parser.Word(); ASSERT(wType == WTYPE_BINARY);
	parser.Move(sizeof(WCHAR)*(wcslen(SZKEY_TRANSLATION) + 1));
	parser.Alignment();

	long structLength = GetLength(data_);
	long parsedLength = parser.GetParsedSize();
	while (parsedLength < structLength)
	{
		this->Value.Add(*(structLangageAndCodePage*)(data_ + parsedLength));
		//parsedLength += GetLength(data_ + parsedLength);
		parsedLength += sizeof(structLangageAndCodePage);
		if (parsedLength % 4 != 0) parsedLength += 4 - (parsedLength % 4);
	}
}
///
CVersionUpdater::CVarFileInfo::~CVarFileInfo() { Reset(); }

void CVersionUpdater::CVarFileInfo::Reset()
{
	for (int i = 0; i <= this->Children.GetUpperBound(); ++i)
	{
		delete this->Children.GetAt(i);
	}
	this->Children.RemoveAll();
}
void CVersionUpdater::CVarFileInfo::Pack(CResourcePacker& packer_) const
{
	packer_.Reset();
	packer_.AppendWord(0);
	packer_.AppendWord(0);
	packer_.AppendWord(WTYPE_TEXT);
	packer_.AppendWString(SZKEY_VARFILEINFO);
	packer_.AppendAlignment();

	for (int i = 0; i <= this->Children.GetUpperBound(); ++i)
	{
		CVar* v = this->Children.GetAt(i);
		if (!v) continue;
		CResourcePacker childPacker;
		v->Pack(childPacker);
		packer_.Append(childPacker.GetResource(), childPacker.GetSize());
		packer_.AppendAlignment();
	}
	packer_.AppendAlignment();
	SetLength(packer_.GetResource(), packer_.GetSize());
}
void CVersionUpdater::CVarFileInfo::Parse(const BYTE* data_)
{
	Reset();

	CResourceParser parser(data_);
	parser.Word();
	parser.Word();
	enumWType wType = (enumWType)parser.Word(); ASSERT(wType == WTYPE_TEXT);
	parser.Move(sizeof(WCHAR)*(wcslen(SZKEY_VARFILEINFO) + 1));
	parser.Alignment();

	long structLength = GetLength(data_);
	long parsedLength = parser.GetParsedSize();
	while (parsedLength < structLength)
	{
		CVar* v = new CVar;
		v->Parse(data_ + parsedLength);
		this->Children.Add(v);
		parsedLength += GetLength(data_ + parsedLength);
		if (parsedLength % 4 != 0) parsedLength += 4 - (parsedLength % 4);
	}
}
///
CVersionUpdater::CVersionInfo::CVersionInfo() :StringFileInfo(NULL), VarFileInfo(NULL)
{
	memset(&this->FixedFileInfo, 0, sizeof(VS_FIXEDFILEINFO));
}
CVersionUpdater::CVersionInfo::~CVersionInfo() { Reset(); }

void CVersionUpdater::CVersionInfo::Reset()
{
	if (this->StringFileInfo) { delete this->StringFileInfo; this->StringFileInfo = NULL; }
	if (this->VarFileInfo) { delete this->VarFileInfo; this->VarFileInfo = NULL; }
}
void CVersionUpdater::CVersionInfo::Pack(CResourcePacker& packer_) const
{
	packer_.Reset();
	packer_.AppendWord(0);
	packer_.AppendWord(sizeof(VS_FIXEDFILEINFO));
	packer_.AppendWord(WTYPE_BINARY);
	packer_.AppendWString(SZKEY_VS_VERSION_INFO);
	packer_.AppendAlignment();
	packer_.Append(&this->FixedFileInfo, sizeof(VS_FIXEDFILEINFO));
	packer_.AppendAlignment();

	if (this->StringFileInfo)
	{
		CResourcePacker childPacker;
		this->StringFileInfo->Pack(childPacker);
		packer_.Append(childPacker.GetResource(), childPacker.GetSize());
		packer_.AppendAlignment();
	}
	if (this->VarFileInfo)
	{
		CResourcePacker childPacker;
		this->VarFileInfo->Pack(childPacker);
		packer_.Append(childPacker.GetResource(), childPacker.GetSize());
		packer_.AppendAlignment();
	}

	packer_.AppendAlignment();
	SetLength(packer_.GetResource(), packer_.GetSize());
}
void CVersionUpdater::CVersionInfo::Parse(const BYTE* data_)
{
	Reset();

	CResourceParser parser(data_);
	parser.Word();
	parser.Word();
	enumWType wType = (enumWType)parser.Word(); ASSERT(wType == WTYPE_BINARY);
	parser.Move(sizeof(WCHAR)*(wcslen(SZKEY_VS_VERSION_INFO) + 1));
	parser.Alignment();
	this->FixedFileInfo = *(VS_FIXEDFILEINFO*)parser.Move(sizeof(VS_FIXEDFILEINFO));
	parser.Alignment();

	const BYTE* next = parser.Move(sizeof(WORD) * 3);
	LPCTSTR v = (LPCTSTR)parser.Move(0);

	//if (_wcsnicmp(v, SZKEY_STRINGFILEINFO, wcslen(SZKEY_STRINGFILEINFO)) == 0)
	if (_wcsnicmp((LPWSTR)v, SZKEY_STRINGFILEINFO, wcslen(SZKEY_STRINGFILEINFO)) == 0)
	{
		this->StringFileInfo = new CStringFileInfo;
		this->StringFileInfo->Parse(next);
		parser.Move(GetLength(next) - sizeof(WORD) * 3);
		//add w
		parser.Alignment();

		next = parser.Move(sizeof(WORD) * 3);
		v = (LPCTSTR)parser.Move(0);
	}
	//if (_wcsnicmp(v, SZKEY_VARFILEINFO, wcslen(SZKEY_VARFILEINFO)) == 0)
	if (_wcsnicmp((LPWSTR)v, SZKEY_VARFILEINFO, wcslen(SZKEY_VARFILEINFO)) == 0)
	{
		this->VarFileInfo = new CVarFileInfo;
		this->VarFileInfo->Parse(next);
	}
}

///
CVersionUpdater::CVersionUpdater() : VersionInfo(NULL) {}
CVersionUpdater::~CVersionUpdater()
{
	if (this->VersionInfo)
	{
		delete this->VersionInfo;
		this->VersionInfo = NULL;
	}
}

bool CVersionUpdater::Open(const CString& exeOrDll)
{
	DWORD handle = 0;
	DWORD blockBytesCount = GetFileVersionInfoSize(exeOrDll, &handle);
	if (blockBytesCount == 0) return false;

	BYTE* blockBytes = new BYTE[blockBytesCount];
	if (!GetFileVersionInfo(exeOrDll, NULL, blockBytesCount, blockBytes))
	{
		delete[]blockBytes;
		return false;
	}

	CVersionInfo* versionInfo = new CVersionInfo;
	versionInfo->Parse(blockBytes);
	delete[]blockBytes;

	if (this->VersionInfo) delete this->VersionInfo;
	this->VersionInfo = versionInfo;
	this->ExeOrDll = exeOrDll;

	return true;
}

int Div(const CString strLine, char split, CStringArray &strArray)
{
	strArray.RemoveAll();//自带清空属性
	CString temp = strLine;
	int tag = 0;
	while (1)
	{
		tag = temp.Find(split);
		if (tag >= 0)
		{
			strArray.Add(temp.Left(tag));
			temp = temp.Right(temp.GetLength() - tag - 1);
		}
		else { break; }
	}
	strArray.Add(temp);
	return strArray.GetSize();
}

bool CVersionUpdater::Update(WORD language_) const
{
	if (!this->VersionInfo) return false;

	CStringArray strArray;

	for (int i = 0; i <= this->VersionInfo->StringFileInfo->Children.GetUpperBound(); ++i)
	{
		CVersionUpdater::CStringTable* st = this->VersionInfo->StringFileInfo->Children.GetAt(i);
		if (0 == language_)
		{
			language_ = st->Language;
		}
		const CString* strFileVersion = st->GetFileVersion();
		CString cs(strFileVersion->AllocSysString());
		Div(*strFileVersion, ',', strArray);
	}

	CResourcePacker packer;
	this->VersionInfo->Pack(packer);

	// 修改版本信息 
	LPVOID lpFixedBuf = NULL;
	DWORD dwFixedLen = 0;
	if (FALSE != VerQueryValue(packer.GetResource(), _T("\\"), &lpFixedBuf, (PUINT)&dwFixedLen))
	{
		VS_FIXEDFILEINFO* pFixedInfo = (VS_FIXEDFILEINFO*)lpFixedBuf;
		pFixedInfo->dwFileVersionMS = MAKELONG(_ttoi(strArray[1]), _ttoi(strArray[0]));
		pFixedInfo->dwFileVersionLS = MAKELONG(_ttoi(strArray[3]), _ttoi(strArray[2]));
	}

	HANDLE resource = BeginUpdateResource(this->ExeOrDll, FALSE);
	if (!resource) return false;

	if (!UpdateResource(resource, RT_VERSION, MAKEINTRESOURCE(VS_VERSION_INFO), language_, packer.GetResource(), packer.GetSize())) return false;
	return EndUpdateResource(resource, FALSE) != FALSE;
}