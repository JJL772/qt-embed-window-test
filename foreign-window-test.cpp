
#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_vulkan.h>
#include <stdexcept>
#include <iostream>

#include <QWindow>
#include <QApplication>
#include <QMainWindow>
#include <QKeyEvent>

class GoofyWindow : public QWindow {
    Q_OBJECT
public:
    void init() {
        show();

        SDL_SetHint(SDL_HINT_VIDEO_FOREIGN_WINDOW_VULKAN, "1");
        
        window_ = SDL_CreateWindowFrom(reinterpret_cast<void*>(this->winId()));
        if (!window_) {
            std::cout << "Error: " << SDL_GetError() << "\n";
            throw std::runtime_error("Window broke, I'm sad");
        }
        SDL_SetWindowResizable(window_, SDL_TRUE);
        SDL_GetWindowSize(window_, &w_, &h_);
    }

    virtual ~GoofyWindow() {
        SDL_DestroyWindow(window_);
    }

    SDL_Window* GetSDLWindow() const { return window_; }

    void resizeEvent(QResizeEvent* ev) override {
        // Problem! SDL needs to be told when the window is resized, apparently.
        SDL_SetWindowSize(window_, ev->size().width(), ev->size().height());

        int w, h;
        
        // These should more or less return the same (besides the differences listed in docs)
        //SDL_GetWindowSize(window_, &w, &h);
        SDL_Vulkan_GetDrawableSize(window_, &w, &h);

        std::cout << "resize " << w_ << " x " << h_ << " -> " << w << " x " << h << "\n";
        w_ = w;
        h_ = h;
    }


private:
    SDL_Window* window_ = nullptr;
    int w_, h_;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow() {
        win_ = new GoofyWindow();
        win_->setMinimumSize(QSize(512,512));
        win_->init();
        container_ = QWidget::createWindowContainer(win_);
        setCentralWidget(container_);

        win_->installEventFilter(this);
    }

    bool eventFilter(QObject* obj, QEvent* ev) override {
        if (obj == win_) {
            switch(ev->type()) {
            case QEvent::Enter:
                qInfo() << "Entered widget"; break;
            case QEvent::Leave:
                qInfo() << "Left widget"; break;
            case QEvent::KeyPress:
                qInfo() << "pressed key" << static_cast<QKeyEvent*>(ev)->key(); break;
            case QEvent::KeyRelease:
                qInfo() << "released key" << static_cast<QKeyEvent*>(ev)->key(); break;
            case QEvent::MouseMove:
                qInfo() << "mouse move"; break;
            case QEvent::FocusIn:
                qInfo() << "focus in"; break;
            case QEvent::FocusOut:
                qInfo() << "focus out"; break;
            default: break;
            }
        }
        return QMainWindow::eventFilter(obj, ev);
    }

private:
    GoofyWindow* win_;
    QWidget* container_;
};



int main(int argc, char** argv) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    QApplication app(argc, argv);

    auto* a = new MainWindow();
    a->show();

    return app.exec();
}

#include "foreign-window-test.moc"