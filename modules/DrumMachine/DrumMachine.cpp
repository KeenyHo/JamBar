#include "DrumMachine.h"
#include "JamBarDrumData.h"
#include <cmath>
#include <algorithm>

namespace jambar {

namespace {

// ============================================================================
// 12 Musical Styles x 4-5 Studio Grooves = 54 Rich Patterns
// Step digits: '0'=rest, '1'..'9'=velocity (<=5 triggers Soft layer, >5 triggers Hard layer)
// ============================================================================

const DrumPatternInfo kPopPatterns[] = {
    { "经典流行 8Beat", 0.0f, 4,
      "9000000090800000", "0000900000009000", "7050705070507000", "0000000000000070", "0000000000000000", "8000000000000000", "0000000000000000" },
    { "现代律动 16Beat", 0.03f, 4,
      "9000006000900000", "0000900000009030", "8464846484648400", "0000000000000070", "0000000000000000", "7000000000000000", "0000000000000000" },
    { "抒情慢摇 Ballad", 0.0f, 4,
      "9000000000800000", "0000000090000000", "6040504060405000", "0000000000000050", "0000000000000000", "6000000000000000", "0000500000000000" },
    { "轻快切分 Pop", 0.05f, 4,
      "9000007000800050", "0000900000009000", "8060806080608060", "0000006000000000", "0000000000000000", "8000000000000000", "0000000000000040" },
    { "副歌推进 Chorus", 0.0f, 2,
      "9000008090000070", "0000900000009000", "0000000000000000", "0000000000000070", "8060806080608060", "9000000000000000", "0000000000000000" },
};

const DrumPatternInfo kRockPatterns[] = {
    { "经典摇滚 Rock", 0.0f, 4,
      "9000007090000000", "0000900000009000", "8060806080608000", "0000000000000080", "0000000000000000", "9000000000000000", "0000000000000000" },
    { "硬摇滚 Ride 强拍", 0.0f, 2,
      "9080008090800000", "0000900000009000", "0000000000000000", "0000000000000000", "9060806090608060", "9000000000000000", "0000000000000000" },
    { "朋克疾走 Punk", 0.0f, 2,
      "9000009090000090", "0000900000009000", "0000000000000000", "8070807080708070", "0000000000000000", "9000000000000000", "0000000000000000" },
    { "布鲁斯摇滚 Shuffle", 0.28f, 4,
      "9000000090070000", "0000900000009003", "8060806080608060", "0000000000000070", "0000000000000000", "8000000000000000", "0000000000000000" },
    { "重拍半速 Half-Time", 0.0f, 4,
      "9000000000800060", "0000000090000000", "8060806080608000", "0000000000000080", "0000000000000000", "9000000000000000", "0000000000000000" },
};

const DrumPatternInfo kFunkPatterns[] = {
    { "切分放克 Funk", 0.08f, 4,
      "9000000700900060", "0000900303009002", "8474840084748464", "0000007000000000", "0000000000000000", "7000000000000000", "0000000000000040" },
    { "律动 Slap Groove", 0.06f, 4,
      "9080000000900080", "0000900400049000", "8565856585658500", "0000000000000070", "0000000000000000", "7000000000000000", "0000000000000000" },
    { "迪斯科四拍 Disco", 0.0f, 4,
      "9000900090009000", "0000800000008000", "7000700070007000", "0070007000700070", "0000000000000000", "7000000000000000", "0000600000006000" },
    { "灵魂乐 Soul", 0.10f, 4,
      "9000007000800000", "0000900000009030", "7050705070507050", "0000000000000060", "0000000000000000", "6000000000000000", "0000000000000000" },
    { "新派 Neo-Soul", 0.22f, 0,
      "9000000600800000", "0000800203008003", "6353635363536353", "0000000000000050", "0000000000000000", "0000000000000000", "0000500000000000" },
};

const DrumPatternInfo kJazzPatterns[] = {
    { "标准摇摆 Swing", 0.31f, 0,
      "5000000050000000", "0000003000000030", "0000700000007000", "0000000000000000", "8000706080007060", "0000000000000000", "0000000000000000" },
    { "波萨诺瓦 Bossa Nova", 0.0f, 0,
      "8006800680068006", "0000000000000000", "6464646464646464", "0000000000000000", "0000000000000000", "0000000000000000", "7007007000700700" },
    { "慢板爵士 Ballad", 0.25f, 0,
      "5000000000400000", "0000300000003000", "0000600000006000", "0000000000000000", "6000504060005040", "0000000000000000", "0000000000000000" },
    { "拉丁融合 Fusion", 0.04f, 4,
      "9000007000900700", "0000800300038000", "7565756575657500", "0000000000000060", "6000600060006000", "7000000000000000", "0000000000000050" },
    { "快板摇摆 Bebop", 0.28f, 0,
      "4000000000400000", "0000004000300040", "0000800000008000", "0000000000000000", "9000807090008070", "0000000000000000", "0000000000000000" },
};

const DrumPatternInfo kBluesPatterns[] = {
    { "芝加哥摇摆 Shuffle", 0.30f, 4,
      "9000000090070000", "0000900000009004", "0000000000000000", "0000000000000000", "8050705080507050", "7000000000000000", "0000000000000000" },
    { "慢速布鲁斯 Slow", 0.30f, 4,
      "9000000000700000", "0000800000008000", "6040504060405040", "0000000000000050", "0000000000000000", "6000000000000000", "0000000000000000" },
    { "德州滑步 Texas", 0.30f, 4,
      "9000900090009000", "0000900500009005", "8060806080608060", "0000000000000060", "0000000000000000", "7000000000000000", "0000000000000000" },
    { "轻快 Jump Blues", 0.28f, 4,
      "8000800080008000", "0000800300008003", "0000700000007000", "0000000000000000", "8000706080007060", "7000000000000000", "0000000000000000" },
};

const DrumPatternInfo kReggaePatterns[] = {
    { "One Drop 雷鬼", 0.14f, 0,
      "0000000090000000", "0000000000000000", "6050705060507000", "0000000000000060", "0000000000000000", "0000000000000000", "0000000080000000" },
    { "Steppers 四拍雷鬼", 0.12f, 4,
      "9000900090009000", "0000000080000000", "7050705070507050", "0000000000000060", "0000000000000000", "6000000000000000", "0000000070000000" },
    { "Rockers 切分雷鬼", 0.14f, 4,
      "9000000090000070", "0000000090000000", "7050705070507050", "0000006000000000", "0000000000000000", "7000000000000000", "0000500000005000" },
    { "轻快 Ska 斯卡", 0.0f, 4,
      "9000000090000000", "0000900000009000", "0070007000700070", "0000000000000060", "0000000000000000", "7000000000000000", "0000000000000000" },
};

const DrumPatternInfo kLatinPatterns[] = {
    { "桑巴 Samba", 0.0f, 4,
      "9007900790079007", "0000000000000000", "7565756575657565", "0000000000000000", "0000000000000000", "6000000000000000", "7007070007007070" },
    { "拉丁恰恰 Cha-Cha", 0.0f, 0,
      "8000007080000070", "0000000000000000", "6060606060606060", "0000000000000000", "7000707070007070", "0000000000000000", "0000700000007070" },
    { "曼波 Mambo", 0.0f, 4,
      "8000007000008000", "0000000000000000", "0000000000000000", "0000000000000000", "8007007000707000", "7000000000000000", "7007007000707000" },
    { "雷击顿 Reggaeton", 0.0f, 4,
      "9000900090009000", "0008008000080080", "6050605060506050", "0000000000000000", "0000000000000000", "7000000000000000", "0007007000070070" },
};

const DrumPatternInfo kHipHopPatterns[] = {
    { "Boom Bap 经典", 0.14f, 0,
      "9000000700800000", "0000900000009000", "7050705070507000", "0000000000000060", "0000000000000000", "0000000000000000", "0000500000005000" },
    { "Lo-Fi 慵懒摇摆", 0.24f, 0,
      "9000000600800000", "0000800000008003", "6040504060405040", "0000000000000050", "0000000000000000", "0000000000000000", "0000000000000000" },
    { "Trap 现代滚镲", 0.0f, 4,
      "9000000000800060", "0000000090000000", "7676765776767657", "0000000000000070", "0000000000000000", "7000000000000000", "0000000070000000" },
    { "老派 Old School", 0.08f, 0,
      "9000008000900000", "0000900000009000", "7060706070607060", "0000000000000070", "0000000000000000", "0000000000000000", "0000700000007000" },
    { "爵士说唱 JazzHop", 0.20f, 0,
      "8000006000800000", "0000800200028000", "6454645464546454", "0000000000000000", "5000000050000000", "0000000000000000", "0000000000000000" },
};

const DrumPatternInfo kElectronicPatterns[] = {
    { "House 四拍电子", 0.0f, 4,
      "9000900090009000", "0000000000000000", "6040604060406040", "0080008000800080", "0000000000000000", "7000000000000000", "0000800000008000" },
    { "Techno 工业四拍", 0.0f, 4,
      "9000900090009000", "0000000000000000", "7575757575757575", "0070007000700070", "6060606060606060", "6000000000000000", "0000600000006000" },
    { "碎拍 Breakbeat", 0.04f, 4,
      "9000000000900000", "0000900303009000", "7565756575657500", "0000000000000070", "0000000000000000", "8000000000000000", "0000000000000000" },
    { "Drum & Bass 疾速", 0.0f, 4,
      "9000000000900000", "0000900000000090", "8464846484648464", "0000000000000060", "0000000000000000", "8000000000000000", "0000000000000000" },
    { "Synthwave 80s", 0.0f, 4,
      "9000000090000000", "0000900000009000", "7050705070507050", "0000000000000070", "0000000000000000", "8000000000000000", "0000700000007000" },
};

const DrumPatternInfo kMetalPatterns[] = {
    { "双踩疾速 Double Bass", 0.0f, 2,
      "9797979797979797", "0000900000009000", "0000000000000000", "8080808080808080", "0000000000000000", "9000000000000000", "0000000000000000" },
    { "鞭击金属 Thrash", 0.0f, 2,
      "9000900090009000", "0090009000900090", "0000000000000000", "8080808080808080", "0000000000000000", "9000000000000000", "0000000000000000" },
    { "重型半拍 Breakdown", 0.0f, 2,
      "9090009000909000", "0000000090000000", "0000000000000000", "8000800080008000", "0000000000000000", "9000900090009000", "0000000000000000" },
    { "金属核切分 Metalcore", 0.0f, 2,
      "9700979000970090", "0000900000009000", "0000000000000000", "0000000000000000", "9060906090609060", "9000000000000000", "0000000000000000" },
};

const DrumPatternInfo kFolkPatterns[] = {
    { "轻快乡村 Train Beat", 0.0f, 4,
      "8000000080000000", "5383538353835383", "6050605060506050", "0000000000000000", "0000000000000000", "6000000000000000", "0000000000000000" },
    { "木吉他扫弦 Acoustic", 0.0f, 4,
      "8000007000700000", "0000700000007000", "6040504060405040", "0000000000000050", "0000000000000000", "6000000000000000", "0000600000006000" },
    { "蓝草切分 Bluegrass", 0.05f, 0,
      "8000000080000000", "0000700000007000", "7050705070507050", "0000000000000000", "0000000000000000", "0000000000000000", "0000700000007000" },
    { "圆舞曲感 3/4 Feel", 0.0f, 0,
      "8000000000000000", "0000000000000000", "7000500050000000", "0000000000000000", "6000500050000000", "0000000000000000", "0000600060000000" },
};

const DrumPatternInfo kPracticePatterns[] = {
    { "四分基础节拍 Quarter", 0.0f, 0,
      "8000000080000000", "0000800000008000", "7000700070007000", "0000000000000000", "0000000000000000", "0000000000000000", "0000000000000000" },
    { "八分基础律动 Eighth", 0.0f, 0,
      "8000000080000000", "0000800000008000", "7050705070507050", "0000000000000000", "0000000000000000", "0000000000000000", "0000000000000000" },
    { "十六分练习 16th", 0.0f, 0,
      "8000000080000000", "0000800000008000", "7454745474547454", "0000000000000000", "0000000000000000", "0000000000000000", "0000000000000000" },
    { "反拍切分训练 Offbeat", 0.0f, 0,
      "8000007000800000", "0000800000008000", "0070007000700070", "0000000000000000", "0000000000000000", "0000000000000000", "0000000000000000" },
};

const DrumStyleInfo kStyles[] = {
    { "流行 Pop",       kPopPatterns,        5 },
    { "摇滚 Rock",      kRockPatterns,       5 },
    { "放克 Funk",      kFunkPatterns,       5 },
    { "爵士 Jazz",      kJazzPatterns,       5 },
    { "布鲁斯 Blues",   kBluesPatterns,      4 },
    { "雷鬼 Reggae",    kReggaePatterns,     4 },
    { "拉丁 Latin",     kLatinPatterns,      4 },
    { "嘻哈 Hip-Hop",   kHipHopPatterns,     5 },
    { "电子 Electro",   kElectronicPatterns, 5 },
    { "金属 Metal",     kMetalPatterns,      4 },
    { "民谣 Folk",      kFolkPatterns,       4 },
    { "基础练习",       kPracticePatterns,   4 },
};

const char* kKitNames[DrumMachine::kNumKits] = {
    "GSCW 温暖真鼓",
    "GMRock 摇滚真鼓",
    "TR-909 电子鼓"
};

constexpr float kPieceGain[DrumMachine::NumPieces] = {
    1.05f, // Kick
    0.96f, // Snare
    0.62f, // ClosedHat
    0.68f, // OpenHat
    0.65f, // Ride
    0.60f, // Crash
    0.82f  // Perc
};

} // namespace

// ============================================================================
// DrumMachine Implementation
// ============================================================================

DrumMachine::DrumMachine() = default;

const char* DrumMachine::getKitNameUtf8(int kitIndex)
{
    int idx = juce::jlimit(0, kNumKits - 1, kitIndex);
    return kKitNames[idx];
}

int DrumMachine::getNumStyles()
{
    return static_cast<int>(sizeof(kStyles) / sizeof(kStyles[0]));
}

const DrumStyleInfo& DrumMachine::getStyle(int styleIndex)
{
    int idx = juce::jlimit(0, getNumStyles() - 1, styleIndex);
    return kStyles[idx];
}

DrumMachine::SampleBuffer DrumMachine::decodeOgg(const void* data, int sizeBytes)
{
    SampleBuffer out;
    if (!data || sizeBytes <= 0)
        return out;

    juce::OggVorbisAudioFormat oggFormat;
    auto memStream = std::make_unique<juce::MemoryInputStream>(data, static_cast<size_t>(sizeBytes), false);
    std::unique_ptr<juce::AudioFormatReader> reader(oggFormat.createReaderFor(memStream.release(), true));
    if (!reader || reader->lengthInSamples <= 0)
        return out;

    const int numSamples = static_cast<int>(std::min<juce::int64>(reader->lengthInSamples, 44100 * 4));
    const int numChannels = static_cast<int>(reader->numChannels);
    juce::AudioBuffer<float> tmp(juce::jmax(2, numChannels), numSamples);
    tmp.clear();
    reader->read(&tmp, 0, numSamples, 0, true, numChannels > 1);

    out.sampleRate = (reader->sampleRate > 8000.0) ? reader->sampleRate : 44100.0;
    out.left.assign(tmp.getReadPointer(0), tmp.getReadPointer(0) + numSamples);
    if (numChannels > 1)
        out.right.assign(tmp.getReadPointer(1), tmp.getReadPointer(1) + numSamples);
    else
        out.right = out.left;

    return out;
}

void DrumMachine::ensureSamplesLoaded()
{
    if (samplesLoaded_)
        return;

    // Kit 0: #3 GSCW Kit 1 (Yamaha 16x16 + CustomWorks 6x13 + Sabian AAX/RobMor)
    kits_[0][Kick][0]      = decodeOgg(JamBarDrums::gscw_kick_s_ogg,  JamBarDrums::gscw_kick_s_oggSize);
    kits_[0][Kick][1]      = decodeOgg(JamBarDrums::gscw_kick_h_ogg,  JamBarDrums::gscw_kick_h_oggSize);
    kits_[0][Snare][0]     = decodeOgg(JamBarDrums::gscw_snare_s_ogg, JamBarDrums::gscw_snare_s_oggSize);
    kits_[0][Snare][1]     = decodeOgg(JamBarDrums::gscw_snare_h_ogg, JamBarDrums::gscw_snare_h_oggSize);
    kits_[0][ClosedHat][0] = decodeOgg(JamBarDrums::gscw_chh_s_ogg,   JamBarDrums::gscw_chh_s_oggSize);
    kits_[0][ClosedHat][1] = decodeOgg(JamBarDrums::gscw_chh_h_ogg,   JamBarDrums::gscw_chh_h_oggSize);
    kits_[0][OpenHat][0]   = decodeOgg(JamBarDrums::gscw_ohh_ogg,     JamBarDrums::gscw_ohh_oggSize);
    kits_[0][OpenHat][1]   = kits_[0][OpenHat][0];
    kits_[0][Ride][0]      = decodeOgg(JamBarDrums::gscw_ride_s_ogg,  JamBarDrums::gscw_ride_s_oggSize);
    kits_[0][Ride][1]      = decodeOgg(JamBarDrums::gscw_ride_h_ogg,  JamBarDrums::gscw_ride_h_oggSize);
    kits_[0][Crash][0]     = decodeOgg(JamBarDrums::gscw_crash_ogg,   JamBarDrums::gscw_crash_oggSize);
    kits_[0][Crash][1]     = kits_[0][Crash][0];
    kits_[0][Perc][0]      = decodeOgg(JamBarDrums::gscw_perc_ogg,    JamBarDrums::gscw_perc_oggSize);
    kits_[0][Perc][1]      = kits_[0][Perc][0];

    // Kit 1: #5 Hydrogen GMRockKit (GPL-2.0)
    kits_[1][Kick][0]      = decodeOgg(JamBarDrums::gm_kick_s_ogg,  JamBarDrums::gm_kick_s_oggSize);
    kits_[1][Kick][1]      = decodeOgg(JamBarDrums::gm_kick_h_ogg,  JamBarDrums::gm_kick_h_oggSize);
    kits_[1][Snare][0]     = decodeOgg(JamBarDrums::gm_snare_s_ogg, JamBarDrums::gm_snare_s_oggSize);
    kits_[1][Snare][1]     = decodeOgg(JamBarDrums::gm_snare_h_ogg, JamBarDrums::gm_snare_h_oggSize);
    kits_[1][ClosedHat][0] = decodeOgg(JamBarDrums::gm_chh_s_ogg,   JamBarDrums::gm_chh_s_oggSize);
    kits_[1][ClosedHat][1] = decodeOgg(JamBarDrums::gm_chh_h_ogg,   JamBarDrums::gm_chh_h_oggSize);
    kits_[1][OpenHat][0]   = decodeOgg(JamBarDrums::gm_ohh_ogg,     JamBarDrums::gm_ohh_oggSize);
    kits_[1][OpenHat][1]   = kits_[1][OpenHat][0];
    kits_[1][Ride][0]      = decodeOgg(JamBarDrums::gm_ride_s_ogg,  JamBarDrums::gm_ride_s_oggSize);
    kits_[1][Ride][1]      = decodeOgg(JamBarDrums::gm_ride_h_ogg,  JamBarDrums::gm_ride_h_oggSize);
    kits_[1][Crash][0]     = decodeOgg(JamBarDrums::gm_crash_ogg,   JamBarDrums::gm_crash_oggSize);
    kits_[1][Crash][1]     = kits_[1][Crash][0];
    kits_[1][Perc][0]      = decodeOgg(JamBarDrums::gm_perc_ogg,    JamBarDrums::gm_perc_oggSize);
    kits_[1][Perc][1]      = kits_[1][Perc][0];

    // Kit 2: #9 Roland TR-909
    kits_[2][Kick][0]      = decodeOgg(JamBarDrums::tr909_kick_ogg,  JamBarDrums::tr909_kick_oggSize);
    kits_[2][Kick][1]      = kits_[2][Kick][0];
    kits_[2][Snare][0]     = decodeOgg(JamBarDrums::tr909_snare_ogg, JamBarDrums::tr909_snare_oggSize);
    kits_[2][Snare][1]     = kits_[2][Snare][0];
    kits_[2][ClosedHat][0] = decodeOgg(JamBarDrums::tr909_chh_ogg,   JamBarDrums::tr909_chh_oggSize);
    kits_[2][ClosedHat][1] = kits_[2][ClosedHat][0];
    kits_[2][OpenHat][0]   = decodeOgg(JamBarDrums::tr909_ohh_ogg,   JamBarDrums::tr909_ohh_oggSize);
    kits_[2][OpenHat][1]   = kits_[2][OpenHat][0];
    kits_[2][Ride][0]      = decodeOgg(JamBarDrums::tr909_ride_ogg,  JamBarDrums::tr909_ride_oggSize);
    kits_[2][Ride][1]      = kits_[2][Ride][0];
    kits_[2][Crash][0]     = decodeOgg(JamBarDrums::tr909_crash_ogg, JamBarDrums::tr909_crash_oggSize);
    kits_[2][Crash][1]     = kits_[2][Crash][0];
    kits_[2][Perc][0]      = decodeOgg(JamBarDrums::tr909_perc_ogg,  JamBarDrums::tr909_perc_oggSize);
    kits_[2][Perc][1]      = kits_[2][Perc][0];

    samplesLoaded_ = true;
}

void DrumMachine::prepare(double sampleRate)
{
    sampleRate_ = (sampleRate > 8000.0) ? sampleRate : 48000.0;
    ensureSamplesLoaded();
    reset();
}

void DrumMachine::reset()
{
    for (auto& v : voices_)
        v = SampleVoice{};
    wasPlaying_ = false;
    currentStep_ = 0;
    barCount_ = 0;
    samplesUntilNextStep_ = 0.0;
}

void DrumMachine::setPlaying(bool play)
{
    playing_.store(play, std::memory_order_release);
}

void DrumMachine::setKitIndex(int kitIdx)
{
    kitIndex_.store(juce::jlimit(0, kNumKits - 1, kitIdx), std::memory_order_relaxed);
}

void DrumMachine::setStyleIndex(int styleIdx)
{
    int clampedStyle = juce::jlimit(0, getNumStyles() - 1, styleIdx);
    styleIndex_.store(clampedStyle, std::memory_order_relaxed);
    int curPat = patternIndex_.load(std::memory_order_relaxed);
    int maxPat = kStyles[clampedStyle].numPatterns - 1;
    if (curPat > maxPat)
        patternIndex_.store(0, std::memory_order_relaxed);
}

void DrumMachine::setPatternIndex(int patternIdx)
{
    int sIdx = styleIndex_.load(std::memory_order_relaxed);
    int maxPat = kStyles[juce::jlimit(0, getNumStyles() - 1, sIdx)].numPatterns - 1;
    patternIndex_.store(juce::jlimit(0, maxPat, patternIdx), std::memory_order_relaxed);
}

float DrumMachine::nextRand01()
{
    rngState_ ^= rngState_ << 13;
    rngState_ ^= rngState_ >> 17;
    rngState_ ^= rngState_ << 5;
    return static_cast<float>(rngState_ & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
}

void DrumMachine::triggerPiece(int kitIdx, Piece piece, float velocity)
{
    if (!samplesLoaded_)
        return;

    int k = juce::jlimit(0, kNumKits - 1, kitIdx);
    int layer = (velocity > 0.56f) ? 1 : 0;
    const SampleBuffer& buf = kits_[k][piece][layer];
    if (!buf.valid())
        return;

    // Authentic Hi-Hat choke: closed hat smoothly cuts active open hi-hat voices
    if (piece == ClosedHat)
    {
        for (auto& v : voices_)
            if (v.active && v.piece == OpenHat)
                v.choking = true;
    }

    // Find free voice slot, or steal the oldest / furthest-along voice
    SampleVoice* slot = nullptr;
    double bestPos = -1.0;
    for (auto& v : voices_)
    {
        if (!v.active)
        {
            slot = &v;
            break;
        }
        if (v.pos > bestPos)
        {
            bestPos = v.pos;
            slot = &v;
        }
    }
    if (!slot)
        return;

    // Subtle pitch humanization (+/- 0.5%) keeps repeated hits sounding natural
    double humanPitch = 0.995 + static_cast<double>(nextRand01()) * 0.010;
    slot->active = true;
    slot->buf = &buf;
    slot->piece = piece;
    slot->pos = 0.0;
    slot->inc = (buf.sampleRate / sampleRate_) * humanPitch;
    // Slight velocity loudness curve: soft hits stay musical & audible
    float shapedVel = 0.30f + 0.70f * velocity;
    slot->gain = shapedVel * kPieceGain[piece];
    slot->choking = false;
    slot->chokeEnv = 1.0f;
}

void DrumMachine::triggerStep(int stepIdx, int barIdx, int kitIdx, const DrumPatternInfo& pat)
{
    auto velOf = [&](const char* grid) -> float {
        if (!grid) return 0.0f;
        char c = grid[stepIdx & 15];
        if (c <= '0' || c > '9') return 0.0f;
        return static_cast<float>(c - '0') / 9.0f;
    };

    if (float v = velOf(pat.kick); v > 0.0f)  triggerPiece(kitIdx, Kick, v);
    if (float v = velOf(pat.snare); v > 0.0f) triggerPiece(kitIdx, Snare, v);
    if (float v = velOf(pat.chh); v > 0.0f)   triggerPiece(kitIdx, ClosedHat, v);
    if (float v = velOf(pat.ohh); v > 0.0f)   triggerPiece(kitIdx, OpenHat, v);
    if (float v = velOf(pat.ride); v > 0.0f)  triggerPiece(kitIdx, Ride, v);
    if (float v = velOf(pat.perc); v > 0.0f)  triggerPiece(kitIdx, Perc, v);

    if (pat.crashEveryBars > 0 && (barIdx % pat.crashEveryBars == 0))
    {
        if (float v = velOf(pat.crash); v > 0.0f)
            triggerPiece(kitIdx, Crash, v);
    }
}

void DrumMachine::processAdd(float* outL, float* outR, int numSamples,
                             float activeBpm, double beatPos)
{
    if (!outL || !outR || numSamples <= 0 || !samplesLoaded_)
        return;

    const bool playing = playing_.load(std::memory_order_acquire);
    const float vol = volume_.load(std::memory_order_relaxed);
    const int kitIdx = kitIndex_.load(std::memory_order_relaxed);

    // Check if any voice tail is still ringing out after stop
    bool anyActive = false;
    for (const auto& v : voices_)
        if (v.active) { anyActive = true; break; }

    if (!playing && !anyActive)
    {
        wasPlaying_ = false;
        return;
    }

    const int sIdx = juce::jlimit(0, getNumStyles() - 1, styleIndex_.load(std::memory_order_relaxed));
    const auto& style = kStyles[sIdx];
    const int pIdx = juce::jlimit(0, style.numPatterns - 1, patternIndex_.load(std::memory_order_relaxed));
    const auto& pat = style.patterns[pIdx];

    const double bpm = juce::jlimit(40.0, 300.0, static_cast<double>(activeBpm > 1.0f ? activeBpm : 120.0f));
    const double baseStepSamples = (60.0 / bpm) * sampleRate_ * 0.25; // 16th note

    // On start edge: phase-align to current beat position so Beat 1 locks tightly!
    if (playing && !wasPlaying_)
    {
        wasPlaying_ = true;
        barCount_ = 0;
        if (beatPos >= 0.0)
        {
            double stepPosInBar = std::fmod(beatPos * 4.0, 16.0);
            if (stepPosInBar < 0.0) stepPosInBar += 16.0;
            int stepFloor = static_cast<int>(stepPosInBar) & 15;
            double frac = stepPosInBar - std::floor(stepPosInBar);
            if (frac < 0.12)
            {
                currentStep_ = stepFloor;
                samplesUntilNextStep_ = 0.0;
            }
            else
            {
                currentStep_ = (stepFloor + 1) & 15;
                samplesUntilNextStep_ = (1.0 - frac) * baseStepSamples;
            }
        }
        else
        {
            currentStep_ = 0;
            samplesUntilNextStep_ = 0.0;
        }
    }
    else if (!playing)
    {
        wasPlaying_ = false;
    }

    const float chokeCoeff = static_cast<float>(std::exp(-1.0 / (0.012 * sampleRate_)));

    for (int i = 0; i < numSamples; ++i)
    {
        if (playing)
        {
            if (samplesUntilNextStep_ <= 0.0)
            {
                triggerStep(currentStep_, barCount_, kitIdx, pat);
                const bool isEven = (currentStep_ % 2 == 0);
                const double swingMul = isEven ? (1.0 + pat.swing) : (1.0 - pat.swing);
                samplesUntilNextStep_ += juce::jmax(32.0, baseStepSamples * swingMul);
                currentStep_ = (currentStep_ + 1) & 15;
                if (currentStep_ == 0)
                    ++barCount_;
            }
            samplesUntilNextStep_ -= 1.0;
        }

        float mixL = 0.0f;
        float mixR = 0.0f;

        for (auto& v : voices_)
        {
            if (!v.active || !v.buf)
                continue;

            const size_t idx0 = static_cast<size_t>(v.pos);
            const size_t maxIdx = v.buf->left.size();
            if (idx0 >= maxIdx)
            {
                v.active = false;
                continue;
            }

            const size_t idx1 = (idx0 + 1 < maxIdx) ? (idx0 + 1) : idx0;
            const float frac = static_cast<float>(v.pos - static_cast<double>(idx0));
            const float sL = v.buf->left[idx0] + (v.buf->left[idx1] - v.buf->left[idx0]) * frac;
            const float sR = v.buf->right[idx0] + (v.buf->right[idx1] - v.buf->right[idx0]) * frac;

            float g = v.gain;
            if (v.choking)
            {
                v.chokeEnv *= chokeCoeff;
                g *= v.chokeEnv;
                if (v.chokeEnv < 0.001f)
                    v.active = false;
            }

            mixL += sL * g;
            mixR += sR * g;
            v.pos += v.inc;
        }

        // Gentle studio soft-clipper preserves punchy drum transients without digital clipping
        mixL = std::tanh(mixL * 0.92f) * vol;
        mixR = std::tanh(mixR * 0.92f) * vol;

        outL[i] += mixL;
        outR[i] += mixR;
    }
}

} // namespace jambar
