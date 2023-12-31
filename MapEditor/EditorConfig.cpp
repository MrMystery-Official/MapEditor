#include "EditorConfig.h"

void EditorConfig::Save() {
	BinaryVectorWriter Writer;

	Writer.WriteBytes("EDTC");
	Writer.WriteByte(0x01); //Version
	Writer.WriteInteger(Config::RomFSPath.length(), sizeof(uint32_t));
	Writer.WriteInteger(Config::BfresPath.length(), sizeof(uint32_t));
	Writer.WriteInteger(Config::ExportPath.length(), sizeof(uint32_t));

	Writer.WriteBytes(Config::RomFSPath.c_str());
	Writer.WriteBytes(Config::BfresPath.c_str());
	Writer.WriteBytes(Config::ExportPath.c_str());

	std::ofstream File(Config::GetWorkingDirFile("Editor.conf"), std::ios::binary);
	std::copy(Writer.GetData().cbegin(), Writer.GetData().cend(),
		std::ostream_iterator<unsigned char>(File));
	File.close();
}

void EditorConfig::Load() {
	std::ifstream File(Config::GetWorkingDirFile("Editor.conf"), std::ios::binary);
	File.seekg(0, std::ios_base::end);
	std::streampos FileSize = File.tellg();
	std::vector<unsigned char> Bytes(FileSize);
	File.seekg(0, std::ios_base::beg);
	File.read(reinterpret_cast<char*>(Bytes.data()), FileSize);
	File.close();

	BinaryVectorReader Reader(Bytes);

	char magic[5];
	Reader.Read(magic, 4);
	magic[4] = '\0';
	if (strcmp(magic, "EDTC") != 0) {
		std::cout << "ERROR: Expected EDTC, got " << magic << std::endl;
		return;
	}

	uint8_t Version = Reader.ReadUInt8();
	if (Version != 0x01) {
		std::cout << "ERROR: Expected Version 1, but got " << (int)Version << "!" << std::endl;
		return;
	}

	uint32_t RomFSLength = Reader.ReadUInt32();
	uint32_t BFRESLength = Reader.ReadUInt32();
	uint32_t ExportPathLength = Reader.ReadUInt32();

	Config::RomFSPath = "";
	Config::BfresPath = "";
	Config::ExportPath = "";

	for (int i = 0; i < RomFSLength; i++) {
		Config::RomFSPath += Reader.ReadInt8();
	}
	for (int i = 0; i < BFRESLength; i++) {
		Config::BfresPath += Reader.ReadInt8();
	}
	for (int i = 0; i < ExportPathLength; i++) {
		Config::ExportPath += Reader.ReadInt8();
	}
}