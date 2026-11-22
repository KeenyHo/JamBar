#pragma once
#include <JuceHeader.h>
#include <atomic>
#include <array>
#include <vector>

namespace jambar {

struct DrumPatternInfo
{
    const char* nameUtf8;
    float swing;         // 0.0 = straight 16th, 0.25..0.33 = shuffle/swing
    int crashEveryBars;  // 0 = none, 2 = every 2 bars, 4 = every 4 bars
    // 16-step velocity grids ('0' = rest, '1'..'9' = soft to hard accent)
    const char* kick;
    const char* snare;
    const char* chh;     // closed hi-hat
    const char* ohh;     // open hi-hat
    const char* ride;    // ride cymbal
    const char* crash;   // crash accent
    const char* perc;    // rimshot / sidestick / clap
};

struct DrumStyleInfo
{
    const char* nameUtf8;
    const DrumPatternInfo* patterns;
    int numPatterns;
};

class DrumMachine
{
public:
    enum Piece
    {
        Kick = 0,
        Snare,
        ClosedHat,
        OpenHat,
        Ride,
        Crash,
        Perc,
        NumPieces
    };

    static constexpr int kNumKits = 3;

    DrumMachine();

    void prepare(double sampleRate);
    void reset();

    // Thread-safe UI controls
    void setPlaying(bool play);
    bool isPlaying() const { return playing_.load(std::memory_order_acquire); }

    void setKitIndex(int kitIdx);
    int getKitIndex() const { return kitIndex_.load(std::memory_order_relaxed); }

    void setStyleIndex(int styleIdx);
    int getStyleIndex() const { return styleIndex_.load(std::memory_order_relaxed); }

    void setPatternIndex(int patternIdx);
    int getPatternIndex() const { return patternIndex_.load(std::memory_order_relaxed); }

    void setVolume(float vol) { volume_.store(juce::jlimit(0.0f, 1.5f, vol), std::memory_order_relaxed); }
    float getVolume() const { return volume_.load(std::memory_order_relaxed); }

    // Catalog metadata for UI dropdowns
    static int getNumKits() { return kNumKits; }
    static const char* getKitNameUtf8(int kitIndex);
    static int getNumStyles();
    static const DrumStyleInfo& getStyle(int styleIndex);

    // Audio-thread render: mixes real stereo drum kit directly into outL / outR.
    // activeBpm comes ONLY from room BPM when connected, or offline local BPM when disconnected
    // (NEVER DAW host BPM).
    void processAdd(float* outL, float* outR, int numSamples, float activeBpm,
                    double beatPos = -1.0);

private:
    struct SampleBuffer
    {
        std::vector<float> left;
        std::vector<float> right;
        double sampleRate = 44100.0;
        bool valid() const { return !left.empty() && left.size() == right.size(); }
    };

    struct SampleVoice
    {
        bool active = false;
        const SampleBuffer* buf = nullptr;
        Piece piece = Kick;
        double pos = 0.0;
        double inc = 1.0;
        float gain = 1.0f;
        bool choking = false;
        float chokeEnv = 1.0f;
    };

    void ensureSamplesLoaded();
    static SampleBuffer decodeOgg(const void* data, int sizeBytes);

    void triggerStep(int stepIdx, int barIdx, int kitIdx, const DrumPatternInfo& pat);
    void triggerPiece(int kitIdx, Piece piece, float velocity);
    float nextRand01();

    double sampleRate_ = 48000.0;
    bool samplesLoaded_ = false;

    // [kit][piece][layer: 0=soft, 1=hard]
    SampleBuffer kits_[kNumKits][NumPieces][2];

    static constexpr int kMaxVoices = 24;
    std::array<SampleVoice, kMaxVoices> voices_;

    std::atomic<bool> playing_{ false };
    std::atomic<int> kitIndex_{ 0 };      // Default: #3 GSCW 温暖真鼓
    std::atomic<int> styleIndex_{ 0 };
    std::atomic<int> patternIndex_{ 0 };
    std::atomic<float> volume_{ 0.78f };

    // Sequencer state (audio thread only)
    bool wasPlaying_ = false;
    int currentStep_ = 0;
    int barCount_ = 0;
    double samplesUntilNextStep_ = 0.0;
    uint32_t rngState_ = 0x9E3779B9u;
};

} // namespace jambar
