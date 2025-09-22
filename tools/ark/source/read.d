module ark.read;

import std.stdio;
import std.string;

struct FileEntry {
	bool   folder;
	size_t size;
	string name;
	size_t contentsOffset;

	FileEntry[] folderContents;
}

class ArchiveReader {
	File      file;
	ushort    ver;
	uint      entryNum;
	size_t    contentsStart;
	ubyte[]   strings;
	FileEntry root;

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
		ushort ret  = Read8();
		ret        |= (cast(ushort) Read8()) << 8;
		return ret;
	}

	uint Read32() {
		uint ret  = Read8();
		ret      |= (cast(uint) Read8()) << 8;
		ret      |= (cast(uint) Read8()) << 16;
		ret      |= (cast(uint) Read8()) << 24;
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

	ubyte[] ReadFile(FileEntry entry) {
		file.seek(entry.contentsOffset, SEEK_SET);

		auto ret = file.rawRead(new ubyte[entry.size]);

		if (ret.length != entry.size) {
			stderr.writefln("Error reading '%s'", entry.name);
			stderr.writefln("File is meant to be %d bytes", entry.size);
			stderr.writefln("Could only read %d bytes", ret.length);
			stderr.writefln("File is located at offset %.8x", entry.contentsOffset);
			assert(0);
		}
		return ret;
	}

	FileEntry ReadEntry() {
		FileEntry entry;

		entry.folder = Read8() != 0;
		entry.size   = Read32();

		// DO NOT forget to check this in the C version
		uint offset = Read32();
		writefln("Entry '%s' has path offset %d", entry.name, offset);
		entry.name = cast(string) (cast(char*) &strings[offset]).fromStringz();

		entry.contentsOffset = file.tell;

		if (entry.folder) {
			size_t length = Read32();

			foreach (i ; 0 .. length) {
				entry.folderContents ~= ReadEntry();
			}
		}
		else {
			file.seek(entry.size, SEEK_CUR);
		}

		return entry;
	}

	void Read() {
		// read header
		ver = Read16();
		writefln("Arkas version is %d", ver);
		Read8(); // unused

		auto stringLen = Read32();
		writefln("String table is %d bytes", stringLen);
		Read32(); // random number

		// read strings
		strings  = file.rawRead(new ubyte[stringLen]);
		if (strings.length != stringLen) {
			throw new Exception("Unexpected EOF");
		}
		strings ~= 0;

		// read file entries
		root = ReadEntry();
	}
}
