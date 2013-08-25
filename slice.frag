#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

// 光源の方向
const vec3 l = vec3(0.424, 0.566, 0.707);

// テクスチャのサンプラ
uniform sampler3D vtex;

// テクスチャの勾配テクスチャのサンプラ
uniform sampler3D gtex;

// 比較するアルファ値
uniform float a;

// テクスチャ座標
in vec3 t;

// フレームバッファに出力するデータ
layout (location = 0) out vec4 fc;

void main(void)
{
  vec4 v = texture(vtex, t);
  vec4 n = texture(gtex, t);

  fc = vec4(n.rgb, (v.a - a) / (1.0 - a));
  if (fc.a < 0.0) discard;
}
