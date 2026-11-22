#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <array>
#include <vector>

namespace jambar {

/**
 * 3-Track Interval-Synchronized Stereo Loop Station.
 *
 * Per-slot state machine:
 *   Empty     --[click]--> Armed (waits for next loop Beat-1 boundary)
 *   Armed     --[boundary]--> Recording (captures dry stereo input for 1 full loop)
 *   Armed     --[click]--> Empty (cancel arm)
 *   Recording --[boundary]--> Playing (auto-stops recording, loops forever, phase-locked)
 *   Recording --[click]--> Empty (abort & clear)
 *   Playing   --[click]--> Empty (clear & stop)
 *
 * Audio-thread guarantees:
 *   - 100% lock-free & allocation-free inside process()
 *   - Buffers pre-allocated in prepare(sampleRate)
 *   - Recording taps dry local input BEFORE DrumMachine and BEFORE Loop playback mix
 *     so existing loops & drums never bleed/feedback into newly recorded loops.
 *   - Playback mixes into local stereo input so all 1..3 loops are monitored locally
 *     AND broadcast to the room automatically.
 */
class LoopStation
{
public:
    static constexpr int kNumSlots = 3;

    enum class SlotState : int
    {
        Empty = 0,
        Armed = 1,
        Recording = 2,
        Playing = 3
    };

    LoopStation();

    void prepare(double sampleRate);
    void clearAll();

    // Thread-safe UI controls
    void triggerButton(int slot);
    void setVolume(int slot, float vol);
    float getVolume(int slot) const;

    SlotState getState(int slot) const;
    float getProgress(int slot) const;
    bool isAnyActive() const;

    /**
     * Audio-thread processing.
     * @param dryInL/dryInR        Clean local stereo input BEFORE drum machine & loops
     * @param mixOutL/mixOutR      Stereo destination buffer (inPtrs[0..1]) to add playing loops into
     * @param numSamples           Block size
     * @param boundarySampleIndex  Exact sample index [0, numSamples-1] where next loop Beat 1 starts, or -1 if none in this block
     * @param expectedLoopSamples  Nominal loop length in samples (used only for recording progress bar)
     */
    void process(const float* dryInL, const float* dryInR,
                 float* mixOutL, float* mixOutR,
                 int numSamples,
                 int boundarySampleIndex,
                 int expectedLoopSamples);

private:
    struct SlotData
    {
        std::vector<float> left;
        std::vector<float> right;
        SlotState state = SlotState::Empty;
        size_t writePos = 0;
        size_t recordedLen = 0;
        size_t playPos = 0;
    };

    void finalizeRecording(SlotData& s);
    void handleBoundary(int slot);

    double sampleRate_ = 48000.0;
    size_t maxLoopSamples_ = 0;
    std::array<SlotData, kNumSlots> slots_;

    std::array<std::atomic<int>, kNumSlots> pubState_;
    std::array<std::atomic<float>, kNumSlots> pubProgress_;
    std::array<std::atomic<float>, kNumSlots> volume_;
    std::array<std::atomic<bool>, kNumSlots> clickPending_;
    std::atomic<bool> clearAllPending_{ false };
};

} // namespace jambar
