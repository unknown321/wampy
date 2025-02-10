#ifndef DMPCONFIG_DMPCLIENT_H
#define DMPCONFIG_DMPCLIENT_H

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace pst {
    namespace dmpfeature {
        class DmpFeature {
          public:
            DmpFeature();
            ~DmpFeature();
            //            bool IsBundledHp(pst::dmpfeature::DmpFeature::Hp const &);
            bool IsShipInvalid();
            bool IsModelInvalid();
            bool IsFeaturedAtrac();
            bool IsFeaturedVideo();
            bool IsFeaturedVolAvls();
            bool IsBundledHpInvalid();
            bool IsFeaturedVolAlert();
            void GetFeatureUsbPidMsc(bool &, std::string &);
            void GetFeatureUsbPidUac(bool &, std::string &);
            void GetFeatureUsbVidMsc(bool &, std::string &);
            void GetFeatureUsbVidUac(bool &, std::string &);
            bool IsFeaturedMqaRender();
            bool GetFeatureVolumeInit();
            bool IsFeaturedNoiseCancel();
            bool GetFeatureFixedUsingEq();
            bool IsFeaturedBtlGain2Step();
            bool IsFeaturedSourceDirect();
            bool GetNumOfExternalStorage();
            bool IsFeaturedDsdProcNative();
            bool IsFeaturedSeHpDsdNative();
            bool IsFeaturedSelectUsingEq();
            bool IsFeaturedVolStrategyEu();
            bool IsFeaturedAnalyzerTuning();
            bool IsFeaturedClearAudioPlus();
            bool IsFeaturedLpcmDsdRemaster();
            bool IsFeaturedWmportOutAnalog();
            bool IsFeaturedDsd256ProcNative();
            bool IsFeaturedUsbdacOutDsdNative();
            void GetDseeHxCustomHpDcfgFilename(std::string &);
            bool GetFeatureVolumeSeGainHighInit();
            bool GetFeatureVolumeBtlGainHighInit();
            bool GetFeatureVolumeSeGainNormalInit();
            bool GetFeatureVolumeBtlGainNormalInit();
            bool GetFeatureVolumeSeGainHighAvlsThresh();
            bool GetFeatureVolumeBtlGainHighAvlsThresh();
            bool GetFeatureVolumeSeGainNormalAvlsThresh();
            bool GetFeatureVolAlertIntervalShortageInMin();
            bool GetFeatureVolumeBtlGainNormalAvlsThresh();
            bool GetFeatureVolumeSeGainHighVolAlertThresh();
            bool GetFeatureVolumeBtlGainHighVolAlertThresh();
            bool GetFeatureVolumeSeGainNormalVolAlertThresh();
            bool GetFeatureVolumeBtlGainNormalVolAlertThresh();
            //            bool IsShip(pst::dmpfeature::DmpFeature::Ship const &);
            //            bool IsModel(pst::dmpfeature::DmpFeature::Model const &);
        };
    } // namespace dmpfeature

    /* namespace playservice {
         class IPlayEventListener {
           public:
             virtual void f1() { fprintf(stderr, "WOW: %s\n", __PRETTY_FUNCTION__); };
         };

         class PlayEventListener: public IPlayEventListener {
           public:
             virtual void f1() override { fprintf(stderr, "1WOW: %s\n", __PRETTY_FUNCTION__ );};
             //            char padding[0x100];
         };

         class PlayStatus {
           public:
             int i;
         };


         class TrackSequence{
           public:

             class Track{
               public:
                 Track(){};
                 ~Track(){};
                 unsigned int id;
                 std::string name;
             };

             TrackSequence() {};
             ~TrackSequence(){};

             Track *p1;
             int idx;
             unsigned int* p3; // 0x98
             unsigned int* p4; // == 0
             unsigned int* p5; // == p6
             unsigned int* p6; // == p5
             unsigned int track_id;
             unsigned int zero;
             TrackSequence::Track* track;
             unsigned int* p7;
             unsigned int p8;
             unsigned int* p9;
             unsigned int* p10;
             unsigned int* p11;
         };


         struct playstate_t {};
         struct media_origin_t{};
         class PrevTrackOption{};
         class PrevGroupOption{};
         struct dpc_mode_t{};
         class PlaySpeed{};
         struct change_track_mode_t{};
     }
     */
    namespace core {
        using myfunc = std::function<void()>;
        class Framework {
          public:
            static Framework *GetReference();
            void StartForApplication(myfunc, bool);
            void StartForDaemon(int, char const **, myfunc);
            void StartForApplicationTestOnly();
            void Stop();
            void Pump(bool);
            void UpdateCurrentContextHangTimeout(unsigned long long);
        };

        struct status_t {};
    } // namespace core

    namespace services {
        class ITunerPlayerService {
          public:
            class StereoMode {
              public:
                int value;
            };
        };
        class TunerPlayerServiceClient {
          public:
            void GetBandwidth(unsigned int &, unsigned int &, unsigned int &);
            void GetFrequency(unsigned int &);
            void GetName() const;
            void GetSoftBandwidth(unsigned int &, unsigned int &, unsigned int &);
            int GetTunerState();
            void IsRunningAutoTuning();
            void Play();
            void SetFrequency(unsigned int const &);
            void SetSoftBandwidth(unsigned int const &, unsigned int const &, unsigned int const &);
            void StartAutoTuning(unsigned int const &, bool const &, unsigned int const &);
            void Stop();
            void StopAutoTuning();
            void Open();
            void Close();
            void GetStereoMode(pst::services::ITunerPlayerService::StereoMode &);
            void SetStereoMode(pst::services::ITunerPlayerService::StereoMode const &);
            //            void AddListener(pst::services::ITunerPlayerService::IServiceListener *, std::__1::basic_string<char,
            //            std::__1::char_traits<char>, std::__1::allocator<char>> const &); void
            //            GetMuteMode(pst::services::ITunerPlayerService::MuteMode &); void
            //            GetSenseMode(pst::services::ITunerPlayerService::SenseMode &); void
            //            GetSignalLevel(pst::services::ITunerPlayerService::SignalLevel &); void
            //            GetStereoState(pst::services::ITunerPlayerService::StereoMode &); void
            //            ReadDeviceSettings(pst::services::binder::TransactionParam &, pst::services::ITunerPlayerService::DeviceSettings
            //            &); void ReadSoundSettings(pst::services::binder::TransactionParam &,
            //            pst::services::ITunerPlayerService::SoundSettings &); void
            //            RemoveListener(pst::services::ITunerPlayerService::IServiceListener *); void
            //            SetDeviceSettings(pst::services::ITunerPlayerService::DeviceSettings const &); void
            //            SetMuteMode(pst::services::ITunerPlayerService::MuteMode const &); void
            //            SetSenseMode(pst::services::ITunerPlayerService::SenseMode const &); void
            //            SetSoundSettings(pst::services::ITunerPlayerService::SoundSettings const &); void
            //            SizeOfDeviceSettings(pst::services::ITunerPlayerService::DeviceSettings const &); void
            //            SizeOfSoundSettings(pst::services::ITunerPlayerService::SoundSettings const &); void
            //            WriteDeviceSettings(pst::services::ITunerPlayerService::DeviceSettings const &,
            //            pst::services::binder::TransactionParam &); void
            //            WriteSoundSettings(pst::services::ITunerPlayerService::SoundSettings const &,
            //            pst::services::binder::TransactionParam &);
        };

        class TunerPlayerServiceClientFactory {
          public:
            TunerPlayerServiceClient *CreateInstance();
        };

        class SoundServiceFwClient {
          public:
        };
        namespace sound {
            class mobile {
              public:
                void CreateFilter(std::basic_string<char, std::char_traits<char>, std::allocator<char>> const &);
                class FilterChain {
                  public:
                    void ShowConfiguration();
                };
            };

            class VptMode {
              public:
                int value;
            };

            class ToneType {
              public:
                int value;
            };

            class UserPresetNo {
              public:
                int value;
            };

            class Eq6Band {
                int value;
            };
            class Eq10Band {
              public:
                int value;
            };

            class Eq6BandPreset {
                int value;
            };

            class Eq10BandPreset {
                int value;
            };
            class EqType {
              public:
                int value;
            };

            class EffectSettingsDmp {};
            class ToneCenterFreq {
              public:
                int value;
            };
            class DseeHxCustomMode {};
            class DcPhaseFilterType {
              public:
                int value;
            };

            class EffectCtrlDmp {
              public:
                EffectCtrlDmp();
                ~EffectCtrlDmp();

                int GetVptMode();
                bool IsDseeAiOn();
                bool IsDseeHxOn();
                int SetEq6Band(bool);
                int SetVptMode(pst::services::sound::VptMode);
                bool IsEq6BandOn();
                int SetEq10Band(bool);
                int GetToneValue(pst::services::sound::ToneType);
                bool IsEq10BandOn();
                int SetToneValue(pst::services::sound::ToneType, int);
                int SetVinylizer(bool);
                bool IsVinylizerOn();
                int UpdateVptMode(pst::services::sound::UserPresetNo);
                int UpdateVptMode();
                int GetToneValuedB(pst::services::sound::ToneType);
                int LoadUserPreset(pst::services::sound::UserPresetNo);
                int SaveUserPreset(pst::services::sound::UserPresetNo);
                int SetToneControl(bool);
                int UpdateVptOnOff(pst::services::sound::UserPresetNo);
                int UpdateVptOnOff();
                int GetEq6BandValue(pst::services::sound::Eq6Band);
                bool IsToneControlOn();
                int SetDseeHxCustom(bool);
                int SetEq6BandValue(pst::services::sound::Eq6Band, int);
                int SetSourceDirect(bool);
                int GetEq10BandValue(pst::services::sound::Eq10Band);
                int GetEq6BandPreset();
                int GetSelectUsingEq();
                int GetVinylizerType();
                bool IsDseeHxCustomOn();
                bool IsSourceDirectOn();
                int SetEq10BandValue(pst::services::sound::Eq10Band, int);
                int SetEq6BandPreset(pst::services::sound::Eq6BandPreset);
                int SetSelectUsingEq(pst::services::sound::EqType);
                int SetVinylizerType(unsigned int);
                int UpdateToneValues(pst::services::sound::UserPresetNo);
                int UpdateToneValues();
                int GetEq6BandValuedB(pst::services::sound::Eq6Band);
                int GetPresetSettings(pst::services::sound::UserPresetNo, pst::services::sound::EffectSettingsDmp *);
                int GetToneCenterFreq(pst::services::sound::ToneType);
                int SetClearAudioPlus(bool);
                int SetToneCenterFreq(pst::services::sound::ToneType, pst::services::sound::ToneCenterFreq);
                int UpdateDseeAiOnOff(pst::services::sound::UserPresetNo);
                int UpdateDseeAiOnOff();
                int UpdateDseeHxOnOff(pst::services::sound::UserPresetNo);
                int UpdateDseeHxOnOff();
                int GetEq10BandValuedB(pst::services::sound::Eq10Band);
                bool IsClearAudioPlusOn();
                int UpdateEq6BandOnOff(pst::services::sound::UserPresetNo);
                int UpdateEq6BandOnOff();
                int DisableSoundEffects();
                int GetDseeHxCustomMode();
                int SetClearPhaseWmport(bool);
                int SetDseeHxCustomMode(pst::services::sound::DseeHxCustomMode);
                int UpdateEq10BandOnOff(pst::services::sound::UserPresetNo);
                int UpdateEq10BandOnOff();
                int UpdateEq6BandPreset(pst::services::sound::UserPresetNo);
                int UpdateEq6BandPreset();
                int UpdateEq6BandValues(pst::services::sound::UserPresetNo);
                int UpdateEq6BandValues(pst::services::sound::Eq6BandPreset);
                int UpdateEq6BandValues();
                int UpdateSelectUsingEq(pst::services::sound::UserPresetNo);
                int UpdateSelectUsingEq();
                int UpdateVinylizerType(pst::services::sound::UserPresetNo);
                int UpdateVinylizerType();
                int GetDcPhaseFilterType();
                bool IsClearPhaseWmportOn();
                int ReenableSoundEffects();
                int SetClearPhaseSpeaker(bool);
                int SetDcPhaseFilterType(pst::services::sound::DcPhaseFilterType);
                int SetDcPhaseLinearizer(bool);
                int SetDynamicNormalizer(bool);
                int UpdateEffectSettings(pst::services::sound::UserPresetNo);
                int UpdateEffectSettings();
                int UpdateEq10BandValues(pst::services::sound::UserPresetNo);
                int UpdateEq10BandValues();
                int UpdateVinylizerOnOff(pst::services::sound::UserPresetNo);
                int UpdateVinylizerOnOff();
                bool IsClearPhaseSpeakerOn();
                bool IsDcPhaseLinearizerOn();
                bool IsDynamicNormalizerOn();
                int SetBtAudioSoundEffect(bool);
                int UpdateToneCenterFreqs(pst::services::sound::UserPresetNo);
                int UpdateToneCenterFreqs();
                bool IsBtAudioSoundEffectOn();
                int SetClearPhaseHeadphone(bool);
                int UpdateDseeHxCustomMode(pst::services::sound::UserPresetNo);
                int UpdateDseeHxCustomMode();
                int UpdateToneControlOnOff(pst::services::sound::UserPresetNo);
                int UpdateToneControlOnOff();
                bool IsClearPhaseHeadphoneOn();
                int UpdateDcPhaseFilterType(pst::services::sound::UserPresetNo);
                int UpdateDcPhaseFilterType();
                int UpdateDseeHxCustomOnOff(pst::services::sound::UserPresetNo);
                int UpdateDseeHxCustomOnOff();
                int UpdateSourceDirectOnOff(pst::services::sound::UserPresetNo);
                int UpdateSourceDirectOnOff();
                int UpdateClearAudioPlusOnOff(pst::services::sound::UserPresetNo);
                int UpdateClearAudioPlusOnOff();
                int UpdateClearPhaseWmportOnOff(pst::services::sound::UserPresetNo);
                int UpdateClearPhaseWmportOnOff();
                int UpdateClearPhaseSpeakerOnOff(pst::services::sound::UserPresetNo);
                int UpdateClearPhaseSpeakerOnOff();
                int UpdateDcPhaseLinearizerOnOff(pst::services::sound::UserPresetNo);
                int UpdateDcPhaseLinearizerOnOff();
                int UpdateDynamicNormalizerOnOff(pst::services::sound::UserPresetNo);
                int UpdateDynamicNormalizerOnOff();
                int UpdateBtAudioSoundEffectOnOff(pst::services::sound::UserPresetNo);
                int UpdateBtAudioSoundEffectOnOff();
                int UpdateClearPhaseHeadphoneOnOff(pst::services::sound::UserPresetNo);
                int UpdateClearPhaseHeadphoneOnOff();
                int SetVpt(bool);
                bool IsVptOn();
                int SetDseeAi(bool);
                int SetDseeHx(bool);
            };
        } // namespace sound

        namespace configuration {
            struct GroupId {
                int value;
            };

            struct KeyId {
                int value;
            };

            //            class ConfigurationService {
            //            public:
            //                virtual void GetInt(unsigned int const&, unsigned int const&,
            //                int&); virtual ~ConfigurationService() {};
            //            };

            class Configuration {
              public:
                int ServiceClient{};

                Configuration();
                //                ~Configuration();
                bool GetCrcStatus();
                //                int GetLastStatus(pst::services::IConfigurationService::Status const&);

                //                int Get(std::vector<pst::services::configuration::SettingValue>&);
                //                int Set(std::vector<pst::services::configuration::SettingValue>&);
                int SetInt(pst::services::configuration::GroupId, pst::services::configuration::KeyId, int);
                int SetBool(pst::services::configuration::GroupId, pst::services::configuration::KeyId, bool);
                int SetString(pst::services::configuration::GroupId, pst::services::configuration::KeyId, std::string const &);
                int SetString(pst::services::configuration::GroupId, pst::services::configuration::KeyId, char const *);
                int
                SetBinary(pst::services::configuration::GroupId, pst::services::configuration::KeyId, unsigned int, unsigned char const *);
                int SetFloat(pst::services::configuration::GroupId, pst::services::configuration::KeyId, float);

                int GetSystemNvp(unsigned int const &, std::vector<unsigned char> &);
                int SetSystemNvp(unsigned int const &, std::vector<unsigned char> const &);

                void Reset();
                void Flush();
                int GetInt(pst::services::configuration::GroupId, pst::services::configuration::KeyId, int &);
                int GetBool(pst::services::configuration::GroupId, pst::services::configuration::KeyId, bool &);
                int GetFloat(pst::services::configuration::GroupId, pst::services::configuration::KeyId, float &);
                int GetBinary(pst::services::configuration::GroupId, pst::services::configuration::KeyId, unsigned int, unsigned char *);
                int GetString(pst::services::configuration::GroupId, pst::services::configuration::KeyId, std::string);
                int GetString(pst::services::configuration::GroupId, pst::services::configuration::KeyId, unsigned int, char *);

                int
                LockSecureParam(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned long long const &);
                int
                GetIntWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned long long const &, int &);
                int
                SetIntWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned long long const &, int const &);
                int
                GetBoolWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned long long const &, bool &);
                int
                SetBoolWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned long long const &, bool const &);
                int
                GetStringWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned long long const &, std::string &);
                int
                SetStringWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned long long const &, std::string const &);
                int
                GetStringWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned int const &, unsigned long long const &, char *);
                int
                SetStringWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned long long const &, char const *);
                int
                GetBinaryWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned int const &, unsigned long long const &, unsigned char *);
                int
                SetBinaryWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned int const &, unsigned long long const &, unsigned char const *);
                int
                GetFloatWithPassword(pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned long long const &, float &);
                int SetFloatWithPassword(
                    pst::services::configuration::GroupId const &, pst::services::configuration::KeyId const &, unsigned long long const &,
                    float
                );
            };
        } // namespace configuration
          /*
                  namespace playerservice {
                      class UnknownClass1 {
                        public:
                          virtual ~UnknownClass1(){};
                          virtual void ucf1() { fprintf(stderr, "%s\n", __PRETTY_FUNCTION__); };
                      };

                          class IPlayController {
                            public:
                              virtual ~IPlayController(){};
                              virtual int Connect(pst::playservice::PlayEventListener *);
                              virtual bool IsConnected() const;
                              virtual void Disconnect();
                              virtual void GetCurrentStatus(pst::playservice::PlayStatus &);
                              virtual void HasSetList(bool *);
                              virtual void ClosePlayer();
                              virtual void ChangePlayState(pst::playservice::playstate_t);
                              virtual void NextTrack();
                              virtual void PrevTrack(pst::playservice::PrevTrackOption const *);
                              virtual void NextGroup();
                              virtual void PrevGroup(pst::playservice::PrevGroupOption const *);
                              virtual void SeekTime(pst::playservice::media_origin_t, int);
                              virtual pst::playservice::PlayStatus *SetTrackSequence(std::shared_ptr<pst::playservice::TrackSequence> const
             &);   virtual void SetTrackSequenceWithDpcMode(   std::shared_ptr<pst::playservice::TrackSequence> const &,
             pst::playservice::dpc_mode_t
                              );
                              virtual void SetPlaySpeed(pst::playservice::PlaySpeed const &);
                              virtual void ResetPlaySpeed();
                              virtual void ResetPartialMode();
                              virtual void SetPartialMode(int, int);
                              virtual void Suspend();
                              virtual void Resume();
                              virtual void SetDpcKey(int);
                              virtual void SetSilenceEnabled(bool);
                              virtual void OnPlayStatusUpdated(unsigned int, pst::playservice::PlayStatus const &);
                              virtual void OnPlayTimeUpdated(int, int);
                              virtual void Alloc();
                              virtual void AllocNext();
                              virtual void AllocPrev();
                              virtual void OnNextTrack(pst::playservice::change_track_mode_t);
                              virtual void OnPrevTrack(pst::playservice::change_track_mode_t, bool);
                          };

                          class PlayerService;
                          class IPlayerService;
                          class PlayController : public IPlayController, public UnknownClass1 {
                            public:

                            public:
                              IPlayerService* PlayerService1;
                              int i1 = 0; // 0xC
                              int i2 = 0;
                              int i3 = 0;
                              int i4 = 0;
                              PlayerService* PlayerService; // expected 0x10, playerservice
                              int i6 = 0; // expected 0x14, iplayerservice
                              int i7 = 0; // expected 0x18
                              int i8;
                              char* Name; // expected 0x20
                              int i10;
                              int* session_id; // crash
                              int i12;
                              int i13;
                              int i14; // crash
                              int i15; // crash
                              int i16; // 0x48
                              // ... 0x30 - PlayEventListener
                              virtual ~PlayController();
                              PlayController(pst::services::playerservice::PlayerService*, IPlayerService*, char const*, char const*);
                              int Connect(pst::playservice::PlayEventListener*);
                              bool IsConnected() const;
                              virtual void Disconnect();
                              void GetCurrentStatus(pst::playservice::PlayStatus&);
                              void HasSetList(bool*);
                              void ClosePlayer();
                              void ChangePlayState(pst::playservice::playstate_t);
                              void NextTrack();
                              void PrevTrack(pst::playservice::PrevTrackOption const*);
                              void NextGroup();
                              void PrevGroup(pst::playservice::PrevGroupOption const*);
                              void SeekTime(pst::playservice::media_origin_t, int);
                              pst::playservice::PlayStatus* SetTrackSequence(std::shared_ptr<pst::playservice::TrackSequence> const&);
                              void SetTrackSequenceWithDpcMode(std::shared_ptr<pst::playservice::TrackSequence> const&,
             pst::playservice::dpc_mode_t);   void SetPlaySpeed(pst::playservice::PlaySpeed const&);   void ResetPlaySpeed();   void
             ResetPartialMode();   void SetPartialMode(int, int);   void Suspend();   void Resume();
                          };

                          class IPlayerService {
                              class PlayController_OnPlayStatusUpdated_Param {};
                              class PlayController_TrackSequence_Alloc_Request {};
                              class PlayController_TrackSequence_Param {};
                              class PlayController_TrackSequence_OnTrack_Request {};
                              class PlayController_OnPlayTimeUpdated_Param {};

                            public:
                              virtual void
                              PlayController_Alloc(IPlayerService::PlayController_TrackSequence_Alloc_Request const &,
             IPlayerService::PlayController_TrackSequence_Param &) = 0;   virtual void
                              PlayController_AllocNext(IPlayerService::PlayController_TrackSequence_Alloc_Request const &,
             IPlayerService::PlayController_TrackSequence_Param &) = 0;   virtual void
                              PlayController_AllocPrev(IPlayerService::PlayController_TrackSequence_Alloc_Request const &,
             IPlayerService::PlayController_TrackSequence_Param &) = 0;   virtual void
                              PlayController_OnNextTrack(IPlayerService::PlayController_TrackSequence_OnTrack_Request const &,
             IPlayerService::PlayController_TrackSequence_Param &) = 0;   virtual void
                              PlayController_OnPrevTrack(IPlayerService::PlayController_TrackSequence_OnTrack_Request const &,
             IPlayerService::PlayController_TrackSequence_Param &) = 0;   virtual void
             PlayController_OnPlayTimeUpdated(IPlayerService::PlayController_OnPlayTimeUpdated_Param const &) = 0;   virtual void
             PlayController_OnPlayStatusUpdated(IPlayerService::PlayController_OnPlayStatusUpdated_Param const &) = 0;   virtual void
             OverrideMe() { printf("override me"); };
                          };



                          class PlayerService : public IPlayerService {
                            public:
                              PlayerService();
                              virtual ~PlayerService();
                              //                IServiceClient* iServiceClient;
                              int i0;
                              int i1;
                              int i2;
                              int i3;
                              int* iServiceClient; //PlayerServiceClient
                              int i5;
                              int i6;
                              char* ServiceNameWithPid; // 0x20
                              int i7;
                              char* i8; // 0x28
                              int i9;
                              int i10; // 0x30
                              int i11; // 0x34
                              int i12;
                              int i13;
                              int i14;
                              char* LogString; // 0x44
                              int i16;
                              int i17;


                              static PlayerService *GetInstance();
                              PlayController getPlayController(char const *);
                          };
                      }
                      */
    }     // namespace services

    namespace dmpconfig {
        class Key {
          public:
            uint value;
            uint v2;
            uint v3;
        };

        class DmpConfigItem {};

        class DmpConfigData { // size 0xc
          public:
            int v1;
            int v2;
            int v3;
            int v4;
            void GetGroupId(pst::dmpconfig::Key const &);
            void GetInitialValue(pst::dmpconfig::Key const &);
            void Find(pst::dmpconfig::Key const &);
            const char *GetName(pst::dmpconfig::Key const &);
            void GetKeyId(pst::dmpconfig::Key const &);
            void GetKeyInt(pst::dmpconfig::Key const &);
            DmpConfigData(std::vector<pst::dmpconfig::DmpConfigItem>);
            ~DmpConfigData();
        };

        class DmpConfig {
          public:
            DmpConfigData *configData;
            std::map<int, int> m1{};
            std::map<int, int> m2{};
            int v1;
            //            int v2;
            //            int v3;
            //            int v4;
            //            int v5;
            //            int v6;
            //            int v7;
            //            DmpConfigData *configData2;

            DmpConfig();
            ~DmpConfig();
            void Nothing();
            void Nothing2();
            void RestoreFromTmpBuf();
            void RestoreInitialValues();
            void CheckConfigurationInitialized(pst::dmpconfig::Key const &);
            void CheckConfigurationFy18Initialized();
            void RestoreInitialValuesToConfiguration(pst::dmpconfig::Key const &);
            void RestoreInitialValuesToConfigurationFy18();
            void PushToTmpBuf(std::vector<pst::dmpconfig::Key> const &);
            void Get(pst::dmpconfig::Key const &, int &);
            void Set(pst::dmpconfig::Key const &, int const &, bool adbOn);
            void DumpAll();
        };

    } // namespace dmpconfig
} // namespace pst

#endif // DMPCONFIG_DMPCLIENT_H
