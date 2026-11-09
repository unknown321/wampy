// Client lifecycle policy integrated in main.cpp. GPL-3.0-or-later.
// All methods run on ONE owner thread. Adapters must provide ordered, fresh
// display/native/GPU observations. QWindow::isVisible() alone does NOT prove
// the native renderer is quiescent. No device I/O or power writes exist here.
#pragma once
#include <cstdint>

namespace wampy_review {
enum class Visibility { Unknown, Visible, Hidden };
enum class Action { None, QueryNative, HideNative, ShowNative, RecoverGraphics, Draw };
enum class Phase { Idle, Waiting, Active, Failed };
struct Decision {
    Action action;
    std::uint64_t generation;
    bool render;
};

class ResumePolicy {
public:
    // Design values only. Device traces must determine real timing bounds.
    static const int settle_ms = 0;
    static const int timeout_ms = 2000;
    static const int retry_ms = 100;
    static const int max_hide_attempts = 3;

    void preferWampy(bool wanted, std::int64_t now) {
        if (wanted_ == wanted) return;
        yieldRequested_ = !wanted;
        wanted_ = wanted;
        begin(now);
    }
    void observeDisplay(bool known, bool on, std::int64_t now) {
        if (known_ == known && on_ == on) return;
        known_ = known;
        on_ = on;
        begin(now);
    }
    // POWER release is only a hint: it does not distinguish blank from wake.
    void powerReleased() {}
    void setStorageExported(bool value, std::int64_t now) {
        if (exported_ == value) return;
        if (value && wanted_) yieldRequested_ = true;
        if (!value) yieldRequested_ = false;
        exported_ = value;
        begin(now);
    }
    void setGraphicsHealthy(bool healthy) { graphicsHealthy_ = healthy; }
    void observeNative(std::uint64_t generation, Visibility visibility,
                       bool backendReady, bool renderThreadQuiescent) {
        if (generation != generation_) return; // reject a previous wake's reply
        visibility_ = visibility;
        backendReady_ = backendReady;
        quiescent_ = renderThreadQuiescent;
    }
    std::uint64_t generation() const { return generation_; }
    Phase phase() const { return phase_; }
    void giveUp() { phase_ = Phase::Failed; }
    bool wanted() const { return wanted_; }
    int hideAttempts() const { return hideAttempts_; }

    Decision step(std::int64_t now) {
        if (!known_ || !on_) return emit(Action::None);
        if (!wanted_ || exported_) {
            if (!fallbackIssued_ && (yieldRequested_ || visibility_ == Visibility::Hidden)) {
                fallbackIssued_ = true;
                yieldRequested_ = false;
                return emit(Action::ShowNative);
            }
            return emit(Action::None);
        }
        if (phase_ == Phase::Failed) return emit(Action::None);
        if (phase_ == Phase::Active) {
            if (visibility_ == Visibility::Visible) {
                begin(now);
                phase_ = Phase::Failed; // yield to native UI; do not fight a popup
                return emit(Action::None);
            }
            if (visibility_ == Visibility::Hidden && quiescent_ && graphicsHealthy_)
                return emit(Action::Draw);
            begin(now); // native takeover or graphics error invalidates ownership
        }
        if (now >= deadline_) {
            phase_ = Phase::Failed;
            fallbackIssued_ = true;
            return emit(Action::ShowNative); // one attempt, no restart loop
        }
        if (now < nextAction_) return emit(Action::None);
        if (!backendReady_ || visibility_ == Visibility::Unknown) {
            nextAction_ = now + retry_ms;
            return emit(Action::QueryNative);
        }
        if (visibility_ == Visibility::Visible || !quiescent_) {
            if (hideAttempts_ >= max_hide_attempts) {
                phase_ = Phase::Failed;
                return emit(Action::ShowNative);
            }
            ++hideAttempts_;
            nextAction_ = now + retry_ms;
            return emit(Action::HideNative);
        }
        if (!quiescent_) {
            nextAction_ = now + retry_ms;
            return emit(Action::QueryNative);
        }
        if (!graphicsHealthy_) {
            if (!recoveryIssued_) {
                recoveryIssued_ = true;
                return emit(Action::RecoverGraphics);
            }
            return emit(Action::None);
        }
        phase_ = Phase::Active;
        return emit(Action::Draw);
    }

private:
    bool wanted_ = false, known_ = false, on_ = false, exported_ = false;
    bool backendReady_ = false, quiescent_ = false, graphicsHealthy_ = true;
    bool recoveryIssued_ = false, fallbackIssued_ = false, yieldRequested_ = false;
    Visibility visibility_ = Visibility::Unknown;
    Phase phase_ = Phase::Idle;
    std::uint64_t generation_ = 0;
    std::int64_t deadline_ = 0, nextAction_ = 0;
    int hideAttempts_ = 0;

    void begin(std::int64_t now) {
        ++generation_;
        phase_ = wanted_ && known_ && on_ && !exported_ ? Phase::Waiting : Phase::Idle;
        visibility_ = Visibility::Unknown;
        backendReady_ = false;
        quiescent_ = false;
        hideAttempts_ = 0;
        recoveryIssued_ = fallbackIssued_ = false;
        deadline_ = now + timeout_ms;
        nextAction_ = now + settle_ms;
    }
    Decision emit(Action action) const {
        return {action, generation_, action == Action::Draw};
    }
};
} // namespace wampy_review
