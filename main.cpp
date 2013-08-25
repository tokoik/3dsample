#include <iostream>
#include <cstdlib>
#include <cmath>

// �⏕�v���O����
#include "gg.h"
using namespace gg;

// �m�C�Y
#include "Noise3.h"

// 3D �e�N�X�`���̃T�C�Y
#define TEXWIDTH  128
#define TEXHEIGHT 128
#define TEXDEPTH  128

// 3D �e�N�X�`���̃p�^�[��
#define CHECKER     0
#define SPHERE      1
#define NOISE       2
#define PERLIN      3
#define TURBULENCE  4
#define PATTERN     TURBULENCE

// �X���C�X��
#define SLICES    256

// �w�i�F�^���E�F
static const GLfloat border[] = { 1.0f, 1.0f, 1.0f, 0.0f };

//
// �X���C�X�̍쐬
//
static GLuint slice(void)
{
  // ���_�z��I�u�W�F�N�g���쐬���Č�������
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // ���_�o�b�t�@�I�u�W�F�N�g���쐬���Č�������
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // �o�b�t�@�I�u�W�F�N�g���m�ۂ���
  static const GLfloat p[] =
  {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof p, p, GL_STATIC_DRAW);

  // ���_�ʒu�� index == 0 �� in �ϐ����瓾��
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  return vao;
}

//
// �{�����[���f�[�^�̍쐬
//
static GLuint volume(GLint width, GLint height, GLint depth, int pattern)
{
  // ��Ɨp���������m�ۂ���
  GLubyte (*texture)[4] = new GLubyte[width * height * depth][4];
  GLubyte (*t)[4] = texture;

  // �m�C�Y�֐�������������
  Noise3 noise(5, 5, 5);

  // �m�C�Y�֐����g���ăe�N�X�`�������
  for (GLint k = 0; k < depth; ++k)
  {
    double z = (double)k / (double)depth;

    for (GLint j = 0; j < height; ++j)
    {
      double y = (double)j / (double)height;

      for (GLint i = 0; i < width; ++i)
      {
        double x = (double)i / (double)width;

        (*t)[0] = 160;
        (*t)[1] = 160;
        (*t)[2] = 160;

        if (pattern == CHECKER)
        {
          (*t)[3] = ((i >> 4) + (j >> 4) + (k >> 4)) & 1 ? 0 : 255;
        }
        else if (pattern == NOISE)
        {
          (*t)[3] = static_cast<GLubyte>(noise.noise(x, y, z) * 255.0);
        }
        else if (pattern == PERLIN)
        {
          (*t)[3] = static_cast<GLubyte>(noise.perlin(x, y, z, 4, 0.5) * 255.0);
        }
        else if (pattern == TURBULENCE)
        {
          (*t)[3] = static_cast<GLubyte>(noise.turbulence(x, y, z, 4, 0.5) * 255.0);
        }
        else if (pattern == SPHERE)
        {
          double px = 2.0 * x - 1.0;
          double py = 2.0 * y - 1.0;
          double pz = 2.0 * z - 1.0;

          (*t)[3] = static_cast<GLubyte>(sqrt(px * px + py * py + pz * pz) * 255.0);
        }

        ++t;
      }
    }
  }

  // �e�N�X�`���I�u�W�F�N�g���쐬���Č�������
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_3D, tex);

  // �e�N�X�`���摜�̓��[�h�P�ʂɋl�ߍ��܂�Ă���
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  // �e�N�X�`�������蓖�Ă�
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0,
    GL_RGBA, GL_UNSIGNED_BYTE, texture);

  // �e�N�X�`���̊g��E�k���ɐ��`��Ԃ�p����
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // �e�N�X�`���̋��E�F��ݒ肷��
  glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, border);

  // �e�N�X�`������͂ݏo�������ɂ͋��E�F��p����
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

  // �e�N�X�`���̌�������������
  glBindTexture(GL_TEXTURE_3D, 0);

  // ��Ɨp���������������
  delete[] texture;

  return tex;
}

//
// �����ݒ�
//
static int init(const char *title)
{
  // GLFW ������������
  if (glfwInit() == GL_FALSE)
  {
    // �������Ɏ��s����
    std::cerr << "Error: Failed to initialize GLFW." << std::endl;
    return 1;
  }

  // OpenGL Version 3.2 Core Profile ��I������
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
  glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // �E�B���h�E���J��
  if (glfwOpenWindow(800, 800, 8, 8, 8, 8, 24, 0, GLFW_WINDOW) == GL_FALSE)
  {
    // �E�B���h�E���J���Ȃ�����
    std::cerr << "Error: Failed to open GLFW window." << std::endl;
    return 1;
  }

  // �J�����E�B���h�E�ɑ΂���ݒ�
  glfwSwapInterval(1);
  glfwSetWindowTitle(title);

  // �⏕�v���O�����ɂ�鏉����
  ggInit();

  // �w�i�F
  glClearColor(border[0], border[1], border[2], border[3]);

  // �B�ʏ����̐ݒ�
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // �A���t�@�u�����f�B���O�̐ݒ�
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // ����������
  return 0;
}

// �������e�ϊ��s��
static GgMatrix mp;

// �g���b�N�{�[��
static GgTrackball tb;

