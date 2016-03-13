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
    { "name": "color", "type": "vec3" }
  ]
}
```
