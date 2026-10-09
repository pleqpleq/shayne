#pragma once
#include <deque>
#include <algorithm>

// Calculate the actual speed of distinct physical presses (not keyboard auto-repeat).
// A small rolling window detects fast spam quickly instead of taking a full second.
class TapSpeed {
public:
    void reset() {
        m_pressTimes.clear();
        m_fast = false;
    }

    bool press(double seconds, double threshold = 8.0) {
        if (!m_pressTimes.empty()) {
            const double gap = seconds - m_pressTimes.back();
            if (gap < 0.0 || gap > 0.35) {
                reset(); // avoid stale CPS after pauses, restarts, and slow taps
            }
        }
        m_pressTimes.push_back(seconds);
        if (m_pressTimes.size() > 5) m_pressTimes.pop_front();
        if (m_pressTimes.size() >= 4) {
            const double duration = m_pressTimes.back() - m_pressTimes.front();
            m_fast = duration > 0.0 && (m_pressTimes.size()-1) / duration > threshold;
        } else {
            m_fast = false;
        }
        return m_fast;
    }

    bool fast() const { return m_fast; }

private:
    std::deque<double> m_pressTimes;
    bool m_fast = false;
};
