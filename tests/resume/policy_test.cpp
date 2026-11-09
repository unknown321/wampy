// Host-only policy tests. This does NOT establish actual device event behavior.
#include "../../src/resume_policy.hpp"
#include <cassert>
#include <iostream>
using namespace wampy_review;

void activate(ResumePolicy& p, long now = 0) {
    p.observeDisplay(true, true, now);
    p.preferWampy(true, now);
    p.observeNative(p.generation(), Visibility::Visible, true, false);
    assert(p.step(now + 150).action == Action::HideNative);
    p.observeNative(p.generation(), Visibility::Hidden, true, true);
    assert(p.step(now + 250).render);
}
int main() {
    ResumePolicy p;
    p.preferWampy(true, 0);
    assert(!p.step(10000).render); // unknown display must not draw
    p.observeDisplay(true, false, 10001);
    assert(p.step(20000).action == Action::None);
    std::cout << "PASS: unknown/off display blocks rendering and commands\n";

    ResumePolicy q; activate(q);
    q.powerReleased();
    q.observeDisplay(true, false, 1000);
    auto old = q.generation();
    assert(q.wanted() && q.step(600000).action == Action::None);
    q.observeDisplay(true, true, 600000);
    q.powerReleased(); // release arriving AFTER wake must not turn render off
    q.observeNative(old, Visibility::Hidden, true, true);
    assert(q.step(600150).action == Action::QueryNative);
    q.observeNative(q.generation(), Visibility::Visible, true, false);
    assert(q.step(600250).action == Action::HideNative);
    q.observeNative(q.generation(), Visibility::Hidden, true, false);
    assert(q.step(600350).action == Action::HideNative);
    q.observeNative(q.generation(), Visibility::Hidden, true, true);
    assert(q.step(600450).render);
    std::cout << "PASS: sleep intent retained, stale reply rejected, native quiescence required\n";

    auto stable = q.generation();
    q.observeDisplay(true, true, 600460);
    q.powerReleased();
    assert(q.generation() == stable && q.step(600470).render);
    std::cout << "PASS: duplicate ON and reordered POWER hints do not reset an active UI\n";

    q.observeDisplay(true, false, 601000);
    q.preferWampy(false, 601010);
    q.observeDisplay(true, true, 602000);
    assert(!q.wanted() && q.step(602500).action == Action::ShowNative);
    assert(q.step(602501).action == Action::None);
    ResumePolicy exit; activate(exit);
    exit.preferWampy(false, 500);
    // No new observation is required to honor an explicit HOLD exit.
    assert(exit.step(501).action == Action::ShowNative);
    assert(exit.step(502).action == Action::None);
    std::cout << "PASS: HOLD off during sleep cancels automatic Wampy return\n";

    ResumePolicy t; activate(t);
    t.observeDisplay(true, false, 1000);
    t.observeDisplay(true, true, 2000);
    t.observeNative(t.generation(), Visibility::Visible, true, false);
    for (int i = 0; i < 3; ++i) assert(t.step(2150 + i * 100).action == Action::HideNative);
    assert(t.step(2450).action == Action::ShowNative && t.phase() == Phase::Failed);
    assert(t.hideAttempts() == 3 && t.step(10000).action == Action::None);
    std::cout << "PASS: hide retry count bounded; failure does not restart forever\n";

    ResumePolicy u; u.observeDisplay(true, true, 0); u.preferWampy(true, 0);
    assert(u.step(150).action == Action::QueryNative);
    assert(u.step(2000).action == Action::ShowNative && u.phase() == Phase::Failed);
    std::cout << "PASS: unready native backend reaches a finite timeout\n";

    ResumePolicy g; g.observeDisplay(true, true, 0); g.preferWampy(true, 0);
    g.observeNative(g.generation(), Visibility::Hidden, true, true);
    g.setGraphicsHealthy(false);
    assert(g.step(150).action == Action::RecoverGraphics);
    assert(g.step(160).action == Action::None);
    g.setGraphicsHealthy(true); assert(g.step(170).render);
    g.setStorageExported(true, 200);
    assert(g.step(201).action == Action::ShowNative && !g.step(202).render);
    std::cout << "PASS: graphics repair requested once; USB export yields to Sony UI\n";

    ResumePolicy n; activate(n);
    n.observeNative(n.generation(), Visibility::Visible, true, false);
    auto previous = n.generation();
    assert(!n.step(500).render && n.generation() != previous);
    assert(n.phase() == Phase::Failed && n.step(10000).action == Action::None);
    std::cout << "PASS: observed native takeover invalidates previous ownership\n";
}
