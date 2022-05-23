#pragma once

class CVersionUpdater
{
public:
	class CResourcePacker
	{
	public:
		CResourcePacker() : Resource(NULL), Size(0) {}
		~CResourcePacker() { Reset(); }

	public:
		void Reset();

		void AppendWord(WORD value_);
		void AppendWords(const WORD* value_, DWORD count_);
		void AppendWChar(WCHAR value_);
		void AppendWString(const WCHAR* value_);
		void AppendAlignment();
		void Append(const void* value_, DWORD size_);

		inline BYTE* GetResource() const { return this->Resource; }
		inline DWORD GetSize() const { return this->Size; }

	private:
		BYTE* Resource;
		DWORD Size;
	};
	class CResourceParser
	{
	public:
		CResourceParser(const BYTE* data_) : Data(data_), ParsedSize(0) {}

	public:
		WORD Word();
		WCHAR WChar();
		void Alignment();
		const BYTE* Move(long size_);
		long GetParsedSize() const { return this->ParsedSize; }

	private:
		const BYTE* Data;
		long ParsedSize;
	};

	struct structLangageAndCodePage
	{
		WORD Language;
		WORD CodePage;
	};

	class CStringTable : public structLangageAndCodePage
	{
	public:
		CStringTable();
		~CStringTable();

	public:
		inline const CString* GetComments() const { return this->Comments; }
		inline const CString* GetCompanyName() const { return this->CompanyName; }
		inline const CString* GetFileDescription() const { return this->FileDescription; }
		inline const CString* GetFileVersion() const { return this->FileVersion; }
		inline const CString* GetInternalName() const { return this->InternalName; }
		inline const CString* GetLegalCopyright() const { return this->LegalCopyright; }
		inline const CString* GetLegalTrademarks() const { return this->LegalTrademarks; }
		inline const CString* GetOriginalFilename() const { return this->OriginalFilename; }
		inline const CString* GetPrivateBuild() const { return this->PrivateBuild; }
		inline const CString* GetProductName() const { return this->ProductName; }
		inline const CString* GetProductVersion() const { return this->ProductVersion; }
		inline const CString* GetSpecialBuild() const { return this->SpecialBuild; }

		inline void SetComments(LPCTSTR value_) { this->Set(Comments, value_); }
		inline void SetCompanyName(LPCTSTR value_) { this->Set(CompanyName, value_); }
		inline void SetFileDescription(LPCTSTR value_) { this->Set(FileDescription, value_); }
		inline void SetFileVersion(LPCTSTR value_) { this->Set(FileVersion, value_); }
		inline void SetInternalName(LPCTSTR value_) { this->Set(InternalName, value_); }
		inline void SetLegalCopyright(LPCTSTR value_) { this->Set(LegalCopyright, value_); }
		inline void SetLegalTrademarks(LPCTSTR value_) { this->Set(LegalTrademarks, value_); }
		inline void SetOriginalFilename(LPCTSTR value_) { this->Set(OriginalFilename, value_); }
		inline void SetPrivateBuild(LPCTSTR value_) { this->Set(PrivateBuild, value_); }
		inline void SetProductName(LPCTSTR value_) { this->Set(ProductName, value_); }
		inline void SetProductVersion(LPCTSTR value_) { this->Set(ProductVersion, value_); }
		inline void SetSpecialBuild(LPCTSTR value_) { this->Set(SpecialBuild, value_); }

		void Set(CString*& which_, LPCTSTR value_);

		void Reset();
		void Pack(CResourcePacker& packer_) const;
		void Parse(const BYTE* data_);

	private:
		CString* Comments;
		CString* CompanyName;
		CString* FileDescription;
		CString* FileVersion;
		CString* InternalName;
		CString* LegalCopyright;
		CString* LegalTrademarks;
		CString* OriginalFilename;
		CString* PrivateBuild;
		CString* ProductName;
		CString* ProductVersion;
		CString* SpecialBuild;
	};
	class CStringFileInfo
	{
	public:
		~CStringFileInfo();

	public:
		CArray<CStringTable*, CStringTable*> Children;

	public:
		void Reset();
		void Pack(CResourcePacker& packer_) const;
		void Parse(const BYTE* data_);
	};

	class CVar
	{
	public:
		CArray<structLangageAndCodePage, structLangageAndCodePage&> Value;

	public:
		void Pack(CResourcePacker& packer_) const;
		void Parse(const BYTE* data_);
	};
	class CVarFileInfo
	{
	public:
		~CVarFileInfo();

	public:
		CArray<CVar*, CVar*> Children;

	public:
		void Reset();
		void Pack(CResourcePacker& packer_) const;
		void Parse(const BYTE* data_);
	};

	class CVersionInfo
	{
	public:
		CVersionInfo();
		~CVersionInfo();

	public:
		VS_FIXEDFILEINFO FixedFileInfo;
		CStringFileInfo* StringFileInfo;
		CVarFileInfo* VarFileInfo;

	public:
		void Reset();
		void Pack(CResourcePacker& packer_) const;
		void Parse(const BYTE* data_);
	};

public:
	CVersionUpdater();
	~CVersionUpdater();

public:
	bool Open(const CString& exeOrDll);
	bool Update(WORD language_ = MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL)) const;

public:
	CVersionInfo* VersionInfo;

private:
	CString ExeOrDll;
};

