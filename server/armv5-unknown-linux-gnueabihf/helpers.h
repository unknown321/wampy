#ifndef WAMPYSERVER_HELPERS_H
#define WAMPYSERVER_HELPERS_H

#include "wampy.h"
#include <QGuiApplication>
#include <QWindow>

#define DLOG(fmt, ...) fprintf(stderr, "[wampyServer] %s %s:%d " fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__)

QWindow *getWindow();

#if SECOND_PASS == 1

#include <QApplication>
#include <QJSValue>
#include <QQmlExpression>
#include <QQuickItem>
#include <QQuickWindow>
#include <QtQml>

void inspectObject(QObject *o, int depth);

QObject *findObject(QQuickItem *parent, const char *name);

QObject *findObjectByMetaType(QObject *o, QString t);

static void dump_props(QObject *o);

// executes javascript expression
// always check result object: res.isValid()
QVariant jsExpr(const char *text, QQmlContext *context, QObject *window);

int getInteger(const char *text, QQmlContext *context, QObject *window);

QString labelToQString(QQuickItem *label);

void dumpBytes(std::string res);

#endif

#endif // WAMPYSERVER_HELPERS_H