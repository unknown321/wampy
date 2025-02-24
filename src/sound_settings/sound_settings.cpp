#include "sound_settings.h"
#include "../util/util.h"
#include "sqlite3.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

sqlite3 *SoundSettings::open() {
    sqlite3 *db;
    auto rc = sqlite3_open_v2(WAMPY_SOUND_SETTINGS_DATABASE, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
    if (rc) {
        DLOG("Can't open database %s: %s\n", WAMPY_SOUND_SETTINGS_DATABASE, sqlite3_errmsg(db));
        sqlite3_close(db);
        return nullptr;
    }

    return db;
}

void SoundSettings::Start() {
    auto db = createSchemaIfNeeded();
    if (db == nullptr) {
        DLOG("cannot create schema\n");
    }

#ifdef DESKTOP
    s = new sound_settings();
    s->fmStatus.state = 2;
    sqlite3_close(db);
    return;
#endif
    auto fullpath = std::string("/dev/shm") + SHMPATH;
    while (!exists(fullpath)) {
        DLOG("waiting for %s\n", fullpath.c_str());
        sleep(1);
    }

    int fd;

    fd = shm_open(SHMPATH, O_RDWR, 0);
    if (fd == -1)
        errExit("shm_open");

    s = static_cast<sound_settings *>(mmap(nullptr, sizeof(sound_settings), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (s == MAP_FAILED)
        errExit("mmap");

    Update();

    if (count(db, "default") == 0) {
        Save("default");
    }
    sqlite3_close(db);
    DLOG("sound settings started\n");
}

void SoundSettings::Update() const {
    DLOG("\n");
    s->command.id = EPstServerCommand::PSC_UPDATE;
    Send();
}

void SoundSettings::Send() const {
#ifdef DESKTOP
    return;
#endif
    if (sem_post(&s->sem1) == -1)
        errExit("sem_post");

    if (sem_wait(&s->sem2) == -1)
        errExit("sem_wait");
}

bool SoundSettings::Exists(const std::string &filename) {
    auto db = open();
    if (db == nullptr) {
        DLOG("no database\n");
        return false;
    }

    auto res = count(db, filename) > 0;
    sqlite3_close(db);
    return res;
}

sqlite3 *SoundSettings::createSchemaIfNeeded() {
    auto db = open();
    if (db == nullptr) {
        DLOG("cannot open sound settings database\n");
        return nullptr;
    }

    auto query = "SELECT name FROM sqlite_master WHERE type='table' AND name='eq';";
    sqlite3_stmt *select_stmt = nullptr;
    auto rc = sqlite3_prepare_v2(db, query, -1, &select_stmt, nullptr);
    if (rc) {
        DLOG("Can't prepare select statement %s (%i): %s\n", query, rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return nullptr;
    }

    int found = 0;
    while (SQLITE_ROW == (rc = sqlite3_step(select_stmt))) {
        found++;
    }

    DLOG("tables found: %d\n", found);

    if (found == 0) {
        query = "CREATE TABLE \"eq\" (\n"
                "\t\"filename\"\tTEXT NOT NULL UNIQUE COLLATE BINARY,\n"
                "\t\"vptOn\"\tINTEGER NOT NULL,\n"
                "\t\"vptMode\"\tINTEGER NOT NULL,\n"
                "\t\"clearPhaseOn\"\tINTEGER NOT NULL,\n"
                "\t\"DNOn\"\tINTEGER NOT NULL,\n"
                "\t\"dseeOn\"\tINTEGER NOT NULL,\n"
                "\t\"eq6On\"\tTEXT NOT NULL,\n"
                "\t\"eq6Preset\"\tINTEGER NOT NULL,\n"
                "\t\"eq6Band0\"\tINTEGER NOT NULL,\n"
                "\t\"eq6Band1\"\tINTEGER NOT NULL,\n"
                "\t\"eq6Band2\"\tINTEGER NOT NULL,\n"
                "\t\"eq6Band3\"\tINTEGER NOT NULL,\n"
                "\t\"eq6Band4\"\tINTEGER NOT NULL,\n"
                "\t\"eq6Band5\"\tINTEGER NOT NULL,\n"
                "\t\"eq10On\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Preset\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Band0\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Band1\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Band2\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Band3\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Band4\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Band5\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Band6\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Band7\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Band8\"\tINTEGER NOT NULL,\n"
                "\t\"eq10Band9\"\tINTEGER NOT NULL,\n"
                "\t\"toneControlOn\"\tINTEGER NOT NULL,\n"
                "\t\"toneControlLow\"\tINTEGER NOT NULL,\n"
                "\t\"toneControlMid\"\tINTEGER NOT NULL,\n"
                "\t\"toneControlHigh\"\tINTEGER NOT NULL,\n"
                "\t\"toneControlLowFreq\"\tINTEGER NOT NULL,\n"
                "\t\"toneControlMidFreq\"\tINTEGER NOT NULL,\n"
                "\t\"toneControlHighFreq\"\tINTEGER NOT NULL,\n"
                "\t\"eqUse\"\tINTEGER NOT NULL,\n"
                "\t\"dcLinearOn\"\tINTEGER NOT NULL,\n"
                "\t\"dcLinearFilter\"\tINTEGER NOT NULL,\n"
                "\t\"clearAudioOn\"\tINTEGER NOT NULL,\n"
                "\t\"directSourceOn\"\tINTEGER NOT NULL,\n"
                "\t\"dseeHXOn\"\tINTEGER NOT NULL,\n"
                "\t\"vinylOn\"\tINTEGER NOT NULL,\n"
                "\t\"vinylType\"\tINTEGER NOT NULL,\n"
                "\t\"dseeCustOn\"\tINTEGER NOT NULL,\n"
                "\t\"dseeCustMode\"\tINTEGER NOT NULL,\n"
                "\tPRIMARY KEY(\"filename\")\n"
                ");";

        rc = sqlite3_prepare_v2(db, query, -1, &select_stmt, nullptr);
        if (rc) {
            DLOG("Can't prepare create statement %s (%i): %s\n", query, rc, sqlite3_errmsg(db));
            sqlite3_finalize(select_stmt);
            sqlite3_close(db);
            return nullptr;
        }

        rc = sqlite3_step(select_stmt);
        if (rc != SQLITE_DONE) {
            DLOG("Can't create table %s (%i): %s\n", query, rc, sqlite3_errmsg(db));
            sqlite3_finalize(select_stmt);
            sqlite3_close(db);
            return nullptr;
        }
    }

    sqlite3_finalize(select_stmt);

    return db;
}

int SoundSettings::Save(const std::string &filename) const {
    auto db = createSchemaIfNeeded();
    if (db == nullptr) {
        DLOG("cannot save %s\n");
        return -1;
    }

    auto query =
        "INSERT OR REPLACE into "
        "eq(\"filename\",\"vptOn\",\"vptMode\",\"clearPhaseOn\",\"DNOn\",\"dseeOn\",\"eq6On\",\"eq6Preset\",\"eq6Band0\",\"eq6Band1\","
        "\"eq6Band2\",\"eq6Band3\",\"eq6Band4\",\"eq6Band5\",\"eq10On\",\"eq10Preset\",\"eq10Band0\",\"eq10Band1\",\"eq10Band2\","
        "\"eq10Band3\",\"eq10Band4\",\"eq10Band5\",\"eq10Band6\",\"eq10Band7\",\"eq10Band8\",\"eq10Band9\",\"toneControlOn\","
        "\"toneControlLow\",\"toneControlMid\",\"toneControlHigh\",\"toneControlLowFreq\",\"toneControlMidFreq\",\"toneControlHighFreq\","
        "\"eqUse\",\"dcLinearOn\",\"dcLinearFilter\",\"clearAudioOn\",\"directSourceOn\",\"dseeHXOn\",\"vinylOn\",\"vinylType\","
        "\"dseeCustOn\", \"dseeCustMode\") "
        "VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt *select_stmt = nullptr;
    auto rc = sqlite3_prepare_v2(db, query, -1, &select_stmt, nullptr);
    if (rc) {
        DLOG("Can't prepare select statement %s (%i): %s\n", query, rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_text(select_stmt, 1, filename.c_str(), (int)filename.size(), SQLITE_TRANSIENT);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 2, s->status.vptOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 3, s->status.vptMode);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 4, s->status.clearPhaseOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 5, s->status.DNOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 6, s->status.dseeOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 7, s->status.eq6On);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 8, s->status.eq6Preset);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 9, s->status.eq6Bands[0]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 10, s->status.eq6Bands[1]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 11, s->status.eq6Bands[2]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 12, s->status.eq6Bands[3]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 13, s->status.eq6Bands[4]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 14, s->status.eq6Bands[5]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 14, s->status.eq6Bands[5]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 15, s->status.eq10On);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 16, s->status.eq10Preset);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 17, s->status.eq10Bands[0]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 18, s->status.eq10Bands[1]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 19, s->status.eq10Bands[2]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 20, s->status.eq10Bands[3]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 21, s->status.eq10Bands[4]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 22, s->status.eq10Bands[5]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 23, s->status.eq10Bands[6]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 24, s->status.eq10Bands[7]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 25, s->status.eq10Bands[8]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 26, s->status.eq10Bands[9]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 27, s->status.toneControlOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 28, s->status.toneControlLow);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 29, s->status.toneControlMid);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 30, s->status.toneControlHigh);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 31, s->status.toneControlLowFreq);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 32, s->status.toneControlMidFreq);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 33, s->status.toneControlMidFreq);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 34, s->status.eqUse);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 35, s->status.dcLinearOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 36, s->status.dcLinearFilter);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 37, s->status.clearAudioOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 38, s->status.directSourceOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 39, s->status.dseeHXOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 40, s->status.vinylOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 41, s->status.vinylType);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 42, s->status.dseeCustOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 43, s->status.dseeCustMode);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_step(select_stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error inserting (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    sqlite3_finalize(select_stmt);
    sqlite3_close(db);

    DLOG("saved %s\n", filename.c_str());
    return 0;
}

int SoundSettings::count(sqlite3 *db, const std::string &filename) {
    if (db == nullptr) {
        DLOG("db is nullpo\n");
        return -1;
    }
    DLOG("looking for %s\n", filename.c_str());

    sqlite3_stmt *select_stmt = nullptr;
    auto query = "select count(filename) from eq where filename = ?;";
    auto rc = sqlite3_prepare_v2(db, query, -1, &select_stmt, nullptr);
    if (rc) {
        DLOG("Can't prepare select statement %s (%i): %s\n", query, rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        return -1;
    }

    rc = sqlite3_bind_text(select_stmt, 1, filename.c_str(), (int)filename.size(), SQLITE_TRANSIENT);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        return -1;
    }

    int res = 0;
    while (SQLITE_ROW == (rc = sqlite3_step(select_stmt))) {
        res = sqlite3_column_int(select_stmt, 0);
    }

    DLOG("found %d rows\n", res);
    sqlite3_finalize(select_stmt);

    return res;
}

int SoundSettings::Remove(const std::string &filename) {
    auto db = open();
    if (db == nullptr) {
        DLOG("cannot open db\n");
        return -1;
    }

    auto query = "DELETE from eq where filename = ?;";
    sqlite3_stmt *select_stmt = nullptr;
    auto rc = sqlite3_prepare_v2(db, query, -1, &select_stmt, nullptr);
    if (rc) {
        DLOG("Can't prepare delete statement %s (%i): %s\n", query, rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_text(select_stmt, 1, filename.c_str(), (int)filename.size(), SQLITE_TRANSIENT);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in delete (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_step(select_stmt);
    if (rc != SQLITE_DONE) {
        DLOG("Can't delete %s (%i): %s\n", query, rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    sqlite3_finalize(select_stmt);
    sqlite3_close(db);

    DLOG("deleted %s\n", filename.c_str());
    return 0;
}

int SoundSettings::GetDir(const std::string &filename, sound_settings *dbValues) {
    auto parts = split(filename, "/");
    parts.erase(parts.end());
    auto fName = join(parts, 0, "/");
    DLOG("new dir is %s\n", fName.c_str());
    return Get(fName, dbValues);
}

int SoundSettings::Get(const std::string &filename, sound_settings *dbValues) {
    if (dbValues == nullptr) {
        DLOG("dbvalues is nullpo\n");
        return -1;
    }

    auto db = open();
    if (db == nullptr) {
        DLOG("db is nullpo\n");
        return -1;
    }

    sqlite3_stmt *select_stmt = nullptr;
    auto query = "select "
                 "vptOn,vptMode,clearPhaseOn,DNOn,dseeOn,eq6On,eq6Preset,eq6Band0,eq6Band1,eq6Band2,eq6Band3,eq6Band4,eq6Band5,"
                 "eq10On,eq10Preset,eq10Band0,eq10Band1,eq10Band2,eq10Band3,eq10Band4,eq10Band5,eq10Band6,eq10Band7,eq10Band8,eq10Band9,"
                 "toneControlOn,toneControlLow,toneControlMid,toneControlHigh,toneControlLowFreq,toneControlMidFreq,toneControlHighFreq,"
                 "eqUse,dcLinearOn,dcLinearFilter,clearAudioOn,directSourceOn,dseeHXOn,vinylOn,vinylType,dseeCustOn,dseeCustMode from eq "
                 "where filename = ?;";
    auto rc = sqlite3_prepare_v2(db, query, -1, &select_stmt, nullptr);
    if (rc) {
        DLOG("Can't prepare select statement %s (%i): %s\n", query, rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_text(select_stmt, 1, filename.c_str(), (int)filename.length(), SQLITE_TRANSIENT);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    int rows = 0;
    while (SQLITE_ROW == (rc = sqlite3_step(select_stmt))) {
        rows++;
        dbValues->status.vptOn = sqlite3_column_int(select_stmt, 0);
        dbValues->status.vptMode = sqlite3_column_int(select_stmt, 1);
        dbValues->status.clearPhaseOn = sqlite3_column_int(select_stmt, 2);
        dbValues->status.DNOn = sqlite3_column_int(select_stmt, 3);
        dbValues->status.dseeOn = sqlite3_column_int(select_stmt, 4);
        dbValues->status.eq6On = sqlite3_column_int(select_stmt, 5);
        dbValues->status.eq6Preset = sqlite3_column_int(select_stmt, 6);
        dbValues->status.eq6Bands[0] = sqlite3_column_int(select_stmt, 7);
        dbValues->status.eq6Bands[1] = sqlite3_column_int(select_stmt, 8);
        dbValues->status.eq6Bands[2] = sqlite3_column_int(select_stmt, 9);
        dbValues->status.eq6Bands[3] = sqlite3_column_int(select_stmt, 10);
        dbValues->status.eq6Bands[4] = sqlite3_column_int(select_stmt, 11);
        dbValues->status.eq6Bands[5] = sqlite3_column_int(select_stmt, 12);
        dbValues->status.eq10On = sqlite3_column_int(select_stmt, 13);
        dbValues->status.eq10Preset = sqlite3_column_int(select_stmt, 14);
        dbValues->status.eq10Bands[0] = sqlite3_column_int(select_stmt, 15);
        dbValues->status.eq10Bands[1] = sqlite3_column_int(select_stmt, 16);
        dbValues->status.eq10Bands[2] = sqlite3_column_int(select_stmt, 17);
        dbValues->status.eq10Bands[3] = sqlite3_column_int(select_stmt, 18);
        dbValues->status.eq10Bands[4] = sqlite3_column_int(select_stmt, 19);
        dbValues->status.eq10Bands[5] = sqlite3_column_int(select_stmt, 20);
        dbValues->status.eq10Bands[6] = sqlite3_column_int(select_stmt, 21);
        dbValues->status.eq10Bands[7] = sqlite3_column_int(select_stmt, 22);
        dbValues->status.eq10Bands[8] = sqlite3_column_int(select_stmt, 23);
        dbValues->status.eq10Bands[9] = sqlite3_column_int(select_stmt, 24);
        dbValues->status.toneControlOn = sqlite3_column_int(select_stmt, 25);
        dbValues->status.toneControlLow = sqlite3_column_int(select_stmt, 26);
        dbValues->status.toneControlMid = sqlite3_column_int(select_stmt, 27);
        dbValues->status.toneControlHigh = sqlite3_column_int(select_stmt, 28);
        dbValues->status.toneControlLowFreq = sqlite3_column_int(select_stmt, 29);
        dbValues->status.toneControlMidFreq = sqlite3_column_int(select_stmt, 30);
        dbValues->status.toneControlHighFreq = sqlite3_column_int(select_stmt, 31);
        dbValues->status.eqUse = sqlite3_column_int(select_stmt, 32);
        dbValues->status.dcLinearOn = sqlite3_column_int(select_stmt, 33);
        dbValues->status.dcLinearFilter = sqlite3_column_int(select_stmt, 34);
        dbValues->status.clearAudioOn = sqlite3_column_int(select_stmt, 35);
        dbValues->status.directSourceOn = sqlite3_column_int(select_stmt, 36);
        dbValues->status.dseeHXOn = sqlite3_column_int(select_stmt, 37);
        dbValues->status.vinylOn = sqlite3_column_int(select_stmt, 38);
        dbValues->status.vinylType = sqlite3_column_int(select_stmt, 39);
        dbValues->status.dseeCustOn = sqlite3_column_int(select_stmt, 40);
        dbValues->status.dseeCustMode = sqlite3_column_int(select_stmt, 41);
    }

    DLOG("found %d rows for %s\n", rows, filename.c_str());

    sqlite3_finalize(select_stmt);
    sqlite3_close(db);
    return 0;
}

bool SoundSettings::ExistsDir(const std::string &filename) {
    auto db = open();
    if (db == nullptr) {
        DLOG("no database\n");
        return false;
    }

    auto parts = split(filename, "/");
    parts.erase(parts.end());
    auto res = count(db, join(parts, 0, "/")) > 0;
    sqlite3_close(db);
    return res;
}

int SoundSettings::RemoveDir(const std::string &filename) {
    auto parts = split(filename, "/");
    parts.erase(parts.end());

    return Remove(join(parts, 0, "/"));
}

int SoundSettings::SaveDir(const std::string &filename) const {
    auto parts = split(filename, "/");
    parts.erase(parts.end());

    return Save(join(parts, 0, "/"));
}

void SoundSettings::SetFM(int v) const {
    s->command.id = PSC_SET_FM;
    s->command.valueInt = v;
    Send();
}

void SoundSettings::SetFMFreq(int v) const {
    s->command.id = PSC_SET_FM_FREQ;
    s->command.valueInt = v;
    Send();
}

void SoundSettings::SetFMStereo(bool v) const {
    s->command.id = PSC_SET_FM_STEREO;
    s->command.valueInt = (int)v;
    Send();
}

void SoundSettings::RefreshAnalyzerPeaks(float sensitivity) {
    if (sharkCalls > 0) {
        DoShark();
        return;
    }
#ifdef DESKTOP
    for (int i = 0; i < HAGOROMO_AUDIO_PEAKS_COUNT; i++) {
        auto v = std::rand() % 100;
        s->peaks[i] = v;
    }

    for (int i = 0; i < HAGOROMO_AUDIO_PEAKS_COUNT; i++) {
        peaks[i] = s->peaks[i];
    }
#else
    bool empty = true;
    for (auto v : s->peaks) {
        if (v != 0) {
            empty = false;
            break;
        }
    }

    // attempt to avoid flicker on resume (doesn't help that much, but still reduced)
    if (empty) {
        return;
    }

    for (int i = 0; i < HAGOROMO_AUDIO_PEAKS_COUNT; i++) {
        auto v = s->peaks[i];
        auto q = 10 * std::log10((double)v / (double)2147483647);
        auto o = q * 2;
        if (q < -81) {
            peaks[i] = 0;
            continue;
        }

        auto pretty = 100 + (o * 150 / 100);
        auto discrete = pretty - (int(pretty) % 5);
        peaks[i] = 100 * std::atan(sensitivity * discrete) / std::atan(sensitivity * 100);
    }
#endif
}

void SoundSettings::DoShark() {
    sharkCalls--;

    int total_steps = SHARK_CALLS - sharkCalls - 1;                                // Calls completed
    int group = total_steps / SHARK_PEAK_DELAY;                                    // Group of 4 calls
    int lap = total_steps / (WAMPY_AUDIO_SPECTRUM_PEAKS_COUNT * SHARK_PEAK_DELAY); // 0 or 1

    int base_index = group % WAMPY_AUDIO_SPECTRUM_PEAKS_COUNT;

    int index;
    if (lap == 0) {
        index = base_index;
    } else {
        index = (WAMPY_AUDIO_SPECTRUM_PEAKS_COUNT - 1) - base_index;
    }

    for (int i = 0; i < WAMPY_AUDIO_SPECTRUM_PEAKS_COUNT; i++) {
        if (i == index) {
            peaks[i] = SHARK_PEAK_VALUE;
        } else {
            peaks[i] = 0;
        }
    }
}

void SoundSettings::StartShark() { sharkCalls = SHARK_CALLS; }

void SoundSettings::SetAnalyzerBandsWinamp() const {
    // winamp does 76 measurements with same step of 150  = 11400 * 2 channels = 22800
    // hagoromo does only 12 for spectrum:
    // 50, 100, 160, 250, 500, 750, 1000, 2000, 4000, 8000, 16000, 28000
    // winamp mode makes steps even and reduces width in half
    int step = 28000 / 2 / 12;

    int mean = MEAN_REGULAR;
    for (int i = 0; i < HAGOROMO_AUDIO_PEAKS_COUNT * 2; i = i + 2) {
        s->command.valuesInt[i] = step * (i / 2 + 1);
        s->command.valuesInt[i + 1] = mean;
    }

    s->command.valueInt = HAGOROMO_AUDIO_PEAKS_COUNT;
    s->command.id = PSC_SET_AUDIO_ANALYZER_BANDS;
    Send();
}

void SoundSettings::SetAnalyzerBandsOrig() const {
    std::vector<int> vals = {50, 100, 160, 250, 500, 750, 1000, 2000, 4000, 8000, 16000, 28000};
    int mean = MEAN_REGULAR;
    for (int i = 0; i < HAGOROMO_AUDIO_PEAKS_COUNT * 2; i = i + 2) {
        s->command.valuesInt[i] = vals.at(i / 2);
        if (i / 2 == 11) {
            mean = MEAN_HR;
        }
        s->command.valuesInt[i + 1] = mean;
    }

    s->command.valueInt = HAGOROMO_AUDIO_PEAKS_COUNT;
    s->command.id = PSC_SET_AUDIO_ANALYZER_BANDS;
    Send();
}

void SoundSettings::SetAnalyzer(int v) const {
    s->command.id = PSC_SET_AUDIO_ANALYZER;
    s->command.valueInt = v;
    Send();
}
