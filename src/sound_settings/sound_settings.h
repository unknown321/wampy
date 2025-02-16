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

#define SHARK_DECAY_DELAY 24
#define SHARK_PEAK_VALUE 25
#define SHARK_PEAK_DELAY 4
#define SHARK_LAPS 1.5
#define SHARK_CALLS (WAMPY_AUDIO_SPECTRUM_PEAKS_COUNT * SHARK_PEAK_DELAY * SHARK_LAPS)
#define MEAN_REGULAR 456
#define MEAN_HR 406

class SoundSettings {
  public:
    sound_settings *s{};
    int peaks[WAMPY_AUDIO_SPECTRUM_PEAKS_COUNT]{0};
    int sharkCalls = 0;

    void Start();
    void Update() const;
    void Send() const;
    void SetFM(int) const;
    void SetFMFreq(int) const;
    void SetFMStereo(bool v) const;
    int Save(const std::string &filename) const;
    int SaveDir(const std::string &filename) const;
    static bool Exists(const std::string &filename);
    static bool ExistsDir(const std::string &filename);
    static int Remove(const std::string &filename);
    static int RemoveDir(const std::string &filename);
    static int Get(const std::string &filename, sound_settings *dbValues);
    static int GetDir(const std::string &filename, sound_settings *dbValues);
    void RefreshAnalyzerPeaks(float sensitivity = 0.03);
    void DoShark();
    void StartShark();
    void SetAnalyzerBandsWinamp() const;
    void SetAnalyzerBandsOrig() const;
    void SetAnalyzer(int v) const;

  private:
    static sqlite3 *open();
    static sqlite3 *createSchemaIfNeeded();
    static int count(sqlite3 *db, const std::string &filename);
};

#endif // WAMPY_SOUND_SETTINGS_H
