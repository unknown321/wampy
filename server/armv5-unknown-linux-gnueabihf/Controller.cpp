#include "Controller.moc"
#include "moc_Controller.cpp"

#include "../qt/qtbase/src/corelib/kernel/qobject_p.h"
#include "command.pb.h"
#include "helpers.h"
#include <QVector>

// main screen with icons is `gridArea -> MusicPlayerTop`, 93, qrc:/sid_0901_library/Contents.qml

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
    DLOG("\n");
    if (provider.usbMounted) {
        DLOG("usb on\n");
        return false;
    }

    if (NavBar == nullptr) {
        DLOG("no navbar\n");
        return false;
    }

    if (!QMetaObject::invokeMethod(NavBar, "GoToPlayer", Qt::DirectConnection)) {
        DLOG("cannot invoke GoToPlayer\n");
        return false;
    }

    return true;
}

bool Controller::Ready() const { return initialized; }

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

    FrameWork = swipeItem->parent();

    auto gridArea = swipeItem->childItems()[0]->childItems()[0];
    int s = gridArea->childItems().size();
    for (int i = 0; i < s; i++) {
        auto loader = gridArea->childItems()[i];
        for (auto kid : loader->childItems()) {
            if (QString(kid->metaObject()->className()).startsWith("DAC_QMLTYPE_")) {
                DAC = kid;

                auto dacLoader = DAC->parent();
                if (dacLoader == nullptr) {
                    continue;
                }
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
        }

        if (MusicPlayerModel != nullptr && DAC != nullptr) {
            break;
        }
    }

    if (MusicPlayerModel != nullptr) {
        getFromModelManager();
        connectFromModelManager();
        initializeVolumeValue();
        getMusicWindow();
        initBasicPlayerControls();
        getAudioSourceMgr();
        connectVolumeSlot();
    }

    if (AudioSourceMgr == nullptr) {
        bpcTimer = new QTimer();
        bpcTimer->setInterval(1000);
        QObject::connect(bpcTimer, SIGNAL(timeout()), this, SLOT(WaitForAudioSource()), Qt::DirectConnection);
        bpcTimer->moveToThread(MusicPlayerModel->thread());
        bpcTimer->start();
        DLOG("started timer\n");
    }

    initialized = true;

    provider.Start();

    provider.FromDAC(DACViewModel); // first volume pull

    return 0;
}

void Controller::WaitForAudioSource() {
    initBasicPlayerControls();
    getAudioSourceMgr();
    initNavBar();
    if (AudioSourceMgr != nullptr && NavBar != nullptr) {
        bpcTimer->stop();
        DLOG("stopped timer\n");
    }
}

void Controller::getMusicWindow() {
    if (MusicWindow != nullptr) {
        DLOG("already found\n");
        return;
    }

    auto res = jsExpression("swipeItem");
    if (!res.isValid()) {
        DLOG("cannot get swipeItem\n");
        return;
    }

    auto pp = jsExpression("popupParent");
    if (!pp.isValid()) {
        DLOG("popupParent invalid\n");
        return;
    }
    auto swipeItem = qvariant_cast<QQuickItem *>(res);

    for (int i = 0; i < swipeItem->childItems().size(); i++) {
        auto loader = swipeItem->childItems().at(i);
        for (auto kid : loader->childItems()) {
            if (QString(kid->metaObject()->className()).startsWith("PlayerWindow_QMLTYPE_")) {
                MusicWindow = kid;
                break;
            }
        }
    }

    if (MusicWindow != nullptr) {
        auto h = QObject::connect(MusicWindow, SIGNAL(destroyed()), this, SLOT(StopRunning()), Qt::DirectConnection);
        if (!bool(h)) {
            DLOG("connect failed\n");
        }
    }
}

void Controller::StopRunning() {
    DLOG("MusicWindow is destroyed, stopping\n");
    disconnect(timeInHeader);
}

void Controller::Hide(Command::Command *c) {
    goToPlayer();

    if (!QMetaObject::invokeMethod(FrameWork, "hide", Qt::QueuedConnection)) {
        DLOG("failed to hide framework\n");
        c->set_code(Command::FAIL);
    }

    c->set_code(Command::OK);
}

void Controller::Show(Command::Command *c) {
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
        s->mutable_windowstatus()->set_visible(Command::VISIBILITY_NO);
    }

    s->set_code(Command::OK);
}

