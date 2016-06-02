# Configuration files
Configuration files in Kokko are JSON formatted.

## Shaders
Shader configuration files define the vertex and fragment shader files, as well
as the shader uniforms whose values are defined by a material. The allowed
values for materialUniforms.type are tex2d, mat4x4, vec4, vec3, vec2, float,
int.

```
{
  "vertexShaderFile": "vert.glsl",
  "fragmentShaderFile": "frag.glsl",
  "materialUniforms":
  [
    { "name": "diffuse_map", "type": "tex2d" },
    { "name": "color_tint", "type": "vec3" }
  ]
}
```

## Materials
Material configuration files define the shader and values for the shader's
material uniform variables. Textures are defined by the file path. Vectors and
matrices are defined by arrays of numbers. Floats and integers are single
numbers.

```
{
  "shader": "diffuse.shader.json",
  "variables":
  [
    { "name": "diffuse_map", "value": "diffuse.glraw" },
    { "name": "color_tint", "value": [ 1.0, 0.8, 0.6 ] }
  ]
}
```