//
// �E�B���h�E�̃T�C�Y�ύX���̏���
//
static void GLFWCALL resize(int w, int h)
{
  // �E�B���h�E�S�̂��r���[�|�[�g�ɂ���
  glViewport(0, 0, w, h);

  // �������e�ϊ��s������߂�i�A�X�y�N�g�� w / h�j
  mp.loadPerspective(0.6f, (GLfloat)w / (GLfloat)h, 1.0f, 10.0f);

  // �g���b�N�{�[�������͈̔͂�ݒ肷��
  tb.region(w, h);
}

//
// �}�E�X�{�^�����쎞�̏���
//
static void GLFWCALL mouse(int button, int action)
{
  // �}�E�X�̌��݈ʒu���擾����
  int x, y;
  glfwGetMousePos(&x, &y);

  switch (button)
  {
  case GLFW_MOUSE_BUTTON_LEFT:
    if (action != GLFW_RELEASE)
    {
      // ���{�^������
      tb.start(x, y);
    }
    else
    {
      // ���{�^���J��
      tb.stop(x, y);
    }
    break;
  case GLFW_MOUSE_BUTTON_MIDDLE:
    break;
  case GLFW_MOUSE_BUTTON_RIGHT:
    break;
  default:
    break;
  }
}

//
// �L�[�{�[�h
//
static void GLFWCALL keyboard(int key, int action)
{
  if (action == GLFW_PRESS)
  {
    static bool blend = true;

    switch (key)
    {
    case GLFW_KEY_SPACE:
      break;
    case GLFW_KEY_BACKSPACE:
    case GLFW_KEY_DEL:
      break;
    case GLFW_KEY_UP:
      break;
    case GLFW_KEY_DOWN:
      break;
    case GLFW_KEY_RIGHT:
      break;
    case GLFW_KEY_LEFT:
      break;
    case 'b':
    case 'B':
      blend = !blend;
      if (blend)
        glEnable(GL_BLEND);
      else
        glDisable(GL_BLEND);
      break;
    case 't':
    case 'T':
      break;
    case GLFW_KEY_ESC:
    case 'Q':
    case 'q':
      exit(0);
    default:
      break;
    }
  }
}

//
// ���C���v���O����
//
int main(int argc, const char * argv[])
{
  // �����ݒ�
  if (init("3D Texture")) return 1;

  // �X���C�X�̍쐬
  GLuint vao = slice();

  // 3D �e�N�X�`���̍쐬
  GLuint vtex = volume(TEXWIDTH, TEXHEIGHT, TEXDEPTH, PATTERN);

  // �V�F�[�_
  GLuint program = ggLoadShader("slice.vert", "slice.frag");
  GLint mtLoc = glGetUniformLocation(program, "mt");
  GLint mcLoc = glGetUniformLocation(program, "mc");
  GLint vtexLoc = glGetUniformLocation(program, "vtex");
  GLint zLoc = glGetUniformLocation(program, "z");
  GLint aLoc = glGetUniformLocation(program, "a");

  // �r���[�ϊ��s��� mv �ɋ��߂�
  GgMatrix mv = ggTranslate(-0.5f, -0.5f, -2.0f);
  //GgMatrix mv = ggLookat(1.5f, 2.0f, 2.5f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f);

  // �E�B���h�E�̃T�C�Y�ύX���ɌĂяo�������̐ݒ�
  glfwSetWindowSizeCallback(resize);

  // �}�E�X�̃{�^�����쎞�ɌĂяo�������̐ݒ�
  glfwSetMouseButtonCallback(mouse);

  // �L�[�{�[�h���쎞�ɌĂяo�������̐ݒ�
  glfwSetKeyCallback(keyboard);

  // �}�E�X�z�C�[���̒l�̃��Z�b�g
  glfwSetMouseWheel(0);

  // �A���t�@�u�����f�B���O��L����
  glEnable(GL_BLEND);

  // �E�B���h�E���J���Ă���Ԃ���Ԃ��`�悷��
  while (glfwGetWindowParam(GLFW_OPENED))
  {
    // �}�E�X�z�C�[���̒l
    GLfloat a = (GLfloat)glfwGetMouseWheel() * 0.01f + 0.5f;

    // ��ʃN���A
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // �V�F�[�_�̎g�p
    glUseProgram(program);
    glUniformMatrix4fv(mtLoc, 1, GL_TRUE, tb.get());
    glUniformMatrix4fv(mcLoc, 1, GL_FALSE, (mp * mv).get());
    glUniform1i(vtexLoc, 0);
    glUniform1f(aLoc, a);

    // 3D �e�N�X�`���̃}�b�s���O
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, vtex);

    // �X���C�X�̕`��
    glBindVertexArray(vao);
    for (int i = 0; i < SLICES; ++i)
    {
      glUniform1f(zLoc, ((GLfloat)i + 0.5f) / (GLfloat)SLICES);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // �_�u���o�b�t�@�����O
    glfwSwapBuffers();

    // �}�E�X����Ȃǂ̃C�x���g�ҋ@
    glfwWaitEvents();

    // ���}�E�X�{�^������
    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) != GLFW_RELEASE)
    {
      // �}�E�X�̌��݈ʒu���擾����
      int x, y;
      glfwGetMousePos(&x, &y);

      // ���{�^���h���b�O
      tb.motion(x, y);
    }
  }

  return 0;
}
