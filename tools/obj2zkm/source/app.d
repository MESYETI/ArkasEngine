module obj2zkm.app;

import std.conv;
import std.file;
import std.range;
import std.stdio;
import std.bitmanip;
import std.algorithm;

struct Vertices {
	float[3] vertices;
}

struct Face {
	uint[3]  indices;
	ubyte[3] colour;
}

ubyte[4] ToLittleEndian(uint value) {
	return nativeToLittleEndian(value);
}

int main(string[] args) {
	if (args.length != 3) {
		stderr.writefln("Expected parameters: %s <file.obj> <file.zkm>", args[0]);
		return 1;
	}

	Vertices[] vertices;
	Face[]     faces;

	foreach (ref string line ; File(args[1], "r").lines()) {
		auto parts = line.split();

		if (parts.empty()) continue;

		switch (parts[0]) {
			case "v": {
				if (parts.length > 4) {
					stderr.writefln("Warning: vertices %d uses W value", vertices.length);
				}

				vertices ~= Vertices(
					[parse!float(parts[1]), parse!float(parts[2]), parse!float(parts[3])]
				);
				break;
			}
			case "f": {
				if (parts.length > 4) {
					stderr.writeln("More than 3 vertices per face not supported");
					stderr.writeln(line);
					return 1;
				}

				Face face;

				foreach (i ; 0 .. 3) {
					if (parts[i + 1].canFind('/')) {
						face.indices[i] = parts[i + 1].split('/')[0].parse!uint() - 1;
					}
					else {
						face.indices[i] = parts[i + 1].parse!uint() - 1;
					}
				}

				faces ~= face;
				break;
			}
			default: {
				stderr.writefln("Unsupported command: '%s'", parts[0]);
			}
		}
	}

	// create zkm file
	auto zkm = File(args[2], "wb");

	// header
	zkm.rawWrite("ZKM");
	zkm.rawWrite(cast(ubyte[]) [0]);
	zkm.rawWrite(ToLittleEndian(cast(uint) vertices.length));
	zkm.rawWrite(ToLittleEndian(cast(uint) faces.length));

	// write vertices
	foreach (ref vertex ; vertices) {
		zkm.rawWrite(ToLittleEndian(*(cast(uint*) &vertex.vertices[0])));
		zkm.rawWrite(ToLittleEndian(*(cast(uint*) &vertex.vertices[1])));
		zkm.rawWrite(ToLittleEndian(*(cast(uint*) &vertex.vertices[2])));
	}

	// write faces
	foreach (ref face ; faces) {
		if (1 /*TODO: make this a cli option*/) {
			// reverse
			zkm.rawWrite(ToLittleEndian(face.indices[0]));
			zkm.rawWrite(ToLittleEndian(face.indices[2]));
			zkm.rawWrite(ToLittleEndian(face.indices[1]));
		} else {
			// don't reverse
			zkm.rawWrite(ToLittleEndian(face.indices[0]));
			zkm.rawWrite(ToLittleEndian(face.indices[1]));
			zkm.rawWrite(ToLittleEndian(face.indices[2]));
		}
		foreach (ref colour ; face.colour) {
			zkm.rawWrite([colour]);
		}
	}

	writefln("Saved to %s", args[2]);

	return 0;
}
