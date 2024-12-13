#include "Controller.moc"
#include "moc_Controller.cpp"

#include "../qt/qtbase/src/corelib/kernel/qobject_p.h"
#include "command.pb.h"
#include "helpers.h"
#include <QVector>

class QObjectConnectionListVector : public QVector<QObjectPrivate::ConnectionList> {
  public:
    bool orphaned; // the QObject owner of this vector has been destroyed while the vector was inUse
    bool dirty;    // some Connection have been disconnected (their receiver is 0) but not removed from the list yet
    int inUse;     // number of functions that are currently accessing this object or its connections
    QObjectPrivate::ConnectionList allsignals;

    QObjectConnectionListVector() : QVector<QObjectPrivate::ConnectionList>(), orphaned(false), dirty(false), inUse(0) {}

    QObjectPrivate::ConnectionList &operator[](int at) {
        if (at < 0)
            return allsignals;
        return QVector<QObjectPrivate::ConnectionList>::operator[](at);
    }
};

Controller::Controller() {}

QVariant Controller::jsExpression(const char *text) { return jsExpr(text, context, window); }

void Controller::jsFromFile() {
    QFile file("/tmp/js");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QByteArray line = file.readAll();

    auto res = jsExpression(line.constData());
    if (!res.isValid()) {
        DLOG("invalid js: %s\n", line.constData());
        file.close();
        return;
    }

    DLOG("res: %s: %s\n", qPrintable(QString(line.constData()).trimmed()), qPrintable(res.toString()));
    file.close();
}

bool Controller::goToPlayer() {
    if (provider.usbMounted) {
        DLOG("usb on\n");
        return false;
    }

    if (isOnPlayerScreen()) {
        return true;
    }

    if (NavBar == nullptr) {
        setup();
        if (NavBar == nullptr) {
            DLOG("no navbar\n");
            return false;
        }
    }

    if (!QMetaObject::invokeMethod(NavBar, "GoToPlayer", Qt::DirectConnection)) {
        DLOG("cannot invoke GoToPlayer\n");
        return false;
    }

    return true;
}

int Controller::Initialize() {
    if (initialized) {
        return 0;
    }

    getModel();

    window = getWindow();
    if (window == nullptr) {
        DLOG("window fail\n");
        return -1;
    }

    context = qmlContext(window);
    if (context == nullptr) {
        DLOG("context fail\n");
        return -1;
    }

    auto res = jsExpression("swipeItem");
    if (!res.isValid()) {
        DLOG("cannot get swipeItem\n");
        return -1;
    }

    auto swipeItem = qvariant_cast<QQuickItem *>(res);

    DLOG("framework\n");
    FrameWork = swipeItem->parent();

    if (setup() == 0) {
        initialized = true;
    }

    return 0;
}

bool Controller::detailsAfterTransition() {
    if ((bool)transitionToDetailsPopup) {
        DLOG("connection exists\n");
        return true;
    }

    if (ScreenTransitionController == nullptr) {
        setup();
    }

    if (ScreenTransitionController == nullptr) {
        DLOG("no transition controller\n");
        return false;
    }

    transitionToDetailsPopup =
        QObject::connect(ScreenTransitionController, SIGNAL(finishedTransition()), this, SLOT(invokeDetailsPopup()), Qt::DirectConnection);

    return (bool)transitionToDetailsPopup;
}

bool Controller::isOnPlayerScreen() {
    auto v = MusicPlayer->property("is_activated");
    if (!v.isValid()) {
        DLOG("invalid prop\n");
        return false;
    }

    return v.toBool();
}

