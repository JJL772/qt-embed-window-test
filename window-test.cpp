
#include <GL/glew.h>

#include <QApplication>
#include <QWidget>
#include <QWindow>
#include <private/qwindow_p.h>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QMainWindow>

#include <SDL2/SDL_video.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_opengl.h>

#include <unordered_map>

#include "window-test.h"

#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut

/*

What's going on in here?!?!?
I did not know until I wrote this program!

Short summary:
 * EmbeddedWindow is a subclass of QWindow
 * EmbeddedWindow will embed SDL_Window within itself
 * calls a render callback every 16ms
 * Creates a QWidget which acts as a container for the window. (This can be used to add the window to layouts and such)

Event loop:
 * Some Qt state events (FocusIn, FocusOut, Show, Hide) are given to container_
 * EmbeddedWindow::eventFilter (in a real application at least) redirects all relevant events from container_ to EmbeddedWindow,
   where those events are correctly handled. i.e. when the hide event is called, SDL should hide the window
 * container_ ONLY receives key/mouse events when the actual QWindow isn't blocking it. Meaning, if I FocusIn the window,
   then click on the frame of the application, then container_ gets those key/mouse events
    * This means that when FocusIn is received, inputsystem (or your equivalent input event system) will need to forward
      any events it receives for the native window to Qt.
      See the event handling done in draw_window for how this works
    * Custom event loops are cool because they bypass Qt's focus handling. That event loop will be continuously receiving
      events (mouse, key, etc) for that window, regardless if Qt considers it in focus, with keyboard grab, etc.

*/

QWindow* s_focus = nullptr;
std::unordered_map<Uint32, QWindow*> s_windows;

EmbeddedWindow::EmbeddedWindow(QWidget* parent, SDL_Window* window, void(*callback)(EmbeddedWindow*)) {    
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window, &info);
    WId w;
    switch(info.subsystem) {
    case SDL_SYSWM_WINDOWS:
        assert(0); break;
    case SDL_SYSWM_WAYLAND:
        w = (WId)info.info.wl.egl_window; // not tested
        break;
    case SDL_SYSWM_X11:
        w = (WId)info.info.x11.window;
        break;
    default: break;
    }

    sdl_ = window;
    s_windows.insert({SDL_GetWindowID(window), this});

    qt_window_private(this)->create(false, w);
    container_ = QWidget::createWindowContainer(this, parent);
    container_->installEventFilter(this);
    container_->setFocusPolicy(Qt::StrongFocus);

    auto* timer = new QTimer(this);
    timer->setInterval(16);
    //connect(timer, &QTimer::timeout, [this, callback]() { callback(this); container_->repaint(); });
    timer->start();
}

EmbeddedWindow::~EmbeddedWindow() {
    s_windows.erase(SDL_GetWindowID(sdl_));
}

bool EmbeddedWindow::eventFilter(QObject* object, QEvent* event) {
    if (object == container_) {
        switch (event->type()) {
        
        // We do *not* care about these events- they're coming from the widget container and will only be sent on clicks to the e.g. frame of the window
        // The actual meat of the event submission and handling is in draw_window()
        //case QEvent::MouseButtonPress:
        //    qInfo() << "Mouse press: " << static_cast<QMouseEvent*>(event)->button(); break;
        //case QEvent::MouseButtonRelease:
        //    qInfo() << "Mouse release: " << static_cast<QMouseEvent*>(event)->button(); break;
        //case QEvent::MouseMove:
        //    qInfo() << "Mouse moved: " << static_cast<QMouseEvent*>(event)->pos(); break;
        //case QEvent::KeyPress:
        //    qInfo() << "Key pressed: " << static_cast<QKeyEvent*>(event)->key(); break;
        //case QEvent::KeyRelease:
        //    qInfo() << "Key released: " << static_cast<QKeyEvent*>(event)->key(); break;
        case QEvent::Resize:
            resizeEvent(static_cast<QResizeEvent*>(event)); break;
        case QEvent::Hide:
            hideEvent(static_cast<QHideEvent*>(event)); break;
        case QEvent::Show:
            showEvent(static_cast<QShowEvent*>(event)); break;
        case QEvent::FocusIn:
            s_focus = this;
            qInfo() << "Focus gained"; break;
        case QEvent::FocusOut:
            s_focus = nullptr;
            qInfo() << "Focus lost"; break;
        default: break;
        }
    }
    return QWindow::eventFilter(object, event);
}

void EmbeddedWindow::showEvent(QShowEvent* event) {
    SDL_ShowWindow(sdl_);
}

void EmbeddedWindow::resizeEvent(QResizeEvent* event) {
    qInfo() << "resized to" << event->size();
    QWindow::resizeEvent(event);
}

void EmbeddedWindow::keyPressEvent(QKeyEvent* event) {
    qInfo() << "Key pressed: " << event->key();
    QWindow::keyPressEvent(event);
}


void EmbeddedWindow::keyReleaseEvent(QKeyEvent* event) {
    qInfo() << "Key released: " << event->key();
    QWindow::keyReleaseEvent(event);
}
void EmbeddedWindow::mouseMoveEvent(QMouseEvent* event) {
    qInfo() << "Mouse moved: local=" << event->pos() << "global=" << event->globalPosition();
    QWindow::mouseMoveEvent(event);
}
void EmbeddedWindow::mousePressEvent(QMouseEvent* event) {
    qInfo() << "Mouse clicked: " << event->pos();
    QWindow::mousePressEvent(event);
}
void EmbeddedWindow::mouseReleaseEvent(QMouseEvent* event) {
    qInfo() << "Mouse released: " << event->pos();
    QWindow::mouseReleaseEvent(event);
}

