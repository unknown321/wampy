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
#ifdef DESKTOP
    return;
#endif
    if (sem_post(&s->sem1) == -1)
        errExit("sem_post");

    /* Wait until peer says that it has finished accessing
       the shared memory. */

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

    rc = sqlite3_bind_int(select_stmt, 2, s->vptOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 3, s->vptMode);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 4, s->clearPhaseOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 5, s->DNOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 6, s->dseeOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 7, s->eq6On);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 8, s->eq6Preset);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 9, s->eq6Bands[0]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 10, s->eq6Bands[1]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 11, s->eq6Bands[2]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 12, s->eq6Bands[3]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 13, s->eq6Bands[4]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 14, s->eq6Bands[5]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 14, s->eq6Bands[5]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 15, s->eq10On);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 16, s->eq10Preset);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 17, s->eq10Bands[0]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }

    rc = sqlite3_bind_int(select_stmt, 18, s->eq10Bands[1]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 19, s->eq10Bands[2]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 20, s->eq10Bands[3]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 21, s->eq10Bands[4]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 22, s->eq10Bands[5]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 23, s->eq10Bands[6]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 24, s->eq10Bands[7]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 25, s->eq10Bands[8]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 26, s->eq10Bands[9]);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 27, s->toneControlOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 28, s->toneControlLow);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 29, s->toneControlMid);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 30, s->toneControlHigh);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 31, s->toneControlLowFreq);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 32, s->toneControlMidFreq);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 33, s->toneControlMidFreq);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 34, s->eqUse);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 35, s->dcLinearOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 36, s->dcLinearFilter);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 37, s->clearAudioOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 38, s->directSourceOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 39, s->dseeHXOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 40, s->vinylOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 41, s->vinylType);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 42, s->dseeCustOn);
    if (SQLITE_OK != rc) {
        fprintf(stderr, "Error binding value in select (%i): %s\n", rc, sqlite3_errmsg(db));
        sqlite3_finalize(select_stmt);
        sqlite3_close(db);
        return -1;
    }
    rc = sqlite3_bind_int(select_stmt, 43, s->dseeCustMode);
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
        dbValues->vptOn = sqlite3_column_int(select_stmt, 0);
        dbValues->vptMode = sqlite3_column_int(select_stmt, 1);
        dbValues->clearPhaseOn = sqlite3_column_int(select_stmt, 2);
        dbValues->DNOn = sqlite3_column_int(select_stmt, 3);
        dbValues->dseeOn = sqlite3_column_int(select_stmt, 4);
        dbValues->eq6On = sqlite3_column_int(select_stmt, 5);
        dbValues->eq6Preset = sqlite3_column_int(select_stmt, 6);
        dbValues->eq6Bands[0] = sqlite3_column_int(select_stmt, 7);
        dbValues->eq6Bands[1] = sqlite3_column_int(select_stmt, 8);
        dbValues->eq6Bands[2] = sqlite3_column_int(select_stmt, 9);
        dbValues->eq6Bands[3] = sqlite3_column_int(select_stmt, 10);
        dbValues->eq6Bands[4] = sqlite3_column_int(select_stmt, 11);
        dbValues->eq6Bands[5] = sqlite3_column_int(select_stmt, 12);
        dbValues->eq10On = sqlite3_column_int(select_stmt, 13);
        dbValues->eq10Preset = sqlite3_column_int(select_stmt, 14);
        dbValues->eq10Bands[0] = sqlite3_column_int(select_stmt, 15);
        dbValues->eq10Bands[1] = sqlite3_column_int(select_stmt, 16);
        dbValues->eq10Bands[2] = sqlite3_column_int(select_stmt, 17);
        dbValues->eq10Bands[3] = sqlite3_column_int(select_stmt, 18);
        dbValues->eq10Bands[4] = sqlite3_column_int(select_stmt, 19);
        dbValues->eq10Bands[5] = sqlite3_column_int(select_stmt, 20);
        dbValues->eq10Bands[6] = sqlite3_column_int(select_stmt, 21);
        dbValues->eq10Bands[7] = sqlite3_column_int(select_stmt, 22);
        dbValues->eq10Bands[8] = sqlite3_column_int(select_stmt, 23);
        dbValues->eq10Bands[9] = sqlite3_column_int(select_stmt, 24);
        dbValues->toneControlOn = sqlite3_column_int(select_stmt, 25);
        dbValues->toneControlLow = sqlite3_column_int(select_stmt, 26);
        dbValues->toneControlMid = sqlite3_column_int(select_stmt, 27);
        dbValues->toneControlHigh = sqlite3_column_int(select_stmt, 28);
        dbValues->toneControlLowFreq = sqlite3_column_int(select_stmt, 29);
        dbValues->toneControlMidFreq = sqlite3_column_int(select_stmt, 30);
        dbValues->toneControlHighFreq = sqlite3_column_int(select_stmt, 31);
        dbValues->eqUse = sqlite3_column_int(select_stmt, 32);
        dbValues->dcLinearOn = sqlite3_column_int(select_stmt, 33);
        dbValues->dcLinearFilter = sqlite3_column_int(select_stmt, 34);
        dbValues->clearAudioOn = sqlite3_column_int(select_stmt, 35);
        dbValues->directSourceOn = sqlite3_column_int(select_stmt, 36);
        dbValues->dseeHXOn = sqlite3_column_int(select_stmt, 37);
        dbValues->vinylOn = sqlite3_column_int(select_stmt, 38);
        dbValues->vinylType = sqlite3_column_int(select_stmt, 39);
        dbValues->dseeCustOn = sqlite3_column_int(select_stmt, 40);
        dbValues->dseeCustMode = sqlite3_column_int(select_stmt, 41);
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