// - hides qWindow
// - navigates to music player screen
// - invokes song detailed info popup
// Window might be hidden by popupReady slot in case when details popup is not ready.
void Controller::Hide(Command::Command *c) {
    for (auto conn : popupReadyConnections) {
        disconnect(conn);
    }

    popupReadyConnections.clear();

    if (provider.usbMounted) {
        DLOG("usb on, just hide\n");
        if (!QMetaObject::invokeMethod(FrameWork, "hide", Qt::QueuedConnection)) {
            DLOG("failed to hide framework\n");
            c->set_code(Command::FAIL);
        }

        c->set_code(Command::OK);

        return;
    }

    waitingForHide = true;

    auto popup = getDetailPopup();
    if (popup) {
        DLOG("details popup exists\n");
        disconnect(transitionToDetailsPopup);
        if (!QMetaObject::invokeMethod(FrameWork, "hide", Qt::QueuedConnection)) {
            DLOG("failed to hide framework\n");
            c->set_code(Command::FAIL);
        }

        c->set_code(Command::OK);

        return;
    }

    if (isOnPlayerScreen()) {
        DLOG("on player screen, creating details popup\n");
        invokeDetailsPopup();
    } else {
        DLOG("not on player screen, connecting invocation after screen transition\n");
        if (detailsAfterTransition() == false) {
            DLOG("cannot connect transition to popup invocation\n");
            c->set_code(Command::FAIL);
            return;
        }

        DLOG("connected transition, going to player screen\n");
        if (!goToPlayer()) {
            DLOG("cannot go to player screen\n");
            disconnect(transitionToDetailsPopup);
            c->set_code(Command::FAIL);
            return;
        }
    }

    c->set_code(Command::OK);
}

