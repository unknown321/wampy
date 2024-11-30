// snippets that are of no use anymore

/*
// https://stackoverflow.com/questions/79066650
// plows through memory looking for QObject pointers
// fails
Q_DECL_UNUSED void find111() {
    auto ObjectsOfInterest = std::map<std::string, QObject *>{
            {"dmpapp::DACViewModel",                nullptr},
            {"dmpapp::MusicPlayerDefaultViewModel", nullptr}
    };

    auto b = qmlEngine(MusicPlayer);
    auto end = (char *) qobject_cast<QQmlApplicationEngine *>(b);
    DLOG("start %p -> end %p\n", qApp, end);

    auto metatable = (char *) (&QObject::staticMetaObject) - 21 * 4;
    char *curpos = (char *) qApp;
    DLOG("metatable %p\n", metatable);
    std::vector<char *> pointersToMetatable{};
    while (curpos < end) {
        curpos = curpos + 4;
        auto pointer = (void *) curpos;
        if (*(void **) pointer == (void *) metatable) {
            pointersToMetatable.emplace_back(curpos);
        }
    }

    for (auto pointer: pointersToMetatable) {
        curpos = (char *) qApp;
        while (curpos < end) {
            curpos = curpos + 4;
            auto curPointer = (void *) curpos;
            if (*(void **) curPointer == pointer) {
                DLOG("found %p, before: %p, at: %p\n", curpos - 4, *(void **) (curpos - 8),
                     *(void **) (curpos - 4));
                QObject *obj = nullptr;
                DLOG("cast obj %p to %p\n", obj, (curpos - 4));
                obj = (QObject *) (curpos - 4);
                DLOG("is widget %d\n", obj->isWidgetType());
                DLOG("thread %p\n", obj->thread());
                DLOG("object name %s\n", obj->objectName().toUtf8().constData());
                DLOG("metaget\n");

                auto meta = obj->metaObject();
                if (meta == nullptr) {
                    DLOG("meta fail\n");
                    return;
                }
            }
        }
    }

    if (ObjectsOfInterest["dmpapp::DACViewModel"] != nullptr) {
        auto dac = ObjectsOfInterest["dmpapp::DACViewModel"];
        auto volV = dac->property("volume");
        if (volV.isValid()) {
            bool ok;
            DLOG("volume: %d\n", volV.toInt(&ok));
        } else {
            DLOG("volume fail\n");
        }
    }

    if (ObjectsOfInterest["dmpapp::MusicPlayerDefaultViewModel"] != nullptr) {
        auto music = ObjectsOfInterest["dmpapp::MusicPlayerDefaultViewModel"];
        auto m = music->property("meta_data");
        auto map = m.toMap();
        auto album = map["album_name"].toString();
        auto artist = map["artist_name"].toString();
        auto title = map["play_title"].toString();
        DLOG("%s %s (%s)\n", artist.toUtf8().constData(), title.toUtf8().constData(), album.toUtf8().constData());
        bool ok;
        auto time = music->property("total_playback_time").toDouble(&ok);
        auto repeat = music->property("repeat_mode").toInt(&ok);
        auto shuffle = music->property("is_shuffle").toBool();
        auto curtime = music->property("currently_playing_time").toDouble(&ok);
        DLOG("%f / %f; %d %d\n", curtime, time, shuffle, repeat);
    }
}
 */

