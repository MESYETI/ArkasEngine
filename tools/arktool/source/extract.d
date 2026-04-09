module ark.extract;

import std.file;
import std.format;
import ark.read;

class Extractor {
	ArchiveReader reader;

	void ExtractHere(FileEntry folder, string where) {
		foreach (ref entry ; folder.folderContents) {
			string path = format("%s/%s", where, entry.name);

			if (entry.folder) {
				mkdir(path);
				ExtractHere(entry, path);
			}
			else {
				write(path, reader.ReadFile(entry));
			}
		}
	}

	void Extract(string archive) {
		reader = new ArchiveReader(archive);
		reader.Read();

		ExtractHere(reader.root, ".");
	}
}
