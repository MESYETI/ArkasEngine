module obj2zkm.app;

import std.conv;
import std.file;
import std.path;
import std.range;
import std.stdio;
import std.bitmanip;
import std.algorithm;

struct Vertices {
	float[3] vertices;
}

struct Face {
	uint[3] indices;
	uint[3] uv;
	uint[3] normals;
	uint    texture;
}

enum TexType {
	Link = 0,
	Png  = 1,
	Art  = 2,

	None
}

struct Texture {
	TexType type = TexType.None;

	union {
		string  link;
		ubyte[] embedded;
	}
}

struct Material {
	string  name;
	Texture texture;
}

struct UV {
	float u;
	float v;
}

struct Normal {
	float x, y, z;
}

ubyte[4] ToLittleEndian(uint value) {
	return nativeToLittleEndian(value);
}

size_t GetMaterial(ref Material[] mats, string name) {
	foreach (i, ref mat ; mats) {
		if (mat.name == name) return i;
	}

	stderr.writefln("POSSIBLE ERROR: Could not find %s", name);
	return 0;
}

int main(string[] args) {
	if (args.length != 3) {
		stderr.writefln("Expected parameters: %s <file.obj> <file.zkm>", args[0]);
		return 1;
	}

	Vertices[] vertices;
	Face[]     faces;
	Material[] materials;
	UV[]       uv;
	Normal[]   normals;

	string thisMat;
	foreach (ref string line ; File(args[1].setExtension("mtl")).lines()) {
		auto parts = line.split();

		if (parts.empty()) continue;

		switch (parts[0]) {
			case "newmtl": {
				thisMat = parts[1];
				break;
			}
			case "map_Ka": {
				string ext = parts[1].extension();

				if ((ext != "png") && (ext != "art")) {
					stderr.writefln("ERROR: Unsupported extension: '%s'", ext);
					return 1;
				}

				Texture tex;
				tex.type      = ext == "art"? TexType.Art : TexType.Png;
				tex.embedded  = cast(ubyte[]) std.file.read(parts[1]);
				materials    ~= Material(thisMat, tex);
				break;
			}
			default: {
				stderr.writefln("Unsupported MTL command: '%s'", parts[0]);
			}
		}
	}

	size_t currentMat;
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
						auto parts2 = parts[i + 1].split('/');

						face.indices[i] = parts2[0].parse!uint() - 1;

						if ((parts2.length >= 2) && (parts2[1] != "")) {
							uint idx = parts2[1].parse!uint();
							
							face.uv[i] = idx - 1;
						}
						if ((parts2.length >= 3) && (parts2[2] != "")) {
							uint idx = parts2[2].parse!uint();
							
							face.normals[i] = idx - 1;
						}
					}
					else {
						face.indices[i] = parts[i + 1].parse!uint() - 1;
					}
				}
				face.texture = cast(uint) currentMat;

				faces ~= face;
				break;
			}
			case "vn": {
				normals ~= Normal(
					parse!float(parts[1]), parse!float(parts[2]), parse!float(parts[3])
				);
				break;
			}
			case "vt": {
				uv ~= UV(
					parse!float(parts[1]),
					parts.length > 2? parse!float(parts[2]) : 0.0
				);
				break;
			}
			case "usemtl": {
				if (parts.length == 1) {
					currentMat = cast(size_t) -1;
				}
				else {
					currentMat = GetMaterial(materials, parts[1]);
				}
				break;
			}
			default: {
				stderr.writefln("Unsupported OBJ command: '%s'", parts[0]);
			}
		}
	}

	// create zkm file
	auto zkm = File(args[2], "wb");

	// header
	zkm.rawWrite("ZKM");
	zkm.rawWrite(cast(ubyte[]) [1]);
	zkm.rawWrite(ToLittleEndian(cast(uint) vertices.length));
	zkm.rawWrite(ToLittleEndian(cast(uint) faces.length));
	zkm.rawWrite(ToLittleEndian(cast(uint) uv.length));
	zkm.rawWrite(ToLittleEndian(cast(uint) materials.length));
	zkm.rawWrite(ToLittleEndian(materials.length? 1 : 0));

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
		zkm.rawWrite(ToLittleEndian(face.uv[0]));
		zkm.rawWrite(ToLittleEndian(face.uv[1]));
		zkm.rawWrite(ToLittleEndian(face.uv[2]));

		foreach (ref normal ; face.normals) {
			Normal value = normals[normal];
			zkm.rawWrite(ToLittleEndian(*(cast(uint*) &value.x)));
			zkm.rawWrite(ToLittleEndian(*(cast(uint*) &value.y)));
			zkm.rawWrite(ToLittleEndian(*(cast(uint*) &value.z)));
		}

		zkm.rawWrite(ToLittleEndian(face.texture));
	}

	// write uv
	foreach (ref value ; uv) {
		zkm.rawWrite(ToLittleEndian(*(cast(uint*) &value.u)));
		zkm.rawWrite(ToLittleEndian(*(cast(uint*) &value.v)));
	}

	// write textures
	foreach (ref texture ; materials) {
		zkm.rawWrite([cast(ubyte) texture.texture.type]);

		if (texture.texture.type == 0) {
			zkm.rawWrite(cast(ubyte[]) texture.texture.link);
		}
		else {
			zkm.rawWrite(ToLittleEndian(cast(uint) texture.texture.embedded.length));
			zkm.rawWrite(texture.texture.embedded);
		}
	}

	zkm.flush();

	writefln("Saved to %s", args[2]);
	writefln("%d vertices", vertices.length);
	writefln("%d faces", faces.length);
	writefln("%d materials", materials.length);
	writefln("%d UVs", uv.length);
	writefln("%d normals", normals.length);

	return 0;
}
