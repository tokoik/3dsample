#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

// テクスチャ座標の変換行列
uniform mat4 mt;

// クリッピング座標系への変換行列
uniform mat4 mc;

// スライスの間隔
uniform float spacing;

// 頂点属性
layout (location = 0) in vec2 pv;

// スライスのテクスチャ座標
out vec3 t;

void main(void)
{
  vec4 p = vec4(pv, (float(gl_InstanceID) + 0.5) * spacing, 1.0);
  t = (mt * (p - 0.5) * 1.732 + 0.5).xyz;
  gl_Position = mc * p;
}