/*
    // clicks on toolbox button, selects corresponding menu item, spawns popup
    Q_DECL_UNUSED int spawnDetailedInfoPopup() {
        if (!isOnPlayerScreen()) {
            DLOG("wrong screen\n");
            return HGRM_RESPONSE_WRONG_SCREEN;
        }

        // swipeItem->qmlLoader->musicWindow->navigationBar
        auto navBarVar = jsExpression("swipeItem.children[9].children[0].children[6]");
        if (!navBarVar.isValid()) {
            DLOG("cannot find navbar\n");
            return -1;
        }

        auto navBar = qvariant_cast<QQuickItem *>(navBarVar);

        auto optionButtonV = navBar->childItems().at(4);
        QObject *optionButton = qobject_cast<QObject *>(optionButtonV);

        MyMouse *mouse = new MyMouse();
        mouse->x = 10;
        mouse->y = 10;
        if (!QMetaObject::invokeMethod(optionButton, "clicked", Qt::BlockingQueuedConnection,
                                       Q_ARG(QObject *, mouse))) {
            DLOG("cannot click navbar options button\n");
            return -1;
        }

        struct timespec tw = {0, 300000000};
        int i = 0;
        for (;;) {
            if (i > 10) {
                DLOG("cannot get loader base after %d retries\n", i);
                return -1;
            }

            auto loaderLengthV = jsExpression("loader.children[0].children.length");
            if (!loaderLengthV.isValid()) {
                DLOG("cannot get loader base\n");
                return -1;
            }

            if (loaderLengthV.toInt() == 3) {
                break;
            }

            nanosleep(&tw, nullptr);
            i++;
        }

        // loader->dmpDialog->loader
        auto loaderV = jsExpression("loader.children[0].children[2]");
        if (!loaderV.isValid()) {
            DLOG("cannot get loader base\n");
            return -1;
        }

        auto loaderQ = qvariant_cast<QQuickItem *>(loaderV);
        i = 0;
        for (;;) {
            if (i > 10) {
                DLOG("cannot get base from loader\n");
                return -1;
            }

            if (loaderQ->childItems().length() > 0) {
                break;
            }

            nanosleep(&tw, nullptr);
            i++;
        }

        auto baseQ = loaderQ->childItems().at(0);
        // base -> menuArea -> levelOneMenu -> list -> QQuickItem.children
        auto m = baseQ->childItems().at(1)->childItems().at(0)->childItems().at(1)->childItems().at(0)->childItems();
        i = 0;
        for (;;) {
            if (i > 10) {
                DLOG("cannot get menu length\n");
                return -1;
            }

            if (m.length() == 7) {
                break;
            }

            nanosleep(&tw, nullptr);
            i++;
        }

        auto base = qobject_cast<QObject *>(baseQ);

        QJSValue item_id = QJSValue(3);
        if (!QMetaObject::invokeMethod(base, "selected", Qt::BlockingQueuedConnection, Q_ARG(QJSValue, item_id))) {
            DLOG("cannot select menu option\n");
            return -1;
        }

        nanosleep(&tw, nullptr);
        if (!QMetaObject::invokeMethod(base, "accepted", Qt::BlockingQueuedConnection)) {
            DLOG("cannot accept menu option\n");
            return -1;
        }

        return 0;
    }
 */

/*
     Q_DECL_UNUSED void closePopups() {
        auto popupParentV = jsExpression("popupParent");
        if (!popupParentV.isValid()) {
            DLOG("failed to find popup parent\n");
            return;
        }

        QQuickItem *popupParent = qvariant_cast<QQuickItem *>(popupParentV);
        if (popupParent == nullptr) {
            DLOG("failed to cast popup parent\n");
            return;
        }

        for (int i = (popupParent->childItems().length() - 1); i >= 0; i--) {
            auto kid = popupParent->childItems().at(i);

            if (kid->property("popupID") == "NowPlayingContentDetailedInfoPopup") {
                if (!QMetaObject::invokeMethod(kid->childItems().at(0), "closeDetailedInfoPopup",
                                               Qt::BlockingQueuedConnection)) {
                    DLOG("fail close detail popup \n");
                }
            }

            if (kid->property("popupID") == "VolumePopup") {
                if (!QMetaObject::invokeMethod(kid->childItems().at(0), "backButtonClicked",
                                               Qt::BlockingQueuedConnection)) {
                    DLOG("fail close volume popup\n");
                }
            }
        }
    }
 */

