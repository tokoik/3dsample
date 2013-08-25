#include <iostream>
#include <cstdlib>
#include <cmath>

// 補助プログラム
#include "gg.h"
using namespace gg;

// ノイズ
#include "Noise3.h"

// 3D テクスチャのサイズ
#define TEXWIDTH  128
#define TEXHEIGHT 128
#define TEXDEPTH  128

// 3D テクスチャのパターン
#define CHECKER     0
#define SPHERE      1
#define NOISE       2
#define PERLIN      3
#define TURBULENCE  4
#define PATTERN     TURBULENCE

// スライス数
#define SLICES    256

// 背景色／境界色
static const GLfloat border[] = { 1.0f, 1.0f, 1.0f, 0.0f };

//
// スライスの作成
//
static GLuint slice(void)
{
  // 頂点配列オブジェクトを作成して結合する
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // 頂点バッファオブジェクトを作成して結合する
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  // バッファオブジェクトを確保する
  static const GLfloat p[] =
  {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f
  };
  glBufferData(GL_ARRAY_BUFFER, sizeof p, p, GL_STATIC_DRAW);

  // 頂点位置は index == 0 の in 変数から得る
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);

  return vao;
}

//
// ボリュームデータの作成
//
static GLuint volume(GLint width, GLint height, GLint depth, int pattern)
{
  // 作業用メモリを確保する
  GLubyte (*texture)[4] = new GLubyte[width * height * depth][4];
  GLubyte (*t)[4] = texture;

  // ノイズ関数を初期化する
  Noise3 noise(5, 5, 5);

  // ノイズ関数を使ってテクスチャを作る
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

  // テクスチャオブジェクトを作成して結合する
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_3D, tex);

  // テクスチャ画像はワード単位に詰め込まれている
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  // テクスチャを割り当てる
  glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth, 0,
    GL_RGBA, GL_UNSIGNED_BYTE, texture);

  // テクスチャの拡大・縮小に線形補間を用いる
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // テクスチャの境界色を設定する
  glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, border);

  // テクスチャからはみ出た部分には境界色を用いる
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

  // テクスチャの結合を解除する
  glBindTexture(GL_TEXTURE_3D, 0);

  // 作業用メモリを解放する
  delete[] texture;

  return tex;
}

//
// 初期設定
//
static int init(const char *title)
{
  // GLFW を初期化する
  if (glfwInit() == GL_FALSE)
  {
    // 初期化に失敗した
    std::cerr << "Error: Failed to initialize GLFW." << std::endl;
    return 1;
  }

  // OpenGL Version 3.2 Core Profile を選択する
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, 3);
  glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, 2);
  glfwOpenWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // ウィンドウを開く
  if (glfwOpenWindow(800, 800, 8, 8, 8, 8, 24, 0, GLFW_WINDOW) == GL_FALSE)
  {
    // ウィンドウが開けなかった
    std::cerr << "Error: Failed to open GLFW window." << std::endl;
    return 1;
  }

  // 開いたウィンドウに対する設定
  glfwSwapInterval(1);
  glfwSetWindowTitle(title);

  // 補助プログラムによる初期化
  ggInit();

  // 背景色
  glClearColor(border[0], border[1], border[2], border[3]);

  // 隠面消去の設定
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  // アルファブレンディングの設定
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // 初期化成功
  return 0;
}

// 透視投影変換行列
static GgMatrix mp;

// トラックボール
static GgTrackball tb;

//
// ウィンドウのサイズ変更時の処理
//
static void GLFWCALL resize(int w, int h)
{
  // ウィンドウ全体をビューポートにする
  glViewport(0, 0, w, h);

  // 透視投影変換行列を求める（アスペクト比 w / h）
  mp.loadPerspective(0.6f, (GLfloat)w / (GLfloat)h, 1.0f, 10.0f);

  // トラックボール処理の範囲を設定する
  tb.region(w, h);
}

//
// マウスボタン操作時の処理
//
static void GLFWCALL mouse(int button, int action)
{
  // マウスの現在位置を取得する
  int x, y;
  glfwGetMousePos(&x, &y);

  switch (button)
  {
  case GLFW_MOUSE_BUTTON_LEFT:
    if (action != GLFW_RELEASE)
    {
      // 左ボタン押下
      tb.start(x, y);
    }
    else
    {
      // 左ボタン開放
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
// キーボード
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
// メインプログラム
//
int main(int argc, const char * argv[])
{
  // 初期設定
  if (init("3D Texture")) return 1;

  // スライスの作成
  GLuint vao = slice();

  // 3D テクスチャの作成
  GLuint vtex = volume(TEXWIDTH, TEXHEIGHT, TEXDEPTH, PATTERN);

  // シェーダ
  GLuint program = ggLoadShader("slice.vert", "slice.frag");
  GLint mtLoc = glGetUniformLocation(program, "mt");
  GLint mcLoc = glGetUniformLocation(program, "mc");
  GLint vtexLoc = glGetUniformLocation(program, "vtex");
  GLint zLoc = glGetUniformLocation(program, "z");
  GLint aLoc = glGetUniformLocation(program, "a");

  // ビュー変換行列を mv に求める
  GgMatrix mv = ggTranslate(-0.5f, -0.5f, -2.0f);
  //GgMatrix mv = ggLookat(1.5f, 2.0f, 2.5f, 0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f);

  // ウィンドウのサイズ変更時に呼び出す処理の設定
  glfwSetWindowSizeCallback(resize);

  // マウスのボタン操作時に呼び出す処理の設定
  glfwSetMouseButtonCallback(mouse);

  // キーボード操作時に呼び出す処理の設定
  glfwSetKeyCallback(keyboard);

  // マウスホイールの値のリセット
  glfwSetMouseWheel(0);

  // アルファブレンディングを有効化
  glEnable(GL_BLEND);

  // ウィンドウが開いている間くり返し描画する
  while (glfwGetWindowParam(GLFW_OPENED))
  {
    // マウスホイールの値
    GLfloat a = (GLfloat)glfwGetMouseWheel() * 0.01f + 0.5f;

    // 画面クリア
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // シェーダの使用
    glUseProgram(program);
    glUniformMatrix4fv(mtLoc, 1, GL_TRUE, tb.get());
    glUniformMatrix4fv(mcLoc, 1, GL_FALSE, (mp * mv).get());
    glUniform1i(vtexLoc, 0);
    glUniform1f(aLoc, a);

    // 3D テクスチャのマッピング
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, vtex);

    // スライスの描画
    glBindVertexArray(vao);
    for (int i = 0; i < SLICES; ++i)
    {
      glUniform1f(zLoc, ((GLfloat)i + 0.5f) / (GLfloat)SLICES);
      glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    // ダブルバッファリング
    glfwSwapBuffers();

    // マウス操作などのイベント待機
    glfwWaitEvents();

    // 左マウスボタン操作
    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) != GLFW_RELEASE)
    {
      // マウスの現在位置を取得する
      int x, y;
      glfwGetMousePos(&x, &y);

      // 左ボタンドラッグ
      tb.motion(x, y);
    }
  }

  return 0;
}
