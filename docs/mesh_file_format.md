# Mesh file format
This document describes the data within the custom format mesh files created by the Blender exporter located in *scripts/kokko_mesh_exporter*. The files created by this exporter can be loaded with *src/Resources/MeshLoader.hpp*. Current version of the exporter and format is 1.0.0.

You can install the exporter by copying the *kokko_mesh_exporter* folder to your Blender installation's *scripts/addons_contrib* folder and enabling it in the add-ons menu. Blender versions from 2.80 onwards are supported.

## File content
There are 4 main parts in the file
- Header
- Bounding box
- Vertex data
- Index data

Everything in the file is written out directly from memory. This means that the endianess of the processor will determine the byte order of each unit of data. This means that on all widely used x86 processors the byte order in the file is flipped from what is described in this document.

### Header
The header is 32 bytes long. It has 8 subparts, in the following order, each 4 bytes long.
- Constant file magic number
- Exporter version number
- Attribute info
- Bounding box start offset
- Vertex start offset
- Vertex count
- Index start offset
- Index count

#### File magic
This is a constant `0x10101991` that will be used to try to determine whether we're trying to load the correct type of file.

#### Exporter version number
The exporter will write out its version number to the file so we can make sure our loader can process the features contained in the file. The exporter version has major, minor and patch components to it. Each of those gets 1 byte of the 4 bytes, leaving one extra for future use.

```
0x00000000
        ^^ Patch version
      ^^ Minor version
	^^ Major version
```

Example: Version 1.23.4 would be `0x00011704`

#### Attribute info
The vertex attributes that are written out can be configured in the exporter and we need to be able to determine at load time which parts of the vertex should be used for what purpose. This encoding scheme is purposefully more generic than I expect to need, but there's no downside to being more flexible. It also includes the size of the index items.

The information is encoded into a 4-byte unsigned integer. Starting from the least significant bit:
- Triangle index size in bytes, 2 bits. This can be decoded with `1 << (index size - 1)`.
  - Value 0: no index data. This is not yet handled by exporter or loader, but reserved for future use.
  - Value 1: index is unsigned byte
  - Value 2: index is unsigned short
  - Value 3: index is unsigned int
- Position component count, 2 bits. This is encoded as `component count - 1`. Range is 1 to 4.
- Normal attribute count, 1 bit. Always 3 components.
- Tangent attribute count, 1 bit. Always 3 components.
- Bitangent attribute count, 1 bit. Always 3 components.
- Color attribute count, 2 bits. Range is 0 to 3.
- Color attribute 0 component count, 1 bit. This is encoded as `component count - 3`. Range is 3 to 4.
- Color attribute 1 component count, 1 bit. Encoded same as color 0.
- Color attribute 2 component count, 1 bit. Encoded same as color 0.
- Color attribute 3 component count, 1 bit. Encoded same as color 0.
- Texture coordinate attribute count, 2 bits. Range is 0 to 3.
- Texture coordinate attribute 0 component count, 1 bit. This is encoded as `component count - 2`. Range is 2 to 3.
- Texture coordinate attribute 1 component count, 1 bit. Encoded same as texture coordinate 0.
- Texture coordinate attribute 2 component count, 1 bit. Encoded same as texture coordinate 0.
- Texture coordinate attribute 3 component count, 1 bit. Encoded same as texture coordinate 0.

```
0b00000000000000000000000000000000
                                ^^ [30,31] Index size
                              ^^ [28,30] Position component count
                             ^ [27] Normal attribute count
                            ^ [26] Tangent attribute count
                           ^ [25] Bitangent attribute count
                         ^^ [23,24] Color attribute count
                        ^ [22] Color 0 component count
                       ^ [21] Color 1 component count
                      ^ [20] Color 2 component count
                     ^ [19] Color 3 component count
                   ^^ [17,18] Texture coordinates attribute count
                  ^ [16] Texture coordinate 0 component count
                 ^ [15] Texture coordinate 1 component count
                ^ [14] Texture coordinate 2 component count
               ^ [13] Texture coordinate 3 component count
```

#### Start offsets and counts
These properties are 4-byte unsigned integers. The vertex and index start offsets tell where each of the data buffers start within the file. These offsets are in bytes. The vertex and index counts determine how many vertices and triangle indices there are in the mesh.

### Bounding box
The bounding box segment has 6 floats that contain the following parameters of an axis-aligned bounding box in the same order:
- X-axis center
- Y-axis center
- Z-axis center
- X-axis extents
- Y-axis extents
- Z-axis extents

### Vertex data

### Index data
Contains the triangle indices. Size of each index is determined based on the vertex count. If 