module ark.app;

import std.stdio;

private const string usage = "
Usage: %s OPERATION <ARGS>

Operations:
    x ARCHIVE.ark
	    Extracts ARCHIVE to the current directory
    a NAME.ark FILES...
        Creates an archive at NAME.ark containing FILES...
    l NAME.ark
        Lists all files in archive NAME.ark
";

int main(string[] args) {
	if (args.length == 0) {
		stderr.writeln("what");
		return 1;
	}
	if (args.length != 2) {
		writefln(usage, args[0]);
		return 0;
	}

	switch (args[1]) {
		case "x":
		case "a":
		case 'l':
		default: {
			stderr.writefln("Unknown operation '%s'", args[1]);
		}
	}
}
