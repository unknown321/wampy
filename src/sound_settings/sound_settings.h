#ifndef WAMPY_SOUND_SETTINGS_H
#define WAMPY_SOUND_SETTINGS_H

#include "pshm_ucase.h"
#include "sqlite3.h"
#include "string"

#ifdef DESKTOP
#define WAMPY_SOUND_SETTINGS_DATABASE "../eqSettings.dat"
#else
#define WAMPY_SOUND_SETTINGS_DATABASE "/contents/wampy/eqSettings.dat"
#endif

class SoundSettings {
  public:
    sound_settings *s;

    void Start();
    void Update() const;
    int Save(const std::string &filename) const;
    int SaveDir(const std::string &filename) const;
    static bool Exists(const std::string &filename);
    static bool ExistsDir(const std::string &filename);
    static int Remove(const std::string &filename);
    static int RemoveDir(const std::string &filename);
    static int Get(const std::string &filename, sound_settings *dbValues);
    static int GetDir(const std::string &filename, sound_settings *dbValues);

  private:
    static sqlite3 *open();
    static sqlite3 *createSchemaIfNeeded();
    static int count(sqlite3 *db, const std::string &filename);
};

#endif // WAMPY_SOUND_SETTINGS_H
