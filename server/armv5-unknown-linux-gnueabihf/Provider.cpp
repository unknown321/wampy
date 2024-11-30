#include "Provider.moc"
#include "moc_Provider.cpp"

#include "helpers.h"

#include <QJSValue>
#include <QQmlExpression>
#include <QQuickItem>

enum detailIndex {
    TITLE = 0,
    ARTIST,
    ALBUM_ARTIST,
    ALBUM,
    GENRE,
    YEAR,
    COMPOSER,
    TRACK,
    DISC,
    LENGTH,
    SENSME_STATUS,
    STORAGE_LOCATION,
    FILE_NAME,
    CODEC,
    BIT_RATE,
    SAMPLE_RATE,
    BIT_DEPTH,
    LYRICS_DATA,
};

Provider::Provider() {}

// mounted
void Provider::FromMSC(QObject *msc) {
    usbMounted = msc->property("unmountExported").toBool();
    DLOG("Mounted: %d\n", usbMounted);
}

void Provider::MSCSlot() { FromMSC(sender()); }

// curTime, time, artist, album, title
void Provider::FromMusicPlayer(QObject *o) {
    auto m = o->property("meta_data");
    auto map = m.toMap();
    album = map["album_name"].toString();
    artist = map["artist_name"].toString();
    title = map["play_title"].toString();
    hires = (int)o->property("is_high_resolution").toBool();
    DLOG("%s %s (%s), hires %d\n", artist.toUtf8().constData(), title.toUtf8().constData(), album.toUtf8().constData(), hires);

    time = (int)o->property("total_playback_time").toDouble(nullptr);
    curTime = (int)o->property("currently_playing_time").toDouble(nullptr);
    DLOG("%d / %d\n", curTime, time);
    GetPlaylist();
}

void Provider::MusicPlayerSlot() { FromMusicPlayer(sender()); }

void Provider::UpdateElapsed() {
    bool ok;
    curTime = (int)sender()->property("currently_playing_time").toDouble(&ok);
}

void Provider::FromDAC(QObject *o) {
    bool ok;
    volume = o->property("volume").toInt(&ok);
    //    DLOG("volume %d\n", volume);
}

void Provider::VolumeSlot() { FromDAC(sender()); }

// playState, repeat, shuffle
void Provider::FromMusicWindow(QObject *o) {
    auto m = o->property("basicPlayerControls");
    if (!m.isValid()) {
        DLOG("failed to get basicPlayerControls\n");
        return;
    }

    auto map = m.toMap();
    auto keys = {"playState", "repeatModeState", "shuffleState"};
    for (auto k : keys) {
        if (map.count(k) == 0) {
            DLOG("missing key %s\n", k);
            return;
        }
    }

    isPlaying = map["playState"].toBool();
    shuffle = map["shuffleState"].toBool();
    bool ok;
    repeat = map["repeatModeState"].toInt(&ok);

    DLOG("is playing %d, shuffle %d, repeat %d\n", isPlaying, shuffle, repeat);
}

void Provider::MusicWindowSlot() { FromMusicWindow(sender()); }