/*
    class QObjectConnectionListVector : public QVector<QObjectPrivate::ConnectionList> {
    public:
        bool orphaned; //the QObject owner of this vector has been destroyed while the vector was inUse
        bool dirty; //some Connection have been disconnected (their receiver is 0) but not removed from the list yet
        int inUse; //number of functions that are currently accessing this object or its connections
        QObjectPrivate::ConnectionList allsignals;

        QObjectConnectionListVector()
                : QVector<QObjectPrivate::ConnectionList>(), orphaned(false), dirty(false), inUse(0) {}

        QObjectPrivate::ConnectionList &operator[](int at) {
            if (at < 0)
                return allsignals;
            return QVector<QObjectPrivate::ConnectionList>::operator[](at);
        }
    };

    Q_DECL_UNUSED QObject *getViaReceiver(QString name, QObject *obj) {
        DLOG("%s\n", obj->metaObject()->className());
        auto data = obj->property("data");
        if (data.isValid()) {
            auto dl = data.toList();
            DLOG("dl len: %d\n", dl.length());
        } else {
            DLOG("no data property\n");
        }

        auto connectionLists = QObjectPrivate::get(obj)->connectionLists;
        for (auto v: *connectionLists) {
            auto f = v.first;
            if (f != nullptr) {
                DLOG("sender: %s\n", f->sender->metaObject()->className());
                DLOG("receiver: %s\n", f->receiver->metaObject()->className());
                if (QString(f->receiver->metaObject()->className()) == name) {
                    DLOG("inloop %s %p\n", f->receiver->metaObject()->className(), f->receiver);
                    return f->receiver;
                }
            }
        }
        return nullptr;
    }

    Q_DECL_UNUSED void followConnections() {
        auto qob = qobject_cast<QObject *>(MusicPlayer);
        auto MusicPlayerDefaultViewModel = getViaReceiver("dmpapp::MusicPlayerDefaultViewModel", qob);
        Q_ASSUME(MusicPlayerDefaultViewModel);

        auto music = MusicPlayerDefaultViewModel;
        auto m = music->property("meta_data");
        auto map = m.toMap();
        auto album = map["album_name"].toString();

        auto StandardPlayer = getViaReceiver("dmpapp::StandardPlayer", MusicPlayerDefaultViewModel);
        Q_ASSUME(StandardPlayer);
    }
 */

/*
    //    Q_DECL_UNUSED int getVolumeLabel() {
    //        // MSCWindow->headerArea->status->row->loader->volumeIcon_->area->volumeNum->valuetext
    //        QObject *headerArea = findObject(swipeItem, "headerArea");
    //        if (headerArea == nullptr) {
    //            DLOG("cannot find headerArea\n");
    //            return -1;
    //        }
    //
    //        auto volumeIcon = findObjectByMetaType(headerArea, QString("VolumeIcon_"));
    //        if (volumeIcon == nullptr) {
    //            DLOG("cannot find volumeIcon\n");
    //            return -1;
    //        }
    //
    //        QObject *VolumeLabel = findObjectByMetaType(volumeIcon, QString("SCmnMonospaceLabel_"));
    //        if (VolumeLabel == nullptr) {
    //            DLOG("cannot find volumeLabel\n");
    //            return -1;
    //        }
    //
    //        return 0;
    //    }
    //
    */

/*
bool StatusProvider::isOnPlayerScreen() {
    QVariant isActivated = MusicPlayer->property("is_activated");
    if (!isActivated.isValid()) {
        DLOG("invalid property\n");
        return false;
    }

    bool ok = false;
    int ready = isActivated.toInt(&ok);
    if (!ok) {
        DLOG("cannot cast isActivated property to int\n");
        return false;
    }

    return ready == 1;
}
 */
/*
QQuickItem *StatusProvider::getBasicPlayerControls() {

    // swipeItem->(9)MusicWindow->(5)BasicPlayerControls
    auto res = jsExpression("swipeItem.children[9].children[0].children[5]");
    QQuickItem *qm = qvariant_cast<QQuickItem *>(res);
    if (qm->objectName() == "basicPlayerControls") {
        return qm;
    }

    return nullptr;
}
 */

/*
int StatusProvider::spawnVolumePopup() {
    if (!isOnPlayerScreen()) {
        DLOG("not on player screen\n");
        return HGRM_FAILURE;
    }

    auto res = jsExpression("swipeItem.children[9].children[0]");
    QQuickItem *qm = qvariant_cast<QQuickItem *>(res);
    if (qm->objectName() != "MusicWindow") {
        return HGRM_FAILURE;
    }

    // MusicWindow -> headerArea_QMLTYPE_
    auto ha = qm->childItems().at(1);
    if (ha == nullptr) {
        DLOG("fail\n");
        return HGRM_FAILURE;
    }

    if (!QMetaObject::invokeMethod(ha, "volumeBarClicked")) {
        DLOG("cannot click header area\n");
        return HGRM_FAILURE;
    }

    return HGRM_OK;
}
 */

