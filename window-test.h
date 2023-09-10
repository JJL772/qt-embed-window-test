
#include <QWindow>

#pragma once

struct SDL_Window;

class EmbeddedWindow : public QWindow {
  Q_OBJECT
public:
  EmbeddedWindow(QWidget *parent, SDL_Window *window,
                 void (*callback)(EmbeddedWindow *));
  ~EmbeddedWindow();

  bool eventFilter(QObject *object, QEvent *event) override;

  void showEvent(QShowEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

  SDL_Window *sdlWindow() const { return sdl_; }

  QWidget *container_ = nullptr;
  SDL_Window *sdl_ = nullptr;
};
