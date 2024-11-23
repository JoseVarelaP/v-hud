#pragma once

enum eOverlayEffect {
    EFFECT_NONE,
    EFFECT_BLUR_COLOR,
    EFFECT_BLUR,
    EFFECT_BLACK_N_WHITE,
    EFFECT_BLACK_N_WHITE_KO,
    EFFECT_LENS_DISTORTION,
};

class COverlayLayer {
private:
    static eOverlayEffect CurrentEffect;
    static float fShaderConstant[4];
    static bool bInitialised;

public:
    static void Init();
    static void Shutdown();
    static void UpdateFrameBuffer();
    static void RenderEffects();
    static void RenderBlackAndWhite(CRect& rect, CRGBA& c);

public:
    static void SetEffect(eOverlayEffect e) { CurrentEffect = e; }

    static void SetConstantF(float r, float g, float b, float a) {
        fShaderConstant[0] = r;
        fShaderConstant[1] = g;
        fShaderConstant[2] = b;
        fShaderConstant[3] = a;
    }

    static eOverlayEffect& GetCurrentEffect() { return CurrentEffect; }
    static float m_fShaderAlpha;
};

extern void* im2dPixelShader;
extern void _rwSetPixelShader(void* ps);