void Controller::SetVolume(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (DACViewModel == nullptr) {
        DLOG("no DACViewModel\n");
        return;
    }

    int newVal;
    if (c->setvolume().relative()) {
        int oldVal = DACViewModel->property("volume").toInt(nullptr);
        DLOG("oldval is %d\n", oldVal);
        int one = maxVolume / 100;
        if (one < 1) {
            one = 1;
        }

        newVal = oldVal + (one * c->setvolume().relativevalue());
    } else {
        newVal = c->setvolume().valuepercent() * maxVolume / 100;
    }

    if (newVal > maxVolume) {
        newVal = maxVolume;
    }

    if (newVal < 0) {
        newVal = 0;
    }

    DLOG("new volume is %d\n", newVal);

    if (!QMetaObject::invokeMethod(DACViewModel, "OnVolumeDialChanged", Q_ARG(int, newVal))) {
        DLOG("failed to change volume\n");
        return;
    }

    c->set_code(Command::OK);
}

void Controller::Seek(Command::Command *c) {
    c->set_code(Command::FAIL);
    DLOG("seek to %d\n", c->seek().value());

    auto player = getActivePlayer();
    if (player == nullptr) {
        DLOG("no active player found\n");
        return;
    }

    if (!QMetaObject::invokeMethod(player, "OnScrubBarReleased", Qt::QueuedConnection, Q_ARG(int, c->seek().value()))) {
        DLOG("seek fail\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::Prev(Command::Command *c) {
    if (BasicPlayerControls == nullptr) {
        DLOG("no basic player controls\n");
        c->set_code(Command::FAIL);
        return;
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
        DLOG("no basic player controls\n");
        c->set_code(Command::FAIL);
        return;
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
        DLOG("no basic player controls\n");
        c->set_code(Command::FAIL);
        return;
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
        DLOG("no basic player controls\n");
        c->set_code(Command::FAIL);
        return;
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
    if (provider.status->playState == PlayStateE::PLAYING) {
        auto player = getActivePlayer();
        if (player == nullptr) {
            DLOG("no active player found\n");
            return;
        }

        Pause(c);
        seekAfterPause = QObject::connect(player, SIGNAL(playingChanged()), this, SLOT(SeekToZero()), Qt::DirectConnection);
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
        c->set_code(Command::FAIL);
        return;
    }

    auto player = getActivePlayer();
    if (player == nullptr) {
        DLOG("no active player found\n");
        return;
    }

    if (!QMetaObject::invokeMethod(player, "OnRepeatClicked", Qt::QueuedConnection)) {
        DLOG("click failed\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::Shuffle(Command::Command *c) {
    if (MusicPlayerModel == nullptr) {
        c->set_code(Command::FAIL);
        return;
    }

    auto player = getActivePlayer();
    if (player == nullptr) {
        DLOG("no active player found\n");
        return;
    }

    if (!QMetaObject::invokeMethod(player, "OnShuffleClicked", Qt::QueuedConnection)) {
        DLOG("click failed\n");
        c->set_code(Command::FAIL);
        return;
    }

    c->set_code(Command::OK);
}

void Controller::getFromModelManager() {
    auto w = qobject_cast<QObject *>(MusicPlayerModel);
    auto rs = QObjectPrivate::get(w)->receiverList("playMusicChanged()");
    if (rs.empty()) {
        DLOG("no receivers found\n");
        return;
    }

    auto pun = rs.at(0); // proxyUpdateNotifier

    auto ModelManager = pun->parent();
    DLOG("ModelManager children: %d\n", ModelManager->children().size());

    for (const auto &kid : ModelManager->children()) {
        auto k = qobject_cast<QObject *>(kid);
        auto receivers = QObjectPrivate::get(k)->senderList();
        if (receivers.empty()) {
            continue;
        }

        for (const auto receiver : receivers) {
            //            DLOG(
            //                "0x%x %s %s\n",
            //                &receiver,
            //                QString(receiver->metaObject()->className()).toUtf8().constData(),
            //                QString(receiver->objectName()).toUtf8().constData()
            //            );
            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::SettingTopViewModel") && SettingTopViewModel == nullptr) {
                DLOG("found settingTopView\n");
                SettingTopViewModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::EqualizerViewModel") && EqualizerViewModel == nullptr) {
                DLOG("found Equalizer\n");
                EqualizerViewModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::VptViewModel") && VptViewModel == nullptr) {
                DLOG("found Vpt\n");
                VptViewModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::DseeAiViewModel") && DseeAiViewModel == nullptr) {
                DLOG("found Dsee\n");
                DseeAiViewModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::DcPhaseLinearizerViewModel") &&
                DcPhaseLinearizerModel == nullptr) {
                DLOG("found DC phase\n");
                DcPhaseLinearizerModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::VinylProcessorViewModel") &&
                VinylProcessorModel == nullptr) {
                DLOG("found vinyl\n");
                VinylProcessorModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::DACViewModel") && DACViewModel == nullptr) {
                DLOG("found DACViewModel\n");
                DACViewModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::ToneControlViewModel") &&
                ToneControlViewModel == nullptr) {
                DLOG("found ToneControlModel\n");
                ToneControlViewModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::DseeHxViewModel") && DseeHxViewModel == nullptr) {
                DLOG("found dmpapp::DseeHxViewModel\n");
                DseeHxViewModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::MSCViewModel") && MSCViewModel == nullptr) {
                DLOG("found dmpapp::MSCViewModel\n");
                MSCViewModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::MusicPlayerDigitalLevelMeterViewModel") &&
                MusicPlayerDigitalLevelMeterModel == nullptr) {
                DLOG("found dmpapp::MusicPlayerDigitalLevelMeterViewModel\n");
                MusicPlayerDigitalLevelMeterModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::MusicPlayerLevelMeterViewModel") &&
                MusicPlayerLevelMeterModel == nullptr) {
                DLOG("found dmpapp::MusicPlayerLevelMeterViewModel\n");
                MusicPlayerLevelMeterModel = receiver;
                break;
            }

            if (QString(receiver->metaObject()->className()).startsWith("dmpapp::MusicPlayerSpectrumViewModel") &&
                MusicPlayerSpectrumModel == nullptr) {
                DLOG("found dmpapp::MusicPlayerSpectrumViewModel\n");
                MusicPlayerSpectrumModel = receiver;
                break;
            }
        }
    }
}

void Controller::initializeVolumeValue() {
    if (DACViewModel == nullptr) {
        DLOG("no dac view model\n");
        return;
    }
    if (volumeFirstPull) {
        DLOG("already initialized\n");
        return;
    }

    if (QMetaObject::invokeMethod(DACViewModel, "OnDownButtonClicked", Qt::QueuedConnection) &&
        QMetaObject::invokeMethod(DACViewModel, "OnUpButtonClicked", Qt::QueuedConnection)) {
        volumeFirstPull = true;
        DLOG("clicked volume\n");
    } else {
        DLOG("click volume fail\n");
    }
}

void Controller::connectFromModelManager() {
    QMetaObject::Connection h;
    if (MusicPlayerModel != nullptr) {
        h = QObject::connect(MusicPlayerModel, SIGNAL(modifyEntryId()), &provider, SLOT(UpdateEntryID()), Qt::DirectConnection);
        if (!(bool)h) {
            DLOG("failed to connect modifyEntryId\n");
        }

        h = QObject::connect(MusicPlayerModel, SIGNAL(playingChanged()), &provider, SLOT(UpdateElapsed()), Qt::DirectConnection);
        if (!(bool)h) {
            DLOG("failed to connect playingChanged (currently_playing_time)\n");
        }

        DLOG("connected music player model\n");

        provider.moveToThread(MusicPlayerModel->thread());

        provider.UpdateEntryID();
    }

    if (MSCViewModel != nullptr) {
        h = QObject::connect(MSCViewModel, SIGNAL(UnmountExportedStatusChanged()), &provider, SLOT(MSCSlot()), Qt::QueuedConnection);
        if (!(bool)h) {
            DLOG("failed to connect msc\n");
        }

        provider.FromMSC(MSCViewModel);
    }
}

int Controller::toggleVpt(bool enable) {
    if (VptViewModel == nullptr) {
        DLOG("VptViewModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(VptViewModel, "VptChanged", Qt::DirectConnection, Q_ARG(bool, enable))) {
        DLOG("invoke failed\n");
    }

    return 0;
}

void Controller::SetVPT(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (VptViewModel == nullptr) {
        DLOG("VptViewModel is null\n");
        return;
    }

    if (toggleVpt(c->vpt().enabled()) == 0) {
        c->set_code(Command::OK);
    }
}

int Controller::setVptPreset(int p) {
    if (VptViewModel == nullptr) {
        DLOG("VptViewModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(VptViewModel, "VptModeNameChanged", Qt::DirectConnection, Q_ARG(int, p))) {
        DLOG("invoke failed\n");
    }
}

void Controller::SetVPTPreset(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (VptViewModel == nullptr) {
        DLOG("VptViewModel is null\n");
        return;
    }

    if (setVptPreset(c->vptpreset().preset()) == 0) {
        c->set_code(Command::OK);
    }
}

int Controller::toggleDsee(bool enable) {
    if (DseeAiViewModel == nullptr) {
        DLOG("DseeViewModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(DseeAiViewModel, "OnDseeAiChanged", Qt::DirectConnection, Q_ARG(bool, enable))) {
        DLOG("invoke failed\n");
    }

    return 0;
}

void Controller::SetDsee(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (DseeAiViewModel == nullptr) {
        DLOG("DseeViewModel is null\n");
        return;
    }

    if (toggleDsee(c->dsee().enabled()) == 0) {
        c->set_code(Command::OK);
    }
}

int Controller::toggleDCPhase(bool enable) {
    if (DcPhaseLinearizerModel == nullptr) {
        DLOG("DcPhaseLinearizerModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(DcPhaseLinearizerModel, "DcPhaseLinearizerChanged", Qt::DirectConnection, Q_ARG(bool, enable))) {
        DLOG("invoke failed\n");
    }

    return 0;
}

void Controller::SetDCPhase(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (DcPhaseLinearizerModel == nullptr) {
        DLOG("DCPhaseViewModel is null\n");
        return;
    }

    if (toggleDCPhase(c->dcphase().enabled()) == 0) {
        c->set_code(Command::OK);
    }
}

int Controller::setDCPhasePreset(int p) {
    if (DcPhaseLinearizerModel == nullptr) {
        DLOG("DcPhaseLinearizerModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(DcPhaseLinearizerModel, "DcPhaseLinearizerTypeNameChanged", Qt::DirectConnection, Q_ARG(int, p))) {
        DLOG("invoke failed\n");
    }
}

void Controller::SetDCPhasePreset(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (DcPhaseLinearizerModel == nullptr) {
        DLOG("DcPhaseLinearizerModel is null\n");
        return;
    }

    if (setDCPhasePreset(c->dcphasepreset().preset()) == 0) {
        c->set_code(Command::OK);
    }
}

int Controller::toggleVinyl(bool enable) {
    if (VinylProcessorModel == nullptr) {
        DLOG("VinylProcessorModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(VinylProcessorModel, "IsVinylProcessorChanged", Qt::DirectConnection, Q_ARG(bool, enable))) {
        DLOG("invoke failed\n");
    }

    return 0;
}

void Controller::SetVinyl(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (VinylProcessorModel == nullptr) {
        DLOG("VinylProcessorModel is null\n");
        return;
    }

    if (toggleVinyl(c->vinyl().enabled()) == 0) {
        c->set_code(Command::OK);
    }
}

void Controller::SetDirectSource(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (SettingTopViewModel == nullptr) {
        DLOG("SettingTopViewModel is null\n");
        return;
    }

    auto v = SettingTopViewModel->property("isSourceDirectOn");
    if (!v.isValid()) {
        DLOG("invalid prop\n");
        return;
    }

    if (v.toBool() == c->directsource().enabled()) {
        DLOG("no change needed, %d\n", c->directsource().enabled());
        c->set_code(Command::OK);
        return;
    }

    if (toggleDirectSource() == 0) {
        c->set_code(Command::OK);
    }
}

int Controller::toggleClearAudio() {
    if (SettingTopViewModel == nullptr) {
        DLOG("SettingTopViewModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(SettingTopViewModel, "OnClearAudioOnOffToggled", Qt::DirectConnection)) {
        DLOG("invoke failed\n");
    }

    return 0;
}

int Controller::toggleDirectSource() {
    if (SettingTopViewModel == nullptr) {
        DLOG("SettingTopViewModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(SettingTopViewModel, "OnSourceDirectOnOffToggled", Qt::DirectConnection)) {
        DLOG("invoke failed\n");
    }

    return 0;
}

int Controller::setEqPreset(int p) {
    if (EqualizerViewModel == nullptr) {
        DLOG("no equalizer model\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(EqualizerViewModel, "EqualizerPresetNameChanged", Qt::DirectConnection, Q_ARG(int, p))) {
        DLOG("invoke void EqualizerPresetNameChanged(int index) failed, index %d\n", p);
        return -1;
    }

    return 0;
}

void Controller::SetEqPreset(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (EqualizerViewModel == nullptr) {
        DLOG("EqualizerViewModel is null\n");
        return;
    }

    if (setEqPreset(c->eqpreset().preset()) == 0) {
        c->set_code(Command::OK);
    }
}

int Controller::setEqBands(QList<double> &bandValues) {
    if (EqualizerViewModel == nullptr) {
        DLOG("no equalizer model\n");
        return -1;
    }

    auto isEq6 = EqualizerViewModel->property("is_available_effect_eq6");
    if (!isEq6.isValid()) {
        DLOG("invalid prop\n");
        return -1;
    }

    for (int i = 0; i < bandValues.size(); i++) {
        if (bandValues.at(i) < -10 || bandValues.at(i) > 10) {
            DLOG("invalid band value %d: %d\n", i, bandValues.at(i));
            bandValues[i] = 0;
        }
    }

    QList<QString> mems;

    DLOG("got %d band values\n", bandValues.size());
    if (bandValues.size() == 6) {
        mems = {
            "Equalizer6bandValue100hzChanged",
            "Equalizer6bandValue400hzChanged",
            "Equalizer6bandValue1KhzChanged",
            "Equalizer6bandValue2500hzChanged",
            "Equalizer6bandValue6300hzChanged",
            "Equalizer6bandValue16KhzChanged",
        };
    } else {
        mems = {
            "Equalizer10bandValue31hzChanged",
            "Equalizer10bandValue62hzChanged",
            "Equalizer10bandValue125hzChanged",
            "Equalizer10bandValue250hzChanged",
            "Equalizer10bandValue500hzChanged",
            "Equalizer10bandValue1KhzChanged",
            "Equalizer10bandValue2KhzChanged",
            "Equalizer10bandValue4KhzChanged",
            "Equalizer10bandValue8KhzChanged",
            "Equalizer10bandValue16KhzChanged",
        };
    }

    for (int i = 0; i < mems.size(); i++) {
        if (!QMetaObject::invokeMethod(EqualizerViewModel, mems.at(i).toUtf8(), Qt::DirectConnection, Q_ARG(double, bandValues.at(i)))) {
            DLOG("invoke %s failed\n", mems.at(i).toUtf8().constData());
            return -1;
        }
    }

    return 0;
}

void Controller::SetEqBands(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (EqualizerViewModel == nullptr) {
        DLOG("EqualizerViewModel is null\n");
        return;
    }

    QList<double> bands;
    for (auto v : c->eqbands().bandvalue()) {
        bands.push_back(v);
    }

    if (setEqBands(bands) == 0) {
        c->set_code(Command::OK);
    }
}

void Controller::SetClearAudio(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (SettingTopViewModel == nullptr) {
        DLOG("SettingTopViewModel is null\n");
        return;
    }

    auto v = SettingTopViewModel->property("isClearAudioOn");
    if (!v.isValid()) {
        DLOG("invalid prop\n");
        return;
    }

    if (v.toBool() == c->clearaudio().enabled()) {
        DLOG("no change needed\n");
        c->set_code(Command::OK);
        return;
    }

    if (toggleClearAudio() == 0) {
        c->set_code(Command::OK);
    }
}

void Controller::TestCommand(Command::Command *c) {
    DLOG("test command\n");

    //    provider.GetPlaylist();
    //        jsFromFile();
}

QObject *Controller::getActivePlayer() {
    QList<QObject *> players = {MusicPlayerModel, MusicPlayerDigitalLevelMeterModel, MusicPlayerSpectrumModel, MusicPlayerLevelMeterModel};
    for (const auto &p : players) {
        if (p) {
            //            DLOG("checking %s\n", p->metaObject()->className());
            auto active = p->property("is_activated");
            if (!active.isValid()) {
                DLOG("invalid prop\n");
                continue;
            }

            if (active.toBool()) {
                DLOG("player: %s\n", p->metaObject()->className());
                return p;
            }
        } else {
            DLOG("player is null, skipping\n");
        }
    }

    return nullptr;
}

// finds BasicPlayerControls from MusicWindow
void Controller::initBasicPlayerControls() {
    if (BasicPlayerControls != nullptr) {
        return;
    }

    if (MusicWindow == nullptr) {
        DLOG("no music window\n");
        return;
    }

    QQuickItem *bpcQ = nullptr;
    auto mq = qobject_cast<QQuickItem *>(MusicWindow);
    for (auto v : mq->childItems()) {
        if (QString(v->metaObject()->className()).startsWith("BasicPlayerControls_QMLTYPE_")) {
            bpcQ = v;
            break;
        }
    }

    if (bpcQ == nullptr) {
        DLOG("cannot find bpc qml\n");
        return;
    }

    auto w = qobject_cast<QObject *>(bpcQ);
    auto connectionLists = QObjectPrivate::get(w)->connectionLists;
    if (connectionLists == nullptr) {
        DLOG("bpcq no connections\n");
        return;
    }

    for (auto vv : *connectionLists) {
        auto f = vv.first;
        if (f == nullptr) {
            continue;
        }

        if (f->receiver == nullptr) {
            continue;
        }

        if (QString(f->receiver->metaObject()->className()) == "dmpapp::BasicPlayerControls") {
            BasicPlayerControls = f->receiver;
            DLOG("found basic player controls\n");
            break;
        }
    }

    if (BasicPlayerControls == nullptr) {
        DLOG("failed to find basic player controls\n");
        return;
    }

    auto h = QObject::connect(BasicPlayerControls, SIGNAL(updated()), &provider, SLOT(UpdateBasicControls()), Qt::DirectConnection);
    if (!(bool)h) {
        DLOG("failed to connect updated\n");
    }

    //    h = QObject::connect(MusicPlayerModel, SIGNAL(repeatModeChanged()), &provider, SLOT(UpdateRepeat()), Qt::DirectConnection);
    //    if (!(bool)h) {
    //        DLOG("failed to connect repeatModeChanged\n");
    //    }
}

void Controller::connectVolumeSlot() {
    if (DAC == nullptr) {
        DLOG("no dac\n");
    }

    auto h = QObject::connect(DAC, SIGNAL(volumeChanged()), &provider, SLOT(VolumeSlot()), Qt::DirectConnection);
    DLOG("dac connected: %d\n", (bool)h);
    //    provider.moveToThread(DAC->thread());
}

void Controller::getAudioSourceMgr() {
    if (BasicPlayerControls == nullptr) {
        DLOG("no basic player controls\n");
        return;
    }

    auto senders = QObjectPrivate::get(BasicPlayerControls)->senders;
    if (senders == nullptr) {
        DLOG("senders null\n");
        return;
    }

    while (senders) {
        if (senders->sender != nullptr) {
            //            DLOG("sender %s\n", senders->sender->metaObject()->className());
            if (QString(senders->sender->metaObject()->className()) == "dmpapp::AudioSourceMgr") {
                AudioSourceMgr = senders->sender;
                auto h = QObject::connect(
                    AudioSourceMgr,
                    SIGNAL(notifyCurrentPlayState(dmpapp::PlayState)),
                    &provider,
                    SLOT(CurrentPlayState(PlayState)),
                    Qt::DirectConnection
                );
                DLOG("AudioSourceMgr connected: %d\n", (bool)h);
                break;
            }
        }

        senders = senders->next;
    }

    if (AudioSourceMgr == nullptr) {
        DLOG("failed to find AudioSourceMgr\n");
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

    auto *mp = qobject_cast<QQuickItem *>(MusicPlayer);
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

        restoreTitle();
    }

    c->set_code(Command::OK);

    featureBigCoverEnabled = c->featurebigcover().enabled();
}

// big cover is so big that artist and album are hidden under track control buttons
// marquee just works, just slap artist together with title
void Controller::UpdateTitleWithArtist() {
    if (MusicPlayer == nullptr) {
        DLOG("no music player\n");
        return;
    }

    auto *mp = qobject_cast<QQuickItem *>(MusicPlayer);
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

    regularTitle = titleV.toString();
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

void Controller::restoreTitle() {
    if (MusicPlayer == nullptr) {
        DLOG("no music player\n");
        return;
    }

    auto *mp = qobject_cast<QQuickItem *>(MusicPlayer);
    if (mp->childItems().length() < 3) {
        DLOG("not enough children\n");
        return;
    }

    auto meta = mp->childItems().at(2);

    meta->setProperty("playTitle", regularTitle);
}

// volume in header is usually destroyed on some screen transitions
void Controller::getVolumeInHeader() {
    if (MusicWindow == nullptr) {
        DLOG("no music window\n");
        return;
    }

    if (provider.status->entryId == 0) {
        return;
    }

    auto *mw = qobject_cast<QQuickItem *>(MusicWindow);
    if (mw == nullptr) {
        DLOG("MusicWindow is null\n");
        return;
    }
    if (mw->childItems().length() < 2) {
        DLOG("not enough children, got %d\n", mw->childItems().length());
        return;
    }

    auto headerArea = mw->childItems().at(1);
    if (headerArea == nullptr) {
        DLOG("headerArea is null\n");
        return;
    }
    if (headerArea->childItems().length() < 3) {
        DLOG("not enough children, got %d\n", headerArea->childItems().length());
        return;
    }

    auto status = headerArea->childItems().at(2);
    if (status == nullptr) {
        DLOG("status is null\n");
        return;
    }
    if (status->childItems().length() < 1) {
        DLOG("not enough children, got zero\n");
        return;
    }

    auto row = status->childItems().at(0);
    if (row == nullptr) {
        DLOG("row is null\n");
        return;
    }
    if (row->childItems().length() < 3) {
        DLOG("not enough children, got %d\n", row->childItems().length());
        return;
    }

    auto loader = row->childItems().at(2);
    if (loader == nullptr) {
        DLOG("loader is null\n");
        return;
    }
    if (loader->childItems().empty()) {
        DLOG("not enough children, got zero\n");
        return;
    }

    auto VolumeIcon = loader->childItems().at(0);
    if (VolumeIcon == nullptr) {
        DLOG("VolumeIcon is null\n");
        return;
    }
    if (VolumeIcon->childItems().empty()) {
        DLOG("not enough children, got zero\n");
        return;
    }

    auto area = VolumeIcon->childItems().at(0);
    if (area == nullptr) {
        DLOG("area is null\n");
        return;
    }
    if (area->childItems().length() < 2) {
        DLOG("not enough children, got %d\n", area->childItems().length());
        return;
    }

    auto volumeNum = area->childItems().at(1);
    if (volumeNum == nullptr) {
        DLOG("area is null\n");
        return;
    }
    if (volumeNum->childItems().empty()) {
        DLOG("not enough children, got zero\n");
        return;
    }

    volumeValueInHeader = volumeNum->childItems().at(0);
}

void Controller::FeatureShowClock(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (isWalkmanOne) {
        DLOG("disabled on walkmanOne\n");
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

    //    DLOG("current vol is %s\n", volumeValueInHeader->property("text").toByteArray().constData());

    QString v = "";
    if (with_time) {
        QDateTime date = QDateTime::currentDateTime();
        v = QString("%1  %2").arg(provider.status->volumeRaw, 3, 10, QChar('0')).arg(date.toString("hh:mm"));
    } else {
        v = QString("%1").arg(provider.status->volumeRaw, 3, 10, QChar('0'));
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

void Controller::FeatureSetMaxVolume(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (c->featuresetmaxvolume().enabled()) {
        maxVolume = HAGOROMO_AVLS_VOLUME_MAX;
    } else {
        maxVolume = HAGOROMO_DEFAULT_VOLUME_MAX;
    }
    provider.maxVolume = maxVolume;
    provider.FromDAC(DACViewModel);

    c->set_code(Command::OK);
}

int Controller::setToneControlLow(int v) {
    if (ToneControlViewModel == nullptr) {
        DLOG("ToneControlViewModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(ToneControlViewModel, "ToneValueLowChanged", Q_ARG(double, v))) {
        DLOG("failed to change low tone control\n");
        return -1;
    }

    return 0;
}

int Controller::setToneControlMiddle(int v) {
    if (ToneControlViewModel == nullptr) {
        DLOG("ToneControlViewModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(ToneControlViewModel, "ToneValueMiddleChanged", Q_ARG(double, v))) {
        DLOG("failed to change middle tone control\n");
        return -1;
    }

    return 0;
}

int Controller::setToneControlHigh(int v) {
    if (ToneControlViewModel == nullptr) {
        DLOG("ToneControlViewModel is null\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(ToneControlViewModel, "ToneValueHighChanged", Q_ARG(double, v))) {
        DLOG("failed to change high tone control\n");
        return -1;
    }

    return 0;
}

void Controller::SetToneControlValues(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (ToneControlViewModel == nullptr) {
        DLOG("ToneControlViewModel is null\n");
        return;
    }

    if (setToneControlLow(c->tonecontrolvalues().low()) != 0) {
        DLOG("failed to set low tone control\n");
        return;
    }

    if (setToneControlMiddle(c->tonecontrolvalues().middle()) != 0) {
        DLOG("failed to set middle tone control\n");
        return;
    }

    if (setToneControlHigh(c->tonecontrolvalues().high()) != 0) {
        DLOG("failed to set high tone control\n");
        return;
    }

    c->set_code(Command::OK);
}

void Controller::SetToneControlOrEq(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (ToneControlViewModel == nullptr || EqualizerViewModel == nullptr) {
        DLOG("ToneControlViewModel or EqualizerViewModel is nullptr\n");
        return;
    }

    if (setToneControlOrEQ(c->tonecontroloreq().eqid()) != 0) {
        DLOG("failed to set tone control or eq with value %d\n", c->tonecontroloreq().eqid());
        return;
    }

    c->set_code(Command::OK);
}

int Controller::setToneControlOrEQ(int v) {
    if (ToneControlViewModel == nullptr || EqualizerViewModel == nullptr) {
        DLOG("ToneControlViewModel or EqualizerViewModel is nullptr\n");
        return -1;
    }
    DLOG("setting eq in use to %d\n", v);

    // set to 10 band
    if (v == 2) {
        if (!QMetaObject::invokeMethod(ToneControlViewModel, "OptionMenuSelected", Q_ARG(int, 2))) {
            DLOG("failed to change to 10 band eq\n");
            return -1;
        }
        return 0;
    }

    // tone control
    if (v == 3) {
        if (!QMetaObject::invokeMethod(EqualizerViewModel, "OptionMenuSelected", Q_ARG(int, 1))) {
            DLOG("failed to change to tone control\n");
            return -1;
        }

        return 0;
    }

    DLOG("unexpected eq value %d\n", v);

    return 0;
}

void Controller::SetDseeCust(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (DseeHxViewModel == nullptr) {
        DLOG("DseeHxViewModel is nullpo\n");
        return;
    }

    if (setDseeCust(c->dseecust().enabled()) != 0) {
        DLOG("cannot set dsee ai\n");
        return;
    }

    c->set_code(Command::OK);
}

void Controller::SetDseeCustMode(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (DseeHxViewModel == nullptr) {
        DLOG("DseeHxViewModel is nullpo\n");
        return;
    }

    if (setDseeCustMode(c->dseecustmode().mode()) != 0) {
        DLOG("cannot set dsee cust mode\n");
        return;
    }

    c->set_code(Command::OK);
}

int Controller::setDseeCust(bool enable) {
    if (DseeHxViewModel == nullptr) {
        DLOG("DseeHxViewModel is nullpo\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(DseeHxViewModel, "DseeHxChanged", Q_ARG(bool, enable))) {
        DLOG("failed to toggle dsee hx cust \n");
        return -1;
    }

    return 0;
}

int Controller::setDseeCustMode(int v) {
    if (DseeHxViewModel == nullptr) {
        DLOG("DseeHxViewModel is nullpo\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(DseeHxViewModel, "DseeHxTypeNameChanged", Q_ARG(int, v))) {
        DLOG("failed to toggle dsee hx cust\n");
        return -1;
    }

    return 0;
}

void Controller::SetVinylMode(Command::Command *c) {
    c->set_code(Command::FAIL);
    if (VinylProcessorModel == nullptr) {
        DLOG("VinylProcessorModel is nullpo\n");
        return;
    }

    if (setVinylMode(c->vinylmode().mode()) != 0) {
        DLOG("cannot set vinyl mode\n");
        return;
    }

    c->set_code(Command::OK);
}

int Controller::setVinylMode(int v) {
    if (VinylProcessorModel == nullptr) {
        DLOG("VinylProcessorModel is nullpo\n");
        return -1;
    }

    if (!QMetaObject::invokeMethod(VinylProcessorModel, "OnVinylProcessorTypeChanged", Q_ARG(int, v))) {
        DLOG("failed to change vinyl mode\n");
        return -1;
    }

    return 0;
}
