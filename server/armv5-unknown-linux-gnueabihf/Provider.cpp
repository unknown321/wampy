#include "Provider.moc"
#include "moc_Provider.cpp"

#include "helpers.h"

#include <QJSValue>
#include <QQmlExpression>
#include <QQuickItem>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include "../qt/qtbase/src/corelib/kernel/qobject_p.h"

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

Provider::Provider() = default;

void Provider::Start() {
    int fd;

    shm_unlink(HAGOROMO_STATUS_SHM_PATH);

    fd = shm_open(HAGOROMO_STATUS_SHM_PATH, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1) {
        DLOG("shm_open: %s\n", strerror(errno));
        return;
    }

    if (ftruncate(fd, sizeof(struct HagoromoStatus)) == -1) {
        DLOG("ftruncate: %s\n", strerror(errno));
        return;
    }

    /* Map the object into the caller's address space. */

    status = static_cast<HagoromoStatus *>(mmap(nullptr, sizeof(*status), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (status == MAP_FAILED) {
        DLOG("mmap: %s\n", strerror(errno));
        return;
    }

    if (sem_init(&status->sem1, 1, 0) == -1) {
        DLOG("sem_init-sem1: %s\n", strerror(errno));
        return;
    }
}

void Provider::notifyUpdate() const {
    if (sem_post(&status->sem1) == -1) {
        DLOG("sem_post: %s\n", strerror(errno));
        return;
    }
}

// mounted
void Provider::FromMSC(QObject *msc) {
    if (msc == nullptr) {
        DLOG("msc is nullpo\n");
        return;
    }
    usbMounted = msc->property("unmountExported").toBool();
    DLOG("Mounted: %d\n", usbMounted);
}

void Provider::MSCSlot() { FromMSC(sender()); }

void Provider::UpdateEntryID() {
    if (sender() == nullptr) {
        DLOG("nullpo\n");
        return;
    }

    if (MusicPlayerDefaultModel == nullptr) {
        MusicPlayerDefaultModel = sender();
    }

    status->entryId = (int)sender()->property("entry_id").toInt(nullptr);
    // application is shutting down
    if (status->entryId == 0) {
        return;
    }

    if (status->prevEntryId == 0) {
        GetPlaylist();
        status->prevEntryId = status->entryId;
    }

    if (status->entryId != status->prevEntryId) {
        GetPlaylist();
        status->prevEntryId = status->entryId;
    }

    notifyUpdate();
}

void Provider::UpdateElapsed() {
    if (sender() == nullptr) {
        DLOG("nullpo\n");
        return;
    }

    if (MusicPlayerDefaultModel == nullptr) {
        MusicPlayerDefaultModel = sender();
    }

    status->elapsed = (int)sender()->property("currently_playing_time").toInt(nullptr);

    notifyUpdate();
}

void Provider::UpdateBasicControls() {
    if (MusicPlayerDefaultModel == nullptr) {
        DLOG("nullpo\n");
        return;
    }

    status->shuffleOn = (int)MusicPlayerDefaultModel->property("is_shuffle").toInt(nullptr);
    status->repeatMode = (int)MusicPlayerDefaultModel->property("repeat_mode").toInt(nullptr);

    notifyUpdate();
}

void Provider::FromDAC(QObject *o) const {
    if (o == nullptr) {
        DLOG("nullpo\n");
        return;
    }

    auto p = o->property("volume");
    if (!p.isValid()) {
        DLOG("invalid\n");
        return;
    }
    status->volumeRaw = p.toInt(nullptr);
    status->volume = status->volumeRaw * 100 / maxVolume;

    DLOG("volume %d%%, raw %d\n", status->volume, status->volumeRaw);
    notifyUpdate();
}

void Provider::VolumeSlot() { FromDAC(sender()); }

int DurationToInt(const QString &duration) {
    auto parts = duration.split(":");
    if (parts.length() != 2) {
        DLOG("failed to parse duration %s\n", qPrintable(duration));
        return 0;
    }

    QStringRef mins(&parts[0], 0, parts[0].length());
    QStringRef secs(&parts[1], 0, parts[1].length());

    bool ok;
    int minutes = mins.toInt(&ok);
    if (!ok) {
        DLOG("failed to parse minutes: %s\n", mins.toUtf8().constData());
        return 0;
    }

    int seconds = secs.toInt(&ok);
    if (!ok) {
        DLOG("failed to parse seconds: %s\n", secs.toUtf8().constData());
        return 0;
    }

    return minutes * 60 + seconds;
}

int parseTrack(QQuickItem *trackItem, Track *track) {
    if (!trackItem) {
        DLOG("track is nullpo, why?\n");
        return -1;
    }

    if (trackItem->childItems().length() < 3) {
        DLOG("track item child items: %d, expected 3, skipping\n", trackItem->childItems().length());
        return -1;
    }

    auto contents = trackItem->childItems().at(1);

    auto indexN = contents->property("index_text");
    if (!indexN.isValid()) {
        DLOG("invalid track index\n");
        return -1;
    }

    // track number is missing, track is not filled in yet
    if (indexN.toString().length() == 0) {
        return 0;
    }

    bool ok = false;
    track->TrackNumber = indexN.toInt(&ok);
    if (!ok) {
        DLOG("cannot convert track number '%s' to int\n", indexN.toString().toUtf8().constData());
        return -1;
    }

    auto Artist = contents->property("additional_text");
    if (!Artist.isValid()) {
        DLOG("invalid artist\n");
        return -1;
    }
    strncpy(track->Artist, Artist.toString().toUtf8().constData(), sizeof track->Artist);

    auto Title = contents->property("item_text");
    if (!Title.isValid()) {
        DLOG("invalid title\n");
        return -1;
    }

    if (Title.toString().length() == 0) {
        return 0;
    }
    strncpy(track->Title, Title.toString().toUtf8().constData(), sizeof track->Title);

    auto IsPlaying = contents->property("is_play_status_visible");
    if (!IsPlaying.isValid()) {
        DLOG("invalid is_play_status_visible\n");
        return -1;
    }

    if (IsPlaying.toInt() == 1) {
        track->Active = true;
    }

    auto Duration = contents->property("duration_text");
    if (!Duration.isValid()) {
        DLOG("invalid duration\n");
        return -1;
    }

    auto dur = DurationToInt(Duration.toString());
    if (dur == 0) {
        return 0;
    }

    track->Duration = dur;
    DLOG(
        "track: %d. %s ::: %s ::: %d secs, active: %d\n",
        track->TrackNumber,
        qPrintable(track->Artist),
        qPrintable(track->Title),
        track->Duration,
        track->Active
    );

    return 0;
}

// playlist is built from trackSequenceView (swipe left from music player)
// at first launch this list might be empty or incomplete
// update view_index to recalculate contentY value, which forces sequence view to load actual data
void Provider::updatePlaylist(QObject *trackSequenceView) {
    auto indexV = trackSequenceView->property("view_index");
    if (!indexV.isValid()) {
        DLOG("missing index prop\n");
        return;
    }
    auto index = indexV.toInt(nullptr);

    //    DLOG("index %d\n", trackSequenceView->property("view_index").toInt(nullptr));
    trackSequenceView->setProperty("view_index", index + 1);
    //    DLOG("index %d\n", trackSequenceView->property("view_index").toInt(nullptr));
    trackSequenceView->setProperty("view_index", index);
    //    DLOG("index %d\n", trackSequenceView->property("view_index").toInt(nullptr));
}

void Provider::GetPlaylist() {
    if (gridArea == nullptr) {
        auto window = getWindow();
        if (!window) {
            DLOG("no window\n");
            return;
        }

        auto context = qmlContext(window);
        if (context == nullptr) {
            DLOG("no context\n");
            return;
        }

        auto res = jsExpr("gridArea", context, window);
        if (!res.isValid()) {
            DLOG("gridArea res invalid\n");
            return;
        }

        gridArea = qvariant_cast<QQuickItem *>(res);
        if (!gridArea) {
            DLOG("cannot cast gridArea\n");
            return;
        }
    }

    if (gridArea == nullptr) {
        DLOG("no grid area\n");
        return;
    }

    QQuickItem *TrackSequenceView = nullptr;
    for (auto l : gridArea->childItems()) {
        if (l == nullptr) {
            continue;
        }
        for (auto kid : l->childItems()) {
            if (QString(kid->metaObject()->className()).startsWith("TrackSequenceView_QMLTYPE_")) {
                if (QString(kid->objectName()) == "TrackSequence") {
                    TrackSequenceView = kid;
                    break;
                }
            }
        }
    }

    if (TrackSequenceView == nullptr) {
        DLOG("cannot find TrackSequenceView_QMLTYPE_\n");
        return;
    }

    updatePlaylist(TrackSequenceView);

    QQuickItem *listView = nullptr;
    if (TrackSequenceView == nullptr) {
        return;
    }
    for (auto v : TrackSequenceView->childItems()) {
        if (QString(v->metaObject()->className()).startsWith("EdgeDetectListView_QMLTYPE_")) {
            listView = v;
            break;
        }
    }

    if (listView == nullptr) {
        DLOG("cannot get listView in track sequence\n");
        return;
    }

    if (listView->childItems().length() < 1) {
        DLOG("list view has no children\n");
        return;
    }

    auto vv = listView->childItems().at(0);
    if (vv == nullptr) {
        return;
    }
    auto tracks = vv->childItems();
    DLOG("%d tracks in track list\n", tracks.length());

    for (int g = 0; g < PLAYLIST_LENGTH; g++) {
        status->playlist[g].TrackNumber = 0;
        memset(status->playlist[g].Title, 0, PLAYLIST_TRACK_FIELD_SIZE);
        memset(status->playlist[g].Artist, 0, PLAYLIST_TRACK_FIELD_SIZE);
        status->playlist[g].Duration = 0;
        status->playlist[g].Active = false;
    }

    int i = 0;
    for (auto trackItem : tracks) {
        if (i >= PLAYLIST_LENGTH) {
            DLOG("over playlist length\n");
            break;
        }

        if (parseTrack(trackItem, &status->playlist[i]) == 0) {
            i++;
        }
    }
}

void Provider::CurrentPlayState(PlayState play_state) const {
    DLOG("new state is %d\n", play_state);
    status->playState = static_cast<PlayStateE>(play_state);
    notifyUpdate();
}