/*
void StatusProvider::setVolume(Command::Command *c) {
        QVariant res;
    struct timespec tw = {0, 300000000};

    auto popupParentV = jsExpression("popupParent");
    if (!popupParentV.isValid()) {
        DLOG("failed to find popup parent\n");
        return -1;
    }

    QQuickItem *popupParent = qvariant_cast<QQuickItem *>(popupParentV);

    QQuickItem *vpop = nullptr;
    // check if popup already exists
    for (int i = 0; i < popupParent->childItems().length(); i++) {
        auto kid = popupParent->childItems().at(i);
        if (kid->property("popupID") == "VolumePopup") {
            vpop = kid->childItems().at(0);
        }
    }

    if (vpop == nullptr) {
        DLOG("pop not found\n");
        int before = popupParent->childItems().length();
        spawnVolumePopup();

        auto i = 0;
        for (;;) {
            if (i > 10) {
                DLOG("failed to find volume dial\n");
                return -1;
            }

            if (popupParent->childItems().length() > before) {
                break;
            }

            nanosleep(&tw, nullptr);
            i++;
        }

        for (int v = 0; v < popupParent->childItems().length(); v++) {
            auto kid = popupParent->childItems().at(v);
            if (kid->property("popupID") == "VolumePopup") {
                vpop = kid->childItems().at(0);
            }
        }
    }

    if (vpop == nullptr) {
        DLOG("volume popup is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(vpop, "volumeDialChanged", Q_ARG(int, volume))) {
        DLOG("volume change fail\n");
        return -1;
    }

//    status.Volume = volume;

    return 0;
    */

/*
int StatusProvider::getVolume(int *volume) {
    return 0;
    QVariant res;
    int i = 0;
    struct timespec tw = {0, 300000000};

    if (!isOnPlayerScreen()) {
        DLOG("wrong screen\n");
        return 0;
    }

    QQuickItem *vpop = nullptr;
    for (;;) {
        if (i > 10) {
            return -1;
        }

        auto popupParentV = jsExpression("popupParent");
        if (!popupParentV.isValid()) {
            DLOG("failed to find popup parent\n");
            continue;
        }

        QQuickItem *popupParent = qvariant_cast<QQuickItem *>(popupParentV);
        if (popupParent == nullptr) {
            DLOG("failed to cast popup parent\n");
            return 0;
        }

        for (int v = (popupParent->childItems().length() - 1); v >= 0; v--) {
            auto kid = popupParent->childItems().at(v);
            //                DLOG("popup id %s\n", kid->property("popupID").toByteArray().constData());

            if (kid->property("popupID") == "VolumePopup") {
                vpop = kid->childItems().at(0);
                break;
            }
        }

        if (vpop == nullptr) {
            DLOG("invalid volume dial\n");
            spawnVolumePopup();
            nanosleep(&tw, nullptr);
            i++;
        } else {
            break;
        }
    }

    auto vol = vpop->property("volume");
    if (!vol.isValid()) {
        DLOG("invalid volume property\n");
        return -1;
    }

    bool ok = false;
    int v = vol.toInt(&ok);
    if (ok) {
        *volume = v;
    }

    return 0;
}
 */

/*
int StatusProvider::ClickTrackControlButton(int buttonID) {
    if (!initialized) {
        DLOG("not initialized\n");
        Initialize();
        //      return -1;
    }

    auto basicPlayerControls = getBasicPlayerControls();
    if (basicPlayerControls == nullptr) {
        DLOG("cannot find player controls\n");
        return -1;
    }

    //        QQuickItem *qiBpc = qobject_cast<QQuickItem *>(basicPlayerControls);
    // QQuickItem->QQuickRow->QQuickRow->SwipeDetectionButton_QMLTYPE_65_QML_73
    auto playBtn = basicPlayerControls->childItems().at(2)->childItems().at(1)->childItems().at(1)->childItems().at(
            buttonID);
    if (playBtn == nullptr) {
        DLOG("button %d not found\n", buttonID);
        return -1;
    }

    QObject *pb = qobject_cast<QObject *>(playBtn);
    if (!QMetaObject::invokeMethod(pb, "clicked")) {
        DLOG("button %d click failed\n", buttonID);
        return -1;
    }

    return 0;
}
 */

/*
void repeat() {
    if (!initialized) {
        DLOG("not initialized\n");
        Initialize();
        //      return -1;
    }

    auto basicPlayerControls = getBasicPlayerControls();
    if (basicPlayerControls == nullptr) {
        DLOG("cannot find player controls\n");
        return -1;
    }

    // QQuickItem->QQuickRow->repeat
    auto repeatButton = basicPlayerControls->childItems().at(2)->childItems().at(1)->childItems().at(2);
    if (repeatButton == nullptr) {
        DLOG("repeat button not found\n");
        return -1;
    }

    QObject *pb = qobject_cast<QObject *>(repeatButton);
    if (!QMetaObject::invokeMethod(pb, "clicked")) {
        DLOG("repeat button click failed\n");
        return -1;
    }

    return 0;
    }
*/
