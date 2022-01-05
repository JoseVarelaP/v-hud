#include "VHud.h"
#include "Audio.h"
#include "MenuNew.h"
#include "Utility.h"

#include "bass.h"

using namespace plugin;

CAudio Audio;

const char* AudioFileFormat = ".wav";

const char* ChunksFileNames[] = {
    "menu_back",
    "menu_scroll",
    "menu_select",
    "menu_map_move",
    "wheel_background",
    "wheel_open_close",
    "wheel_move",
    "td_loading_music",
    "stats_background",
    "wasted_busted",
    "screen_pulse1"
};

CAudio::CAudio() {

}

void CAudio::Init() {
    if (bInitialised)
        return;

    if (!BASS_Init(-1, 44100, 0, RsGlobal.ps->window, NULL)) {
        printf("[BASS] Can't initialize device. \n");

        if (BASS_ErrorGetCode() != BASS_ERROR_ALREADY)
            return;
        else
            printf("[BASS] Device has already been initialized. \n");
    }
    else {
        printf("[BASS] Initialized \n");
    }

    for (int i = 0; i < NUM_CHUNKS; i++) {
        Chunks[i] = LoadChunkFile(PLUGIN_PATH("VHud\\audio\\frontend"), ChunksFileNames[i]);   
    }

    bInitialised = true;
}

void CAudio::Shutdown() {
    if (!bInitialised)
        return;

    for (int i = 0; i < NUM_CHUNKS; i++) {
        BASS_SampleFree(Chunks[i]);
    }

    BASS_Free();

    printf("[BASS] Shutdown. \n");

    bInitialised = false;
}

void CAudio::Update() {
    if (!bInitialised)
        return;

    MenuNew.DoFadeTune();
}

void CAudio::SetChunksMasterVolume(char vol) {
    fChunksVolume = (vol / 64.0f);
}

void CAudio::SetLoop(bool on) {
    loop = on;
}

unsigned long CAudio::LoadChunkFile(const char* path, const char* name) {
    char file[512];

    sprintf(file, "%s\\%s.wav", path, name);

    return BASS_SampleLoad(FALSE, file, 0, 0, 3, 0);
}

void CAudio::PlayChunk(int chunk, float volume) {
    if (!bInitialised)
        return;

    chunk = clamp(chunk, 0, NUM_CHUNKS - 1);

    auto c = BASS_SampleGetChannel(Chunks[chunk], NULL);

    if (loop) {
        BASS_ChannelFlags(c, BASS_SAMPLE_LOOP, BASS_SAMPLE_LOOP);
    }
    else {
        BASS_ChannelFlags(c, NULL, BASS_SAMPLE_LOOP);
    }

    if (volume < 0.0f) {
        BASS_ChannelSetAttribute(c, BASS_ATTRIB_VOL, ABS(volume));
    }
    else {
        volume = clamp(volume, 0.0f, 1.0f);
        BASS_ChannelSetAttribute(c, BASS_ATTRIB_VOL, volume * (fChunksVolume * 0.5f));
    }
    BASS_ChannelPlay(c, FALSE);
}

void CAudio::SetVolumeForChunk(int chunk, float volume) {
    if (!bInitialised)
        return;

    chunk = clamp(chunk, 0, NUM_CHUNKS - 1);
    volume = clamp(volume, 0.0f, 1.0f);

    BASS_SAMPLE info;
    BASS_SampleGetInfo(Chunks[chunk], &info);
    if (HCHANNEL* c = (HCHANNEL*)malloc(info.max * sizeof(HCHANNEL))) {
        for (int i = 0; i < BASS_SampleGetChannels(Chunks[chunk], c); i++) {
            BASS_ChannelSetAttribute(c[i], BASS_ATTRIB_VOL, volume * (fChunksVolume * 0.5f));
        }
    }
}

void CAudio::StopChunk(int chunk) {
    if (!bInitialised)
        return;

    chunk = clamp(chunk, 0, NUM_CHUNKS - 1);

    BASS_SAMPLE info;
    BASS_SampleGetInfo(Chunks[chunk], &info);
    if (HCHANNEL* c = (HCHANNEL*)malloc(info.max * sizeof(HCHANNEL))) {
        for (int i = 0; i < BASS_SampleGetChannels(Chunks[chunk], c); i++) {
            BASS_ChannelSetPosition(c[i], 0, BASS_POS_BYTE);
            BASS_SampleStop(c[i]);
            BASS_ChannelStop(c[i]);
        }
    }
}