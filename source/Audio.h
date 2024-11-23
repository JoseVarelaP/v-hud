#pragma once
#define NUM_MUSIC_MENU_LOOPS 7
enum eChunks {
    CHUNK_MENU_BACK,
    CHUNK_MENU_SCROLL,
    CHUNK_MENU_SELECT,
    CHUNK_MENU_MAP_MOVE,
    CHUNK_WHEEL_BACKGROUND,
    CHUNK_WHEEL_OPEN_CLOSE,
    CHUNK_WHEEL_MOVE,
    CHUNK_TD_LOADING_MUSIC,
    CHUNK_STATS_BACKGROUND,
    CHUNK_WASTED_BUSTED,
    CHUNK_SCREEN_PULSE1,
    // Failed screen
    CHUNK_FAIL_BG_SFX,
    CHUNK_FAIL_THUD_SFX,
    // Menu background music
    CHUNK_MENU_MUSIC_LOOP1,
    CHUNK_MENU_MUSIC_LOOP2,
    CHUNK_MENU_MUSIC_LOOP3,
    CHUNK_MENU_MUSIC_LOOP4,
    CHUNK_MENU_MUSIC_LOOP5,
    CHUNK_MENU_MUSIC_LOOP6,
    CHUNK_MENU_MUSIC_LOOP7,
    // Phone Sound effects
    PHONE_SFX_UP,
    PHONE_SFX_DOWN,
    PHONE_SFX_PICK_OPTION,
    PHONE_SFX_SCROLL_OPTION,
    NUM_CHUNKS
};

class CAudio {
public:
    bool bInitialised;   
    unsigned long Chunks[NUM_CHUNKS];
    float fChunksVolume;
    bool bLoop;
    bool bResetMainVolume;
    float fPreviousChunkVolume[NUM_CHUNKS];

public:
    void Init();
    void Shutdown();
    void Update();

    unsigned long LoadChunkFile(const char* path, const char* name);
    void PlayChunk(int chunk, float volume);
    float GetVolumeForChunk(int chunk);
    void SetVolumeForChunk(int chunk, float volume);
    void StopChunk(int chunk);
    void SetChunksMasterVolume(char vol);
    void SetLoop(bool on);
};

extern CAudio Audio;