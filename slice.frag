#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

// 光源の方向
const vec3 l = vec3(0.424, 0.566, 0.707);

// テクスチャのサンプラ
uniform sampler3D vtex;

// テクスチャの勾配テクスチャのサンプラ
uniform sampler3D gtex;

// 閾値
uniform float threshold;

// テクスチャ座標
in vec3 t;

// フレームバッファに出力するデータ
layout (location = 0) out vec4 fc;

void main(void)
{
  // 濃度を取り出す
  vec4 v = texture(vtex, t);

  // 濃度のアルファ値を決定する
  fc.a = (v.a - threshold) / (1.0 - threshold);

  // アルファ値が 0 以下ならフラグメントを捨てる
  if (fc.a <= 0.0) discard;

  // 濃度の勾配をそのままフラグメントの色にする
  fc.rgb = texture(gtex, t).rgb;
}
