//
// Copyright 2015,2016,2017 Sony Corporation
//

#include "qeglfshooks.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <stdio.h>

#include "wampy.cpp"

QT_BEGIN_NAMESPACE

struct fbdev_window {
    unsigned short width;
    unsigned short height;
};

class QEglFSmt8590Hooks : public QEglFSHooks {
  public:
    virtual bool hasCapability(QPlatformIntegration::Capability cap) const;
    virtual QByteArray fbDeviceName() const;
    virtual EGLNativeDisplayType platformDisplay() const;
    virtual EGLNativeWindowType createNativeWindow(QPlatformWindow *platformWindow, const QSize &size, const QSurfaceFormat &format);
    virtual void destroyNativeWindow(EGLNativeWindowType window);
    virtual int screenDepth() const;
    virtual QDpi logicalDpi() const;
    virtual void waitForVSync() const;
    virtual bool filterConfig(EGLDisplay display, EGLConfig config) const;
};

bool QEglFSmt8590Hooks::hasCapability(QPlatformIntegration::Capability cap) const {
    switch (cap) {
    case QPlatformIntegration::ThreadedPixmaps:
    case QPlatformIntegration::OpenGL:
    case QPlatformIntegration::ThreadedOpenGL:
    case QPlatformIntegration::BufferQueueingOpenGL:
        return true;
    default:
        return false;
    }
}

QByteArray QEglFSmt8590Hooks::fbDeviceName() const {
    QByteArray fbDev = qgetenv("QT_QPA_EGLFS_FB");
    if (fbDev.isEmpty())
        fbDev = QByteArrayLiteral("/dev/graphics/fb0");
    return fbDev;
}

EGLNativeDisplayType QEglFSmt8590Hooks::platformDisplay() const { return (EGLNativeDisplayType)framebufferIndex(); }

EGLNativeWindowType
QEglFSmt8590Hooks::createNativeWindow(QPlatformWindow *platformWindow, const QSize &size, const QSurfaceFormat &format) {

    Start();

    fbdev_window *fbwin = reinterpret_cast<fbdev_window *>(malloc(sizeof(fbdev_window)));
    if (NULL == fbwin)
        return 0;

    fbwin->width = size.width();
    fbwin->height = size.height();
    return (EGLNativeWindowType)fbwin;
}

void QEglFSmt8590Hooks::destroyNativeWindow(EGLNativeWindowType window) {
    if (window != NULL) {
        free((void *)window);
    }
}

int QEglFSmt8590Hooks::screenDepth() const {
    int screenDepth = QEglFSHooks::screenDepth();
    return screenDepth;
}

QDpi QEglFSmt8590Hooks::logicalDpi() const {
    QDpi retDpi = QEglFSHooks::logicalDpi();
    return retDpi;
}

void QEglFSmt8590Hooks::waitForVSync() const { QEglFSHooks::waitForVSync(); }

bool QEglFSmt8590Hooks::filterConfig(EGLDisplay display, EGLConfig config) const { return QEglFSHooks::filterConfig(display, config); }

QEglFSmt8590Hooks eglFSmt8590Hooks;
QEglFSHooks *platformHooks = &eglFSmt8590Hooks;

QT_END_NAMESPACE