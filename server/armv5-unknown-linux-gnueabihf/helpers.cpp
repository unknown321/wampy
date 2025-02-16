#include "helpers.h"

QWindow *getWindow() {
    QWindowList wl = QGuiApplication::allWindows();
    if (wl.length() == 0) {
        DLOG("no windows\n");
        return nullptr;
    }

    return wl[0];
}

void inspectObject(QObject *o, int depth) {
    QQuickItem *obj = qobject_cast<QQuickItem *>(o);
    auto cc = obj->childItems();
    if (cc.length() == 0) {
        return;
    }

    for (const auto &v : cc) {
        DLOG(
            "%sname: %s, meta: %s\n",
            std::string(depth, '\t').c_str(),
            qPrintable(v->objectName()),
            qPrintable(v->metaObject()->className())
        );
        inspectObject(v, depth + 1);
    }
}

QObject *findObject(QQuickItem *parent, const char *name) {
    for (const auto &v : parent->childItems()) {
        auto vv = v->findChildren<QObject *>(name);
        if (vv.length() > 0) {
            return vv.at(0);
        }
    }

    return nullptr;
}

QObject *findObjectByMetaType(QObject *o, QString t) {
    QQuickItem *obj = qobject_cast<QQuickItem *>(o);
    auto cc = obj->childItems();
    if (cc.length() == 0) {
        return nullptr;
    }

    for (const auto &kid : cc) {
        auto cn = QString(kid->metaObject()->className());
        if (cn.indexOf(t, 0) == 0) {
            return kid;
        };

        auto q = findObjectByMetaType(kid, t);
        if (q != nullptr) {
            return q;
        }
    }

    return nullptr;
}

static void dump_props(QObject *o) {
    auto mo = o->metaObject();
    do {
        DLOG("### Class %s\n", qPrintable(mo->className()));
        std::vector<std::pair<QString, QVariant>> v;
        v.reserve(mo->propertyCount() - mo->propertyOffset());
        for (int i = mo->propertyOffset(); i < mo->propertyCount(); ++i)
            v.emplace_back(mo->property(i).name(), mo->property(i).read(o));
        std::sort(v.begin(), v.end());
        for (auto &i : v)
            DLOG("%s\n", qPrintable(i.first));
    } while ((mo = mo->superClass()));
}

QVariant jsExpr(const char *text, QQmlContext *context, QObject *window) {
    if (context == nullptr) {
        DLOG("tried to execute %s, but context is null\n", text);
        return {};
    }

    auto expr = new QQmlExpression(context, window, text);

    auto res = expr->evaluate();

    if (expr->hasError()) {
        DLOG("err: %s\n", qPrintable(expr->error().toString()));
        return {}; // invalid
    }

    return res;
}

int getInteger(const char *text, QQmlContext *context, QObject *window) {
    auto res = jsExpr(text, context, window);

    if (!res.isValid()) {
        DLOG("invalid int");
        return 0;
    }

    bool ok = false;
    int ans = res.toInt(&ok);
    if (ok) {
        return ans;
    }

    return 0;
}

QString labelToQString(QQuickItem *label) {
    if (label->childItems().length() != 3) {
        return QString("");
    }

    auto text = label->childItems().at(1)->property("text");
    if (!text.isValid()) {
        DLOG("cannot get entry text property\n");
    }

    return text.toString();
}

void dumpBytes(std::string res) {
    for (auto c : res) {
        fprintf(stderr, "0x%02x ", c);
    }
    fprintf(stderr, "\n");
}