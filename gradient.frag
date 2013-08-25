#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

// �e�N�X�`���̃T���v��
uniform sampler3D tex;

// �e�N�X�`�����W
in vec3 t;

// �t���[���o�b�t�@�ɏo�͂���f�[�^
layout (location = 0) out vec4 fc;

void main(void)
{
  float w = texture(tex, t).a;
  float x = w - textureOffset(tex, t, ivec3(1, 0, 0)).a;
  float y = w - textureOffset(tex, t, ivec3(0, 1, 0)).a;
  float z = w - textureOffset(tex, t, ivec3(0, 0, 1)).a;
  vec3 v = vec3(x, y, z);

  fc = vec4(normalize(v) * 0.5 + 0.5, length(v));
}
