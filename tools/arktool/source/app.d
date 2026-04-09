module ark.app;

import std.stdio;
import std.string;
import ark.list;
import ark.write;
import ark.extract;

private const string usage = "
Usage: %s OPERATION <ARGS>

Operations:
    x ARCHIVE.ark
	    Extracts ARCHIVE to the current directory
    a NAME.ark FOLDER
        Creates an archive at NAME.ark containing the files in FOLDER
    l ARCHIVE.ark
        Lists all files in archive NAME.ark
";

int main(string[] args) {
	if (args.length == 0) {
		stderr.writeln("what");
		return 1;
	}
	if (args.length < 2) {
		writefln(usage.strip(), args[0]);
		return 0;
	}

	switch (args[1]) {
		case "x": {
			if (args.length != 3) {
				writeln("x operation requires ARCHIVE.ark parameter");
				return 1;
			}

			auto extract = new Extractor();
			extract.Extract(args[2]);
			break;
		}
		case "a": {
			if (args.length != 4) {
				writeln("a operation requires NAME.ark and FOLDER parameter");
				return 1;
			}

			auto writer = new ArchiveWriter(args[2]);
			writer.Write(args[3]);
			break;
		}
		case "l": {
			if (args.length != 3) {
				writeln("a operation requires ARCHIVE.ark parameter");
				return 1;
			}

			ListContents(args[2]);
			break;
		}
		default: {
			stderr.writefln("Unknown operation '%s'", args[1]);
		}
	}

	return 0;
}
