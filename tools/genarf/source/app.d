module genarf.app;

import std.conv;
import std.file;
import std.stdio;
import std.string;

const string usage = "
Usage: %s FILE.png WIDTH HEIGHT

Converts FILE.png to FILE.arf
";

void main(string[] args) {
	if (args.length != 4) {
		writefln(usage.strip(), args[0]);
	}

	ubyte width  = parse!ubyte(args[2]);
	ubyte height = parse!ubyte(args[3]);

	File file = File(args[1].setExtension("arf"), "wb");
	file.rawWrite(cast(ubyte[]) [0, 0]); // ARF version
	file.rawWrite(cast(ubyte[]) [0, 0]); // Font type
	file.rawWrite(cast(ubyte[]) [width]);
	file.rawWrite(cast(ubyte[]) [height]);

	file.rawWrite(cast(ubyte[]) read(args[1]));
}