// TODO: popupParent is already present in Connector
void Provider::FromDetailsPopup() {
    auto window = getWindow();
    if (!window) {
        DLOG("no window\n");
        return;
    }

    auto context = qmlContext(window);
    if (!context) {
        DLOG("no context\n");
        return;
    }

    auto res = jsExpr("popupParent", context, window);
    if (!res.isValid()) {
        DLOG("popup parent invalid\n");
        return;
    }

    auto popupParent = qvariant_cast<QQuickItem *>(res);
    if (!popupParent) {
        DLOG("cast failed\n");
        return;
    }

    QQuickItem *popupView = nullptr;
    for (auto c : popupParent->childItems()) {
        auto prop = c->property("popupID");
        if (!prop.isValid()) {
            continue;
        }

        if (prop.toString() == "NowPlayingContentDetailedInfoPopup") {
            popupView = c;
            break;
        }
    }

    if (!popupView) {
        DLOG("no details popup found\n");
        return;
    }

    if (popupView->childItems().count() < 1) {
        DLOG("missing popup data\n");
        return;
    }

    auto pop = popupView->childItems().at(0);
    if (pop->childItems().size() < 4) {
        DLOG("not enough children\n");
        return;
    }

    auto flickableDetailedInfo = pop->childItems().at(3);

    if (!flickableDetailedInfo) {
        DLOG("flickable is null\n");
        return;
    }

    auto detailedInfoDelegate = flickableDetailedInfo->childItems().at(0)->childItems().at(0);

    if (detailedInfoDelegate->childItems().size() < 18) {
        DLOG("unexpected list size %d\n", detailedInfoDelegate->childItems().size());
        return;
    }

    for (int i = 0; i < detailedInfoDelegate->childItems().size(); i++) {
        auto entry = detailedInfoDelegate->childItems().at(i);
        if (!entry) {
            DLOG("cast failed\n");
            continue;
        }

        parseDetailsPopupEntry(entry, i);
    }
}

void Provider::UpdateDetails() { FromDetailsPopup(); }

void Provider::parseDetailsPopupEntry(QQuickItem *o, int index) {
    if (o->childItems().size() < 2) {
        //        DLOG("unexpected element count %d\n", o->childItems().size());
        return;
    }

    auto v = o->childItems().at(1)->property("text");
    if (!v.isValid()) {
        DLOG("no text prop\n");
        return;
    }

    auto value = v.toString();

    QString sampleUnit = "";
    //    DLOG("entry %d\n", index);
    switch (index) {
    case CODEC:
        codec = value.split(" ").at(0);
        //            DLOG("codec %s\n", codec.toUtf8().constData());
        break;
    case BIT_DEPTH:
        bitDepth = value.split(" ").at(0).toInt();
        //            DLOG("bitdepth %d\n", bitDepth);
        break;
    case BIT_RATE:
        bitRate = value.split(" ").at(0).toInt();
        //            DLOG("bitrate %d\n", bitRate);
        break;
    case SAMPLE_RATE:
        sampleUnit = value.split(" ").at(1);
        sampleRate = value.split(" ").at(0).toFloat();
        if (sampleUnit == "MHz") {
            sampleRate = sampleRate * 1000;
        }
        //            DLOG("sample rate %d\n", sampleRate);
        break;
    default:
        break;
    }
}

int DurationToInt(QString duration) {
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
    track->Track = indexN.toInt(&ok);
    if (!ok) {
        DLOG("cannot convert track number '%s' to int\n", indexN.toString().toUtf8().constData());
        return -1;
    }

    auto Artist = contents->property("additional_text");
    if (!Artist.isValid()) {
        DLOG("invalid artist\n");
        return -1;
    }
    track->Artist = Artist.toString();

    auto Title = contents->property("item_text");
    if (!Title.isValid()) {
        DLOG("invalid title\n");
        return -1;
    }
    track->Title = Title.toString();

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

    track->Duration = DurationToInt(Duration.toString());
    DLOG(
        "track: %d. %s ::: %s ::: %d secs, active: %d\n",
        track->Track,
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

    QQuickItem *TrackSequenceView = nullptr;
    for (auto l : gridArea->childItems()) {
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

    auto tracks = listView->childItems().at(0)->childItems();
    DLOG("%d tracks in track list\n", tracks.length());

    for (int g = 0; g < PLAYLIST_LENGTH; g++) {
        playlist[g].Track = 0;
        playlist[g].Title = "";
        playlist[g].Artist = "";
        playlist[g].Duration = 0;
        playlist[g].Active = false;
    }

    playlistActiveFound = false;
    int i = 0;
    for (auto trackItem : tracks) {
        if (i >= PLAYLIST_LENGTH) {
            DLOG("over playlist length\n");
            break;
        }

        if (parseTrack(trackItem, &playlist[i]) == 0) {
            if (playlist[i].Active) {
                playlistActiveFound = true;
            }
            i++;
        }
    }
}