// - disconnects all temporary connections
// - shows qWindow
void Controller::Show(Command::Command *c) {
    disconnect(popupParentChildrenChanged);
    disconnect(transitionToDetailsPopup);
    disconnect(detailInfoPopupPositionReset);

    for (auto conn : popupReadyConnections) {
        disconnect(conn);
    }
    popupReadyConnections.clear();

    auto dpop = getDetailPopup();
    if (dpop) {
        if (!QMetaObject::invokeMethod(qobject_cast<QObject *>(dpop->childItems().at(0)), "closeDetailedInfoPopup", Qt::DirectConnection)) {
            DLOG("failed to invoke\n");
        }
    }

    if (!QMetaObject::invokeMethod(FrameWork, "show", Qt::QueuedConnection)) {
        DLOG("failed to show framework\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::GetWindowStatus(Command::Command *s) {
    s->mutable_windowstatus()->set_visible(Command::VISIBILITY_YES);
    if (window->isVisible() == false) {
        waitingForHide = false;
        s->mutable_windowstatus()->set_visible(Command::VISIBILITY_NO);
    }

    s->set_code(Command::OK);
}

void Controller::GetStatus(Command::Command *c) {
    if (!initialized) {
        DLOG("not initialized\n");
        c->set_code(Command::FAIL);
        return;
    }

    if (DACViewModel == nullptr) {
        setup();
    }

    if (!detailInfoPopupPositionReset) {
        connectDetailInfoProvider();
    } else {
        if (!detailsFirstPull) {
            detailsFirstPull = QMetaObject::invokeMethod(ContentDetailedInfo, "listPositionReset", Qt::QueuedConnection);
        }
    }

    if (DACViewModel && !volumeFirstPull) {
        if (QMetaObject::invokeMethod(DACViewModel, "OnDownButtonClicked", Qt::QueuedConnection) &&
            QMetaObject::invokeMethod(DACViewModel, "OnUpButtonClicked", Qt::QueuedConnection)) {
            volumeFirstPull = true;
            DLOG("clicked volume\n");
            DLOG("volume is %d\n", provider.volume);
        } else {
            DLOG("click volume fail\n");
        }
    }

    if (!provider.playlistActiveFound) {
        DLOG("active not found\n");
        provider.GetPlaylist();
    }

    auto s = c->mutable_status();

    s->set_shuffle(provider.shuffle);
    s->set_repeat(provider.repeat);
    s->set_playstate(provider.isPlaying);

    auto pl = s->mutable_playlist();
    for (const auto &track : provider.playlist) {
        auto t = pl->add_track();
        t->set_active(track.Active);
        t->set_artist(track.Artist.toUtf8().constData());
        t->set_title(track.Title.toUtf8().constData());
        t->set_duration(track.Duration);
        t->set_track(track.Track);
    }

    s->set_codec(provider.codec.toUtf8().constData());
    s->set_bitrate(provider.bitRate);
    s->set_samplerate(provider.sampleRate);
    s->set_bitdepth(provider.bitDepth);
    s->set_elapsed(provider.curTime);
    s->set_hires(provider.hires);
    s->set_volume(provider.volume * 100 / 120);

    c->set_code(Command::OK);
}

void Controller::SetVolume(Command::Command *c) {
    if (DACViewModel == nullptr) {
        setup();
        if (DACViewModel == nullptr) {
            c->set_code(Command::FAIL);
            return;
        }
    }

    int newVal = c->setvolume().valuepercent() * 120 / 100;

    if (newVal < 0 || newVal > 120) {
        DLOG("invalid value %d\n", newVal);
        return;
    }

    int oldVal = provider.volume;
    int oldPercent = provider.volume * 100 / 120;

    DLOG("set to %d (%d%%), was %d (%d%%)\n", newVal, c->setvolume().valuepercent(), oldVal, oldPercent);

    // this is a sad case of rounding int and refusing to work with raw volume values
    if (newVal == oldVal) {
        int one = 120 / 100;
        if (oldPercent > c->setvolume().valuepercent()) {
            // decrease
            newVal -= one;
        } else {
            newVal += one;
        }
    }

    if (!QMetaObject::invokeMethod(DACViewModel, "OnVolumeDialChanged", Q_ARG(int, newVal))) {
        DLOG("failed to change volume\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::Seek(Command::Command *c) {
    DLOG("seek to %d\n", c->seek().value());
    if (!QMetaObject::invokeMethod(MusicPlayerModel, "OnScrubBarReleased", Qt::QueuedConnection, Q_ARG(int, c->seek().value()))) {
        DLOG("seek fail\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::Prev(Command::Command *c) {
    if (BasicPlayerControls == nullptr) {
        setup();
        if (BasicPlayerControls == nullptr) {
            DLOG("no basic player controls\n");
            c->set_code(Command::FAIL);
            return;
        }
    }

    if (!QMetaObject::invokeMethod(BasicPlayerControls, "OnPrevButtonClicked", Qt::QueuedConnection)) {
        DLOG("cannot click\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::Next(Command::Command *c) {
    if (BasicPlayerControls == nullptr) {
        setup();
        if (BasicPlayerControls == nullptr) {
            DLOG("no basic player controls\n");
            c->set_code(Command::FAIL);
            return;
        }
    }

    if (!QMetaObject::invokeMethod(BasicPlayerControls, "OnNextButtonClicked", Qt::QueuedConnection)) {
        DLOG("cannot click\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::Play(Command::Command *c) {
    if (BasicPlayerControls == nullptr) {
        setup();
        if (BasicPlayerControls == nullptr) {
            DLOG("no basic player controls\n");
            c->set_code(Command::FAIL);
            return;
        }
    }

    if (!QMetaObject::invokeMethod(BasicPlayerControls, "OnPlayButtonClicked", Qt::QueuedConnection)) {
        DLOG("cannot click\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::Pause(Command::Command *c) {
    if (BasicPlayerControls == nullptr) {
        setup();
        if (BasicPlayerControls == nullptr) {
            DLOG("no basic player controls\n");
            c->set_code(Command::FAIL);
            return;
        }
    }

    if (!QMetaObject::invokeMethod(BasicPlayerControls, "OnPlayButtonClicked", Qt::DirectConnection)) {
        DLOG("cannot click\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::SeekToZero() {
    auto c = Command::Command();
    c.mutable_seek()->set_value(0);
    Seek(&c);
    disconnect(seekAfterPause);
}

void Controller::Stop(Command::Command *c) {
    if (provider.isPlaying) {
        Pause(c);
        seekAfterPause = QObject::connect(MusicPlayerModel, SIGNAL(playingChanged()), this, SLOT(SeekToZero()), Qt::DirectConnection);
        if (!seekAfterPause) {
            DLOG("connection failed\n");
            return;
        }

        if (c->code() != Command::OK) {
            DLOG("failed to pause\n");
            return;
        }
    } else {
        c->mutable_seek()->set_value(0);
        Seek(c);

        if (c->code() != Command::OK) {
            DLOG("failed to seek\n");
            return;
        }
    }

    c->set_code(Command::OK);
}

void Controller::Repeat(Command::Command *c) {
    if (MusicPlayerModel == nullptr) {
        setup();
        if (MusicPlayerModel == nullptr) {
            c->set_code(Command::FAIL);
            return;
        }
    }

    if (!QMetaObject::invokeMethod(MusicPlayerModel, "OnRepeatClicked", Qt::QueuedConnection)) {
        DLOG("click failed\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::Shuffle(Command::Command *c) {
    if (MusicPlayerModel == nullptr) {
        setup();
        if (MusicPlayerModel == nullptr) {
            c->set_code(Command::FAIL);
            return;
        }
    }

    if (!QMetaObject::invokeMethod(MusicPlayerModel, "OnShuffleClicked", Qt::QueuedConnection)) {
        DLOG("click failed\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::TestCommand(Command::Command *c) {
    DLOG("test command\n");
    provider.GetPlaylist();
    //        setup();
    //        jsFromFile();
}

// from gridArea: DAC, DACViewModel, MusicPlayer, MusicPlayerModel, MSC
// from swipeItem: MusicWindow
void Controller::findViewModels(QQuickItem *swipeItem) {
    auto gridArea = swipeItem->childItems()[0]->childItems()[0];
    int s = gridArea->childItems().size();
    for (int i = 0; i < s; i++) {
        auto loader = gridArea->childItems()[i];
        for (auto kid : loader->childItems()) {
            //            DLOG("%d %s\n", i, kid->metaObject()->className());
            if (QString(kid->metaObject()->className()).startsWith("DAC_QMLTYPE_")) {
                DAC = kid;

                auto dacLoader = DAC->parent();
                if (dacLoader == nullptr) {
                    continue;
                }

                auto proxy = dacLoader->property("viewModel");
                if (!proxy.isValid()) {
                    DLOG("cannot get DAC view model proxy\n");
                    continue;
                }

                auto proxyObj = qvariant_cast<QObject *>(proxy);
                if (!proxyObj) {
                    DLOG("no proxy object for dac\n");
                    continue;
                }

                DACViewModel = proxyObj->parent();
            }

            if (QString(kid->metaObject()->className()).startsWith("MusicPlayerDefault_QMLTYPE_")) {
                MusicPlayer = kid;

                auto mLoader = MusicPlayer->parent();
                if (mLoader == nullptr) {
                    continue;
                }

                auto proxy = mLoader->property("viewModel");
                if (!proxy.isValid()) {
                    DLOG("cannot get music Player model proxy\n");
                    continue;
                }

                auto proxyObj = qvariant_cast<QObject *>(proxy);
                if (!proxyObj) {
                    DLOG("no proxy object for music player\n");
                    continue;
                }

                MusicPlayerModel = proxyObj->parent();
            }

            if (QString(kid->metaObject()->className()).startsWith("MSC_QMLTYPE_")) {
                MSC = kid;
            }
        }
    }

    for (int i = 0; i < swipeItem->childItems().size(); i++) {
        auto loader = swipeItem->childItems().at(i);
        for (auto kid : loader->childItems()) {
            if (QString(kid->metaObject()->className()).startsWith("PlayerWindow_QMLTYPE_")) {
                MusicWindow = kid;
                break;
            }
        }
    }
}

// finds BasicPlayerControls from MusicWindow
void Controller::initBasicPlayerControls() {
    if (MusicWindow != nullptr) {
        QQuickItem *bpcQ = nullptr;
        auto mq = qobject_cast<QQuickItem *>(MusicWindow);
        for (auto v : mq->childItems()) {
            if (QString(v->metaObject()->className()).startsWith("BasicPlayerControls_QMLTYPE_")) {
                bpcQ = v;
                break;
            }
        }

        if (bpcQ != nullptr) {
            auto w = qobject_cast<QObject *>(bpcQ);
            auto connectionLists = QObjectPrivate::get(w)->connectionLists;
            if (connectionLists != nullptr) {
                for (auto vv : *connectionLists) {
                    auto f = vv.first;
                    if (f != nullptr) {
                        if (f->receiver == nullptr) {
                            continue;
                        }
                        if (QString(f->receiver->metaObject()->className()) == "dmpapp::BasicPlayerControls") {
                            BasicPlayerControls = f->receiver;
                            break;
                        }
                    }
                }
            }
        }
    }
}

void Controller::initNavBar() {
    if (BasicPlayerControls != nullptr) {
        auto MusicWindowViewModel = BasicPlayerControls->parent();
        if (MusicWindowViewModel != nullptr) {
            for (auto c : MusicWindowViewModel->children()) {
                if (QString(c->metaObject()->className()) == "dmpapp::NavigationBarForSettings") {
                    NavBar = qobject_cast<QObject *>(c);
                    break;
                }
            }
        }
    }
}

void Controller::initScreenTransitionController() {
    if (NavBar == nullptr) {
        return;
    }

    auto connectionLists = QObjectPrivate::get(NavBar)->connectionLists;
    if (connectionLists != nullptr) {
        for (auto vv : *connectionLists) {
            auto f = vv.first;
            if (f != nullptr) {
                if (f->receiver == nullptr) {
                    continue;
                }

                if (QString(f->receiver->metaObject()->className()) == "dmpapp::ScreenTransitionController") {
                    ScreenTransitionController = f->receiver;
                    break;
                }
            }
        }
    }
}

void Controller::setupConnects() {
    if (MSC != nullptr && !provider.MSCHandle) {
        provider.MSCHandle = QObject::connect(MSC, SIGNAL(unmountExportedChanged()), &provider, SLOT(MSCSlot()), Qt::QueuedConnection);
        provider.FromMSC(MSC);
        DLOG("MSC connect: %d\n", (bool)provider.MSCHandle);
    } else {
        ready &= false;
    }

    if (MusicPlayer != nullptr && (!provider.MusicPlayerHandle || !provider.MusicPlayerElapsedHandle)) {
        provider.MusicPlayerHandle =
            QObject::connect(MusicPlayer, SIGNAL(meta_dataChanged()), &provider, SLOT(MusicPlayerSlot()), Qt::QueuedConnection);
        provider.FromMusicPlayer(MusicPlayer);
        DLOG("MusicPlayer connect: %d\n", (bool)provider.MusicPlayerHandle);

        provider.MusicPlayerElapsedHandle =
            QObject::connect(MusicPlayer, SIGNAL(currently_playing_timeChanged()), &provider, SLOT(UpdateElapsed()), Qt::QueuedConnection);
        DLOG("MusicPlayerElapsed connect: %d\n", (bool)provider.MusicPlayerElapsedHandle);
    } else {
        ready &= false;
    }

    if (DAC != nullptr && !provider.DACHandle) {
        provider.DACHandle = QObject::connect(DAC, SIGNAL(volumeChanged()), &provider, SLOT(VolumeSlot()), Qt::QueuedConnection);
        provider.FromDAC(DAC);
        DLOG("DAC connect: %d\n", (bool)provider.DACHandle);
    } else {
        ready &= false;
    }

    if (MusicWindow != nullptr && !provider.MusicWindowHandle) {
        provider.MusicWindowHandle =
            QObject::connect(MusicWindow, SIGNAL(basicPlayerControlsChanged()), &provider, SLOT(MusicWindowSlot()), Qt::QueuedConnection);
        provider.FromMusicWindow(MusicWindow);
        DLOG("MusicWindow connect: %d\n", (bool)provider.MusicWindowHandle);
    } else {
        ready &= false;
    }

    if (!DACViewModel) {
        ready &= false;
    }

    if (!(bool)detailInfoPopupPositionReset) {
        connectDetailInfoProvider();
    }
}

int Controller::setup() {
    if (ready) {
        DLOG("setup already done\n");
        return 0;
    }

    auto res = jsExpression("swipeItem");
    if (!res.isValid()) {
        DLOG("cannot get swipeItem\n");
        return -1;
    }

    auto pp = jsExpression("popupParent");
    if (!pp.isValid()) {
        DLOG("popupParent invalid\n");
        return -1;
    }
    PopupParent = qvariant_cast<QObject *>(pp);

    auto swipeItem = qvariant_cast<QQuickItem *>(res);
    findViewModels(swipeItem);

    initBasicPlayerControls();

    ready = true;

    if (MusicWindow == nullptr || BasicPlayerControls == nullptr) {
        ready &= false;
    }

    initNavBar();
    initScreenTransitionController();

    if (NavBar == nullptr || ScreenTransitionController == nullptr) {
        ready &= false;
    }

    if (provider.thread() != swipeItem->thread()) {
        provider.moveToThread(swipeItem->thread());
    }

    setupConnects();

    return 0;
}

// track details are provided by details popup
// info in that popup is updated by dmpapp::ContentDetailedInfo object
// there is a scrollable area in that popup that gets updated with new values on track change
void Controller::connectDetailInfoProvider() {
    auto pop = getDetailPopup();
    if (!pop) {
        DLOG("no pop\n");
        return;
    }

    auto pop0 = pop->childItems().at(0);
    auto w = qobject_cast<QObject *>(pop0);
    auto senders = QObjectPrivate::get(w)->senders;
    if (senders == nullptr) {
        DLOG("senders null\n");
        return;
    }

    while (senders) {
        if (senders->sender != nullptr) {
            DLOG("sender %s\n", senders->sender->metaObject()->className());
            if (QString(senders->sender->metaObject()->className()) == "dmpapp::ContentDetailedInfo") {
                ContentDetailedInfo = senders->sender;
                detailInfoPopupPositionReset = QObject::connect(
                    ContentDetailedInfo, SIGNAL(listPositionReset()), &provider, SLOT(UpdateDetails()), Qt::DirectConnection
                );
                DLOG("detailInfoPopup connected: %d\n", (bool)detailInfoPopupPositionReset);
                break;
            }
        }

        senders = senders->next;
    }
}

QQuickItem *Controller::getDetailPopup() {
    auto popupParent = qobject_cast<QQuickItem *>(PopupParent);
    if (!popupParent) {
        DLOG("cast failed\n");
        return nullptr;
    }

    DLOG("popups %d\n", popupParent->childItems().count());
    for (auto c : popupParent->childItems()) {
        auto popStatus = c->property("status");
        if (!popStatus.isValid()) {
            continue;
        }

        if (popStatus.toString() == "1") {
            auto pidV = c->property("popupID");
            if (!pidV.isValid()) {
                continue;
            }

            //                DLOG("popup name: %s\n", pidV.toString().toUtf8().constData());
            if (pidV.toString() == "NowPlayingContentDetailedInfoPopup") {
                return c;
            }
        } else {
            DLOG("popup not ready: %s\n", popStatus.toString().toUtf8().constData());
            auto conn = QObject::connect(c, SIGNAL(statusChanged()), this, SLOT(popupReady()), Qt::DirectConnection);
            if (conn) {
                popupReadyConnections.append(conn);
            }
        }
    }

    return nullptr;
}

// sender() is _always_ nullptr, don't try to access it
void Controller::popupReady() {
    DLOG("popup ready\n");

    if (waitingForHide) {
        DLOG("hiding\n");

        if (!QMetaObject::invokeMethod(FrameWork, "hide", Qt::DirectConnection)) {
            DLOG("failed to hide framework\n");
        }

        disconnect(transitionToDetailsPopup);
        disconnect(popupParentChildrenChanged);
        for (auto conn : popupReadyConnections) {
            disconnect(conn);
        }

        popupReadyConnections.clear();
    }
}

void Controller::invokeDetailsPopup() {
    DLOG("invoking details popup\n");
    if (!isOnPlayerScreen()) {
        DLOG("not on player screen, do nothing\n");
        return;
    }

    if (provider.usbMounted) {
        DLOG("usb on, not invoking\n");
        return;
    }

    if (!goToPlayer()) {
        DLOG("not on player screen, not invoking\n");
        return;
    }

    if (NavBar == nullptr) {
        setup();
        if (NavBar == nullptr) {
            DLOG("no navbar\n");
            return;
        }
    }

    if (!popupParentChildrenChanged) {
        popupParentChildrenChanged =
            QObject::connect(PopupParent, SIGNAL(childrenChanged()), this, SLOT(popupAdded()), Qt::DirectConnection);
    }

    if (!popupParentChildrenChanged) {
        DLOG("failed to connect popupParent childrenChanged signal\n");
        return;
    }

    auto menuID = DETAILS_POPUP_MENU_ID;
    if (isWalkmanOne) {
        menuID = DETAILS_POPUP_MENU_ID_WALKMAN_ONE;
    }

    if (!QMetaObject::invokeMethod(NavBar, "OpenOptionMenu", Qt::DirectConnection, Q_ARG(int, menuID))) {
        DLOG("cannot invoke OpenOptionMenu\n");
        return;
    }
}

/*
optimistic scenario:
popup is added and instantly ready

in practice this never happens and this function fails
*/
void Controller::popupAdded() {
    DLOG("popup added\n");
    auto pop = getDetailPopup();
    if (!pop) {
        DLOG("pop added, but no details found\n");
        return;
    }

    if (waitingForHide) {
        DLOG("hiding\n");

        if (!QMetaObject::invokeMethod(FrameWork, "hide", Qt::DirectConnection)) {
            DLOG("failed to hide framework\n");
        }

        disconnect(transitionToDetailsPopup);
        disconnect(popupParentChildrenChanged);

        for (auto conn : popupReadyConnections) {
            disconnect(conn);
        }

        popupReadyConnections.clear();
    }
}

// TODO: On disable: remove artist from title
void Controller::FeatureBigCover(Command::Command *c) {
    double height = 400;
    double imgHeight = 408;

    if (c->featurebigcover().enabled() == featureBigCoverEnabled) {
        return;
    }

    if (c->featurebigcover().enabled()) {
        height = 480;
        imgHeight = 480;
    }

    c->set_code(Command::FAIL);

    if (MusicPlayer == nullptr) {
        DLOG("no music player\n");
        return;
    }

    QQuickItem *mp = qobject_cast<QQuickItem *>(MusicPlayer);
    if (mp->childItems().length() < 4) {
        DLOG("not enough children, got %d\n", mp->childItems().length());
        return;
    }

    auto img = mp->childItems().at(1);
    if (!img->setProperty("height", imgHeight)) {
        DLOG("cannot set height\n");
        return;
    }

    if (img->childItems().length() < 1) {
        DLOG("not enough children\n");
        return;
    }

    auto coverArt = img->childItems().at(0);

    if (!coverArt->setProperty("height", height)) {
        DLOG("cannot set height\n");
        return;
    }

    if (coverArt->childItems().length() < 2) {
        DLOG("not enough children\n");
        return;
    }

    auto coverImage = coverArt->childItems().at(1);
    if (!coverImage->setProperty("height", height)) {
        DLOG("cannot set height\n");
        return;
    }

    if (!coverImage->setProperty("width", height)) {
        DLOG("cannot set width\n");
        return;
    }

    if (c->featurebigcover().enabled()) {
        UpdateTitleWithArtist();
    } else {
        if ((bool)updateTitleWithArtistBigCover) {
            disconnect(updateTitleWithArtistBigCover);
        }
    }

    c->set_code(Command::OK);

    featureBigCoverEnabled = c->featurebigcover().enabled();
}

// big cover is so big that artist and album are hidden under track control buttons
// marquee just works, just slap artist with title
void Controller::UpdateTitleWithArtist() {
    if (MusicPlayer == nullptr) {
        DLOG("no music player\n");
        return;
    }

    QQuickItem *mp = qobject_cast<QQuickItem *>(MusicPlayer);
    if (mp->childItems().length() < 3) {
        DLOG("not enough children\n");
        return;
    }

    auto meta = mp->childItems().at(2);
    QString artist = "";
    auto artistV = meta->property("artistName");
    if (artistV.isValid()) {
        artist = artistV.toString();
    }

    if (artist == "") {
        return;
    }

    auto titleV = meta->property("playTitle");
    if (!titleV.isValid()) {
        DLOG("no title to set\n");
        return;
    }

    auto newTitle = artist + " - " + titleV.toString();
    if (titleV == newTitle) {
        return;
    }

    meta->setProperty("playTitle", artist + " - " + titleV.toString());

    if ((bool)updateTitleWithArtistBigCover == false) {
        // this might fail on single track repeat
        updateTitleWithArtistBigCover =
            QObject::connect(MusicPlayer, SIGNAL(meta_dataChanged()), this, SLOT(UpdateTitleWithArtist()), Qt::DirectConnection);
        if ((bool)updateTitleWithArtistBigCover == false) {
            DLOG("connection failed\n");
        }
    }
}

// works poorly, text doesn't update and other elements must be moved so text would be visible
void Controller::getVolumeInHeaderWalkmanOne() {
    if (MusicWindow == nullptr) {
        DLOG("no music window\n");
        return;
    }

    QQuickItem *mw = qobject_cast<QQuickItem *>(MusicWindow);
    if (mw->childItems().length() < 2) {
        DLOG("not enough children, got %d\n", mw->childItems().length());
        return;
    }

    auto headerArea = mw->childItems().at(1);
    if (headerArea->childItems().length() < 3) {
        DLOG("not enough children, got %d\n", headerArea->childItems().length());
        return;
    }

    auto voluemeArea = headerArea->childItems().at(1);
    if (voluemeArea->childItems().length() < 1) {
        DLOG("not enough children, got zero\n");
        return;
    }

    auto volumeAreaLoader = voluemeArea->childItems().at(0);
    if (volumeAreaLoader->childItems().length() < 2) {
        DLOG("not enough children, got %d\n", volumeAreaLoader->childItems().length());
        return;
    }

    auto quickRectangle = volumeAreaLoader->childItems().at(1);
    if (quickRectangle->childItems().length() < 3) {
        DLOG("not enough children, got zero\n");
        return;
    }

    auto quickItem = quickRectangle->childItems().at(1);
    if (quickItem->childItems().length() < 6) {
        DLOG("not enough children, got zero\n");
        return;
    }

    volumeValueInHeader = quickItem->childItems().at(5);
}

// volume in header is usually destroyed on some screen transitions
void Controller::getVolumeInHeader() {
    if (MusicWindow == nullptr) {
        DLOG("no music window\n");
        return;
    }

    QQuickItem *mw = qobject_cast<QQuickItem *>(MusicWindow);
    if (mw->childItems().length() < 2) {
        DLOG("not enough children, got %d\n", mw->childItems().length());
        return;
    }

    auto headerArea = mw->childItems().at(1);
    if (headerArea->childItems().length() < 3) {
        DLOG("not enough children, got %d\n", headerArea->childItems().length());
        return;
    }

    auto status = headerArea->childItems().at(2);
    if (status->childItems().length() < 1) {
        DLOG("not enough children, got zero\n");
        return;
    }

    auto row = status->childItems().at(0);
    if (row->childItems().length() < 3) {
        DLOG("not enough children, got %d\n", row->childItems().length());
        return;
    }

    auto loader = row->childItems().at(2);
    if (loader->childItems().empty()) {
        DLOG("not enough children, got zero\n");
        return;
    }

    auto VolumeIcon = loader->childItems().at(0);
    if (VolumeIcon->childItems().empty()) {
        DLOG("not enough children, got zero\n");
        return;
    }

    auto area = VolumeIcon->childItems().at(0);
    if (area->childItems().length() < 2) {
        DLOG("not enough children, got %d\n", area->childItems().length());
        return;
    }

    auto volumeNum = area->childItems().at(1);
    if (volumeNum->childItems().empty()) {
        DLOG("not enough children, got zero\n");
        return;
    }

    volumeValueInHeader = volumeNum->childItems().at(0);
}

void Controller::FeatureShowClock(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (isWalkmanOne) {
        DLOG("disabled on walkmanOne");
        return;
    }

    if (c->featureshowclock().enabled() == featureShowClockEnabled) {
        return;
    }

    if (timer == nullptr) {
        timer = new QTimer();
    }

    if (c->featureshowclock().enabled() == false) {
        timer->stop();
        UpdateTime(false);
        c->set_code(Command::OK);
        return;
    }

    timer->setInterval(1000 * CLOCK_UPDATE_INTERVAL_SECONDS);
    if ((bool)timeInHeader == false) {
        timeInHeader = QObject::connect(timer, SIGNAL(timeout()), this, SLOT(UpdateTime()), Qt::DirectConnection);
        if ((bool)timeInHeader == false) {
            DLOG("failed to connect\n");
            return;
        }
    }

    timer->start();
    UpdateTime(true);

    c->set_code(Command::OK);

    featureShowClockEnabled = c->featureshowclock().enabled();
}

void Controller::UpdateTime(bool with_time) {
    if (isWalkmanOne) {
        DLOG("unsupported on WalkmanOne\n");
    } else {
        getVolumeInHeader();
    }

    if (volumeValueInHeader == nullptr) {
        DLOG("no volume in header\n");
        return;
    }

    DLOG("current vol is %s\n", volumeValueInHeader->property("text").toByteArray().constData());

    QString v = "";
    if (with_time) {
        QDateTime date = QDateTime::currentDateTime();
        v = QString("%1  %2").arg(provider.volume, 3, 10, QChar('0')).arg(date.toString("hh:mm"));
    } else {
        v = QString("%1").arg(provider.volume, 3, 10, QChar('0'));
    }

    if (!volumeValueInHeader->setProperty("text", v)) {
        DLOG("failed to set prop\n");
    }

    volumeValueInHeader = nullptr;
}

void Controller::getModel() {
    QFile file("/dev/icx_nvp/033");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    auto modelText = file.readAll();
    DLOG("model %s\n", modelText.constData());

    if (QDir("/etc/.mod").exists()) {
        isWalkmanOne = true;
    }
}

// void SearchContentData(ContentsDBEntryId entry_id) -> trackID + 0x10000000