int sdl_to_qt(SDL_Keysym sym) {
    switch(sym.sym) {
    case SDLK_1: return Qt::Key_1;
    case SDLK_2: return Qt::Key_2;
    case SDLK_3: return Qt::Key_3;
    case SDLK_4: return Qt::Key_4;
    case SDLK_5: return Qt::Key_5;
    case SDLK_6: return Qt::Key_6;
    case SDLK_7: return Qt::Key_7;
    case SDLK_8: return Qt::Key_8;
    case SDLK_9: return Qt::Key_9;
    case SDLK_a: return Qt::Key_A;
    case SDLK_b: return Qt::Key_B;
    case SDLK_c: return Qt::Key_C;
    case SDLK_d: return Qt::Key_D;
    case SDLK_e: return Qt::Key_E;
    case SDLK_f: return Qt::Key_F;
    case SDLK_g: return Qt::Key_G;
    case SDLK_h: return Qt::Key_H;
    case SDLK_i: return Qt::Key_I;
    case SDLK_j: return Qt::Key_J;
    case SDLK_k: return Qt::Key_K;
    case SDLK_l: return Qt::Key_L;
    case SDLK_m: return Qt::Key_M;
    case SDLK_n: return Qt::Key_N;
    case SDLK_o: return Qt::Key_O;
    case SDLK_p: return Qt::Key_P;
    case SDLK_q: return Qt::Key_Q;
    case SDLK_r: return Qt::Key_R;
    case SDLK_s: return Qt::Key_S;
    case SDLK_t: return Qt::Key_T;
    case SDLK_u: return Qt::Key_U;
    case SDLK_v: return Qt::Key_V;
    case SDLK_x: return Qt::Key_X;
    case SDLK_y: return Qt::Key_Y;
    case SDLK_z: return Qt::Key_Z;
    default: return Qt::Key_unknown;
    }
}

Qt::MouseButton sdl_to_qt_mouse(Uint8 b) {
    Qt::MouseButton btns[5] = {
        Qt::MouseButton::LeftButton,
        Qt::MouseButton::RightButton,
        Qt::MouseButton::MiddleButton,
        Qt::MouseButton::ForwardButton,
        Qt::MouseButton::BackButton,
    };
    return btns[b % (sizeof(btns)/sizeof(btns[0]))];
}

void draw_window(EmbeddedWindow* wnd) {
    glViewport(0, 0, wnd->width(), wnd->height());
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(wnd->sdlWindow());

    // Handle SDL events, forward them to the focused QWindow
    SDL_PumpEvents();
    SDL_Event event;
    while(SDL_PollEvent(&event) != 0) {

        auto* focus = s_focus;

        // A word on focus:
        //  - Null focus is BAD for mouse events
        //  - We can determine implicit (i.e. hover) focus based on the window ID provided with the SDL event. (see the s_windows map)

        // This code is not complete obviously
        switch(event.type) {
        case SDL_MOUSEMOTION:
        {
            // null focus is not well liked, so we'll want to find the window that corresponds to this event
            if (!focus)
                focus = s_windows.find(event.motion.windowID)->second;

            auto* ev = new QMouseEvent(QEvent::MouseMove, QPointF(event.motion.x, event.motion.y),
                focus ? focus->mapToGlobal(QPointF(event.motion.x, event.motion.y)) : QPointF(event.motion.x, event.motion.y), Qt::MouseButton::NoButton, Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
            QApplication::postEvent(focus, ev);
            break;
        }
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            if (!focus)
                focus = s_windows.find(event.button.windowID)->second;

            auto* ev = new QMouseEvent(event.type == SDL_MOUSEBUTTONDOWN ? QEvent::MouseButtonPress : QEvent::MouseButtonRelease, QPointF(event.button.x, event.button.y),
                focus ? focus->mapToGlobal(QPointF(event.button.x, event.button.y)) : QPointF(event.motion.x, event.motion.y), sdl_to_qt_mouse(event.button.button), Qt::MouseButton::NoButton, Qt::KeyboardModifier::NoModifier);
            QApplication::postEvent(focus, ev);
            break;
        }
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            if (!focus)
                focus = s_windows.find(event.key.windowID)->second;

            auto* ev = new QKeyEvent(event.type == SDL_KEYDOWN ? QKeyEvent::KeyPress : QKeyEvent::KeyRelease, sdl_to_qt(event.key.keysym), Qt::KeyboardModifier::NoModifier);
            QApplication::postEvent(focus, ev);
            break;
        }
        case SDL_QUIT:
        {
            auto* ev = new QCloseEvent();
            QApplication::postEvent(nullptr, ev);
            break;
        }
        }
    }
}

int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO);
    glewInit();

    QApplication app(argc, argv);

    auto* wnd = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_OPENGL);
    auto* ctx = SDL_GL_CreateContext(wnd);
    SDL_GL_MakeCurrent(wnd, ctx);

    auto* qWnd = new EmbeddedWindow(nullptr, wnd, draw_window);
    qWnd->resize(512, 512);
    qWnd->container_->setMinimumSize(QSize(512,512));

    auto* mainw = new QMainWindow(nullptr);
    mainw->setCentralWidget(qWnd->container_);
    mainw->show();

    return app.exec();
}