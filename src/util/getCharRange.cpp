#include "getCharRange.h"

#include "dlog.h"
#include "sqlite/build-arm/install/include/sqlite3.h"
#include "unicode/unistr.h"
#include "wstring.h"

const char *query = "WITH RECURSIVE split_chars AS (\n"
                    "    SELECT\n"
                    "        SUBSTR(value, 1, 1) AS char,\n"
                    "        SUBSTR(value, 2) AS rest\n"
                    "    FROM artists\n"
                    "    UNION ALL\n"
                    "    SELECT\n"
                    "        SUBSTR(title, 1, 1) AS char,\n"
                    "        SUBSTR(title, 2) AS rest\n"
                    "    FROM object_body\n"
                    "    UNION ALL\n"
                    "    SELECT\n"
                    "        SUBSTR(rest, 1, 1) AS char,\n"
                    "        SUBSTR(rest, 2) AS rest\n"
                    "    FROM split_chars\n"
                    "    WHERE LENGTH(rest) > 0\n"
                    ")\n"
                    "SELECT DISTINCT char\n"
                    "FROM(\n"
                    "SELECT char FROM split_chars\n"
                    ")\n"
                    "WHERE char != ''\n"
                    "ORDER BY char";

static int callback(void *output, int argc, char **argv, char **notUsed) {
    auto res = (std::string *)output;
    res->append(argv[0]);
    return 0;
}

void getCharRange(std::vector<uint32_t> *points) {
    sqlite3 *db;
    char *zErrMsg = nullptr;
    int rc;
    DLOG("getting char range\n");
#ifdef DESKTOP
    auto path = "../MTPDB.dat";
#else
    auto path = "/db/MTPDB.dat";
#endif
    rc = sqlite3_open_v2(path, &db, SQLITE_OPEN_READONLY, nullptr);
    if (rc) {
        DLOG("Can't open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    std::string res;
    rc = sqlite3_exec(db, query, callback, &res, &zErrMsg);
    if (rc != SQLITE_OK) {
        DLOG("SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);

    DLOG("%d characters found\n", utfLen(res));

    size_t index = 0;
    size_t pointsize = 0;
    size_t index_before = 0;
    std::string resUpper;
    while (index < res.size()) {
        index_before = index;
        pointsize = 0;

        auto v = utfToPoint(res, index);
        points->push_back(v);
        pointsize = index - index_before;

        resUpper.clear();
        auto point = res.substr(index_before, pointsize);
        icu::UnicodeString(point.c_str()).toUpper().toUTF8String(resUpper);
        if (point != resUpper) {
            //            printf("%s -> %s\n", point.c_str(), resUpper.c_str());
            size_t i = 0;
            points->push_back(utfToPoint(resUpper, i));
        }
    }
}