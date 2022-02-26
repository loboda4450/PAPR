#pragma once

#include "CommonDef.h"
#include "xPic.h"
#include "xThreadPool.h"

//===============================================================================================================================================================================================================

class xCoeffs {
public:
    static constexpr inline flt32 RGB2YCbCr_F32[3][3] = //BT709
            {
                    {(flt32) (0.21260),  (flt32) (0.71520),  (flt32) (0.07220)}, //Y_R, Y_G, Y_B
                    {(flt32) (-0.11457), (flt32) (-0.38543), (flt32) (0.50000)}, //U_R, U_G ,U_B
                    {(flt32) (0.50000),  (flt32) (-0.45415), (flt32) (-0.04585)}, //V_R, V_B, V_G
            };
    static constexpr inline flt32 YCbCr2RGB_F32[3][3] =  //BT709
            {
                    {(flt32) (1.0), (flt32) (0),        (flt32) (1.5748)}, //R_Y, R_U, R_V
                    {(flt32) (1.0), (flt32) (-0.18732), (flt32) (-0.46812)}, //G_Y, G_U, G_V
                    {(flt32) (1.0), (flt32) (1.8556),   (flt32) (0)}, //B_Y, B_U, B_V
            };
    static constexpr inline flt32 SEPIA_F32[3][3] = //SEPIA
            {
                    {(flt32) (0.393), (flt32) (0.769), (flt32) (0.189)},
                    {(flt32) (0.349), (flt32) (0.686), (flt32) (0.168)},
                    {(flt32) (0.272), (flt32) (0.534), (flt32) (0.131)},
            };

    static constexpr inline int32 RGB2YCbCr_I32[3][3] = { // gotta right shift it by 16 in the end
            {
                    (int32) (13933),
                    (int32) (46871),
                    (int32) (4732),
            },
            {
                    (int32) (-7508),
                    (int32) (-25260),
                    (int32) (32768),
            },
            {
                    (int32) (32768),
                    (int32) (-29763),
                    (int32) (-3005),
            },

    };

    static constexpr inline int32 YCbCr2RGB_I32[3][3] = { // gotta right shift it by 16 in the end
            {
                    (int32) (65536),
                    (int32) (0),
                    (int32) (103206),
            },
            {
                    (int32) (65536),
                    (int32) (-12276),
                    (int32) (-30679),
            },
            {
                    (int32) (65536),
                    (int32) (121609),
                    (int32) (0),
            },

    };

    static constexpr inline int32 SEPIA_I32[3][3] = { // gotta right shift it by 16 in the end
            {
                    (int32) (25756),
                    (int32) (50397),
                    (int32) (12386),
            },
            {
                    (int32) (22872),
                    (int32) (44958),
                    (int32) (11010),
            },
            {
                    (int32) (17826),
                    (int32) (34996),
                    (int32) (8585),
            },

    };
};

//===============================================================================================================================================================================================================

class xSepiaBase : public xCoeffs {
public:
    enum class eMode : int32 {
        SERIAL = -1,
        PIXELS = 0,
        ROWS = 1,
        TILES4 = 2,
        TILES8 = 3,
        TILES16 = 4,
        TILES32 = 5,
        TILES64 = 6,
        TILES128 = 7,
        TILES256 = 8,
        TILES512 = 9,
        TILES1024 = 10,
    };

    constexpr static const xSepiaBase::eMode All_eModes[1] = {/*xSepiaBase::eMode::SERIAL,
                                                               */xSepiaBase::eMode::ROWS};/*
                                                               xSepiaBase::eMode::TILES4,
                                                               xSepiaBase::eMode::TILES8,
                                                               xSepiaBase::eMode::TILES16,
                                                               xSepiaBase::eMode::TILES32,
                                                               xSepiaBase::eMode::TILES64,
                                                               xSepiaBase::eMode::TILES128,
                                                               xSepiaBase::eMode::TILES256,
                                                               xSepiaBase::eMode::TILES512,
                                                               xSepiaBase::eMode::TILES1024,
                                                               xSepiaBase::eMode::PIXELS};*/

protected:
    xThreadPool *m_ThreadPool = nullptr;
    int32 m_VerboseLevel = 0;
    const int32 m_NumComponents = 3;

public:
    void setThreadPool(xThreadPool *ThreadPool) { m_ThreadPool = ThreadPool; }

    void setVerboseLevel(int32 VerboseLevel) { m_VerboseLevel = VerboseLevel; }
};

//===============================================================================================================================================================================================================

class xSepia_STD : public xSepiaBase {
public:
    void testCopyContent(xPic &Dst, const xPic &Src);

    void testYUVtoRGBtoYUV_FLT(xPic &Dst, const xPic &Src, eMode ProcMode);

    void applySepiaEffect_FLT(xPic &Dst, const xPic &Src, eMode ProcMode);

    void testYUVtoRGBtoYUV_INT(xPic &Dst, const xPic &Src, eMode ProcMode);

    void applySepiaEffect_INT(xPic &Dst, const xPic &Src, eMode ProcMode);

    void testYUVtoRGBtoYUV_SSE(xPic &Dst, const xPic &Src, eMode ProcMode);

    void applySepiaEffect_SSE(xPic &Dst, const xPic &Src, eMode ProcMode);

    int32 getTileSize(const int32 &maxPos, const int32 &TileSize, const int32 &curPos) {
        int32 newTileSize = TileSize;
        while (curPos + newTileSize > maxPos and newTileSize >= 1) {
            newTileSize >>= 1;
        }
        return newTileSize;
    }

    void
    createTasks(const xPic &Src, xPic &Dst, const int &Height, const int &Width, const int &TileSize, const bool &Mode);

    void createTasks_butINT(const xPic &Src, xPic &Dst, const int &Height, const int &Width, const int &TileSize,
                            const bool &Mode);

    void createTasks_butSSE(const xPic &Src, xPic &Dst, const int &Height, const int &Width, const int &TileSize,
                            const bool &Mode);

protected:
    static void xTestYUVtoRGBtoYUV_FLT(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX);

    static void xApplySepiaEffect_FLT(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX);

    static void xTestYUVtoRGBtoYUV_INT(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX);

    static void xApplySepiaEffect_INT(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX);

    static void xTestYUVtoRGBtoYUV_SSE(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX);

    static void xApplySepiaEffect_SSE(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX);
};

//===============================================================================================================================================================================================================