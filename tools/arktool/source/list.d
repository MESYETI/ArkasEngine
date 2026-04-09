module ark.list;

import std.array;
import std.stdio;
import std.format;
import ark.read;

private void ListFolder(FileEntry folder, size_t depth = 0) {
	string prefix = replicate(" ", depth);

	writefln("%s%s", prefix, folder.name);

	foreach (ref entry ; folder.folderContents) {
		if (entry.folder) {
			ListFolder(entry, depth + 1);
		}
		else {
			writefln("%s %s", prefix, entry.name);
		}
	}
}

void ListContents(string archive) {
	auto reader = new ArchiveReader(archive);
	reader.Read();

	ListFolder(reader.root);
}
