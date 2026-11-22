#include "LoopStation.h"
#include <algorithm>
#include <cmath>

namespace jambar {

LoopStation::LoopStation()
{
    for (int i = 0; i < kNumSlots; ++i)
    {
        pubState_[static_cast<size_t>(i)].store(static_cast<int>(SlotState::Empty), std::memory_order_relaxed);
        pubProgress_[static_cast<size_t>(i)].store(0.0f, std::memory_order_relaxed);
        volume_[static_cast<size_t>(i)].store(1.0f, std::memory_order_relaxed);
        clickPending_[static_cast<size_t>(i)].store(false, std::memory_order_relaxed);
    }
}

void LoopStation::prepare(double sampleRate)
{
    sampleRate_ = (sampleRate > 8000.0) ? sampleRate : 48000.0;
    // Pre-allocate 60 seconds per stereo loop slot (covers long high-BPI / slow-BPM rooms cleanly)
    const size_t cap = std::min<size_t>(
        static_cast<size_t>(sampleRate_ * 60.0),
        static_cast<size_t>(60u * 96000u));

    if (cap != maxLoopSamples_)
    {
        maxLoopSamples_ = cap;
        for (int i = 0; i < kNumSlots; ++i)
        {
            auto& s = slots_[static_cast<size_t>(i)];
            s.left.assign(maxLoopSamples_, 0.0f);
            s.right.assign(maxLoopSamples_, 0.0f);
            s.state = SlotState::Empty;
            s.writePos = 0;
            s.recordedLen = 0;
            s.playPos = 0;
            pubState_[static_cast<size_t>(i)].store(static_cast<int>(SlotState::Empty), std::memory_order_relaxed);
            pubProgress_[static_cast<size_t>(i)].store(0.0f, std::memory_order_relaxed);
            clickPending_[static_cast<size_t>(i)].store(false, std::memory_order_relaxed);
        }
    }
}

void LoopStation::clearAll()
{
    clearAllPending_.store(true, std::memory_order_release);
}

void LoopStation::triggerButton(int slot)
{
    if (slot < 0 || slot >= kNumSlots)
        return;
    clickPending_[static_cast<size_t>(slot)].store(true, std::memory_order_release);
}

void LoopStation::setVolume(int slot, float vol)
{
    if (slot < 0 || slot >= kNumSlots)
        return;
    volume_[static_cast<size_t>(slot)].store(juce::jlimit(0.0f, 1.5f, vol), std::memory_order_relaxed);
}

float LoopStation::getVolume(int slot) const
{
    if (slot < 0 || slot >= kNumSlots)
        return 1.0f;
    return volume_[static_cast<size_t>(slot)].load(std::memory_order_relaxed);
}

LoopStation::SlotState LoopStation::getState(int slot) const
{
    if (slot < 0 || slot >= kNumSlots)
        return SlotState::Empty;
    return static_cast<SlotState>(pubState_[static_cast<size_t>(slot)].load(std::memory_order_relaxed));
}

float LoopStation::getProgress(int slot) const
{
    if (slot < 0 || slot >= kNumSlots)
        return 0.0f;
    return pubProgress_[static_cast<size_t>(slot)].load(std::memory_order_relaxed);
}

bool LoopStation::isAnyActive() const
{
    for (int i = 0; i < kNumSlots; ++i)
    {
        if (pubState_[static_cast<size_t>(i)].load(std::memory_order_relaxed) != static_cast<int>(SlotState::Empty)
            || clickPending_[static_cast<size_t>(i)].load(std::memory_order_relaxed))
            return true;
    }
    return false;
}

void LoopStation::finalizeRecording(SlotData& s)
{
    s.recordedLen = s.writePos;
    if (s.recordedLen < 64)
    {
        s.state = SlotState::Empty;
        s.recordedLen = 0;
        s.writePos = 0;
        s.playPos = 0;
        return;
    }

    // Apply click-free 5ms edge fade-in and fade-out at loop boundaries
    const size_t fadeN = std::min<size_t>(
        s.recordedLen / 4,
        static_cast<size_t>(sampleRate_ * 0.005));

    if (fadeN > 1)
    {
        const float inv = 1.0f / static_cast<float>(fadeN);
        for (size_t k = 0; k < fadeN; ++k)
        {
            const float g = static_cast<float>(k) * inv;
            s.left[k] *= g;
            s.right[k] *= g;

            const size_t tailIdx = s.recordedLen - 1 - k;
            s.left[tailIdx] *= g;
            s.right[tailIdx] *= g;
        }
    }

    s.state = SlotState::Playing;
    s.playPos = 0;
}

void LoopStation::handleBoundary(int slotIdx)
{
    auto& s = slots_[static_cast<size_t>(slotIdx)];
    if (s.state == SlotState::Armed)
    {
        // Start recording cleanly at Beat 1 of the new loop!
        s.state = SlotState::Recording;
        s.writePos = 0;
        s.recordedLen = 0;
        s.playPos = 0;
    }
    else if (s.state == SlotState::Recording)
    {
        // Guard against any spurious micro-boundary (< 200ms) so recording never self-aborts on glitch
        const size_t minValidLoop = static_cast<size_t>(sampleRate_ * 0.20);
        if (s.writePos >= minValidLoop)
            finalizeRecording(s);
    }
    else if (s.state == SlotState::Playing)
    {
        // Keep playing loops tightly phase-locked to Beat 1 of every loop
        s.playPos = 0;
    }
}

void LoopStation::process(const float* dryInL, const float* dryInR,
                          float* mixOutL, float* mixOutR,
                          int numSamples,
                          int boundarySampleIndex,
                          int expectedLoopSamples)
{
    if (numSamples <= 0 || maxLoopSamples_ == 0)
        return;

    // Handle clear-all request (e.g. session reset)
    if (clearAllPending_.exchange(false, std::memory_order_acquire))
    {
        for (int i = 0; i < kNumSlots; ++i)
        {
            auto& s = slots_[static_cast<size_t>(i)];
            s.state = SlotState::Empty;
            s.writePos = 0;
            s.recordedLen = 0;
            s.playPos = 0;
            pubState_[static_cast<size_t>(i)].store(static_cast<int>(SlotState::Empty), std::memory_order_relaxed);
            pubProgress_[static_cast<size_t>(i)].store(0.0f, std::memory_order_relaxed);
        }
    }

    // Consume UI button clicks lock-free
    for (int i = 0; i < kNumSlots; ++i)
    {
        if (clickPending_[static_cast<size_t>(i)].exchange(false, std::memory_order_acquire))
        {
            auto& s = slots_[static_cast<size_t>(i)];
            if (s.state == SlotState::Empty)
            {
                s.state = SlotState::Armed;
                s.writePos = 0;
                s.recordedLen = 0;
                s.playPos = 0;
            }
            else
            {
                // Armed / Recording / Playing -> Clear and Stop immediately
                s.state = SlotState::Empty;
                s.writePos = 0;
                s.recordedLen = 0;
                s.playPos = 0;
            }
        }
    }

    float vols[kNumSlots];
    for (int i = 0; i < kNumSlots; ++i)
        vols[i] = volume_[static_cast<size_t>(i)].load(std::memory_order_relaxed);

    for (int n = 0; n < numSamples; ++n)
    {
        // Trigger exact sample-accurate loop boundary transitions
        if (n == boundarySampleIndex)
        {
            for (int i = 0; i < kNumSlots; ++i)
                handleBoundary(i);
        }

        const float inL = dryInL ? dryInL[n] : 0.0f;
        const float inR = dryInR ? dryInR[n] : inL;

        float loopMixL = 0.0f;
        float loopMixR = 0.0f;

        for (int i = 0; i < kNumSlots; ++i)
        {
            auto& s = slots_[static_cast<size_t>(i)];
            if (s.state == SlotState::Recording)
            {
                if (s.writePos < maxLoopSamples_)
                {
                    s.left[s.writePos] = inL;
                    s.right[s.writePos] = inR;
                    ++s.writePos;
                }
                if (s.writePos >= maxLoopSamples_)
                {
                    // Reached max 40s safety capacity -> finalize cleanly
                    finalizeRecording(s);
                }
            }
            else if (s.state == SlotState::Playing && s.recordedLen > 0)
            {
                const float v = vols[i];
                loopMixL += s.left[s.playPos] * v;
                loopMixR += s.right[s.playPos] * v;
                ++s.playPos;
                if (s.playPos >= s.recordedLen)
                    s.playPos = 0;
            }
        }

        if (mixOutL) mixOutL[n] += loopMixL;
        if (mixOutR) mixOutR[n] += loopMixR;
    }

    // Publish states & normalized progress for 20Hz UI
    const float denomRec = static_cast<float>(juce::jmax(1, expectedLoopSamples));
    for (int i = 0; i < kNumSlots; ++i)
    {
        const auto& s = slots_[static_cast<size_t>(i)];
        pubState_[static_cast<size_t>(i)].store(static_cast<int>(s.state), std::memory_order_relaxed);
        float prog = 0.0f;
        if (s.state == SlotState::Recording)
            prog = juce::jlimit(0.0f, 1.0f, static_cast<float>(s.writePos) / denomRec);
        else if (s.state == SlotState::Playing && s.recordedLen > 0)
            prog = juce::jlimit(0.0f, 1.0f, static_cast<float>(s.playPos) / static_cast<float>(s.recordedLen));
        pubProgress_[static_cast<size_t>(i)].store(prog, std::memory_order_relaxed);
    }
}

} // namespace jambar
