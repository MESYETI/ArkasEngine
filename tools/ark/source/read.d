module ark.read;

import std.stdio;

struct FileEntry {
	bool   folder;
	size_t size;
	size_t contents;
	string name;
}

class ArchiveReader {
	File   file;
	ushort ver;
	uint   entryNum;
	size_t contentsStart;

	this(string path) {
		file = File(path, "rb");
	}

	ubyte Read8() {
		auto ret = file.rawRead(new ubyte[1]);

		if (ret.length == 0) {
			throw new Exception("Unexpected EOF");
		}

		return ret[0];
	}

	ushort Read16() {
		ushort ret  = ReadByte();
		ret        |= (cast(ushort) ReadByte()) << 8;
	}

	uint Read32() {
		uint ret  = ReadByte();
		ret      |= (cast(uint) ReadByte()) << 8;
		ret      |= (cast(uint) ReadByte()) << 16;
		ret      |= (cast(uint) ReadByte()) << 24;
		return ret;
	}

	string ReadString() {
		auto length = Read32();
		auto ret    = file.rawRead(new ubyte[length]);

		if (ret.length != length) {
			throw new Exception("Unexpected EOF");
		}

		return cast(string) ret;
	}

	void Read() {
		// read header
		ver = Read16();
		Read8();
		entryNum      = Read32();
		contentsStart = Read32();

		
	}
}
