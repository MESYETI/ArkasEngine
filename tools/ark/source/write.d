module ark.write;

import std.file;
import std.path;
import std.stdio;
import core.stdc.time;
import core.stdc.stdlib : srand, rand;

struct StrTableEntry {
	size_t offset;
	string value;
}

struct FileEntry {
	bool   folder;
	string fullPath;
	size_t pathOffset;

	FileEntry[] contents;
}

class ArchiveWriter {
	File file;

	StrTableEntry[] stringTable;

	this(string path) {
		file = File(path, "wb+");
	}

	void Write8(ubyte value) {
		file.rawWrite([value]);
	}

	void Write16(ushort value) {
		Write8(cast(ubyte) (value & 0xFF));
		Write8(cast(ubyte) ((value & 0xFF00) >> 8));
	}

	void Write32(uint value) {
		Write8(cast(ubyte) (value & 0xFF));
		Write8(cast(ubyte) ((value & 0xFF00) >> 8));
		Write8(cast(ubyte) ((value & 0xFF0000) >> 16));
		Write8(cast(ubyte) ((value & 0xFF000000) >> 24));
	}

	void WriteString(string str) {
		foreach (ref ch ; str) {
			Write8(cast(ubyte) ch);
		}

		Write8(0);
	}

	void WriteEntry(FileEntry entry) {
		if (entry.fullPath.baseName() == file.name.baseName()) return;

		Write8(entry.folder? 1 : 0);

		string path = StringTableValue(entry.pathOffset);

		if (entry.folder) {
			Write32(0);
		}
		else {
			Write32(cast(uint) getSize(entry.fullPath));
		}

		Write32(cast(uint) entry.pathOffset);

		if (entry.folder) {
			Write32(cast(uint) entry.contents.length);

			foreach (ref e ; entry.contents) {
				WriteEntry(e);
			}
		}
		else {
			file.rawWrite(cast(ubyte[]) read(entry.fullPath));
		}
	}

	void WriteEntries(FileEntry root) {
		// write header
		Write16(0); // version
		Write8(0); // unused
		Write32(0); // string table length, will be written to later

		srand(cast(int) time(null));
		Write32(rand()); // random number

		// write string table
		size_t size;
		foreach (ref str ; stringTable) {
			WriteString(str.value);
			size += str.value.length + 1;
		}

		auto pos = file.tell();
		file.seek(3, SEEK_SET);
		Write32(cast(uint) size);
		file.seek(pos, SEEK_SET);

		// write entry
		WriteEntry(root);
	}

	size_t StringTableIndex(string value) {
		foreach (ref e ; stringTable) {
			if (e.value == value) {
				return e.offset;
			}
		}

		if (stringTable.length == 0) {
			stringTable ~= StrTableEntry(0, value);
		}
		else {
			stringTable ~= StrTableEntry(
				stringTable[$ - 1].offset + stringTable[$ - 1].value.length + 1, value
			);
		}

		return stringTable[$ - 1].offset;
	}

	string StringTableValue(size_t offset) {
		foreach (ref e ; stringTable) {
			if (e.offset == offset) {
				return e.value;
			}
		}

		assert(0);
	}

	FileEntry GetFolder(string path) {
		FileEntry entry;
		entry.folder     = true;
		entry.fullPath   = path;
		entry.pathOffset = StringTableIndex(path.baseName());

		foreach (DirEntry e ; dirEntries(path, SpanMode.shallow)) {
			if (e.isDir) {
				entry.contents ~= GetFolder(e.name);
			}
			else {
				entry.contents ~= FileEntry(
					false, e.name, StringTableIndex(e.name.baseName())
				);
			}
		}

		return entry;
	}

	void Write(string path) {
		WriteEntries(GetFolder(path));
	}
}
