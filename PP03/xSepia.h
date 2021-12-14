#pragma once

#include "CommonDef.h"
#include "xPic.h"

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

    // all these under are just corresponding to upper ones but multiplied by size of int32 and rounded to be integer, not float or double
    static constexpr inline int32 RGB2YCbCr_I32[3][3] = { // gotta right shift it by 16 in the end
            {13933, 46871, 4732},
            {-7508, -25260, 32768},
            {32768, -29763, -3005}
    };

    static constexpr inline int32 YCbCr2RGB_I32[3][3] = { // gotta right shift it by 16 in the end
            {65536, 0, 103206},
            {65536, -12276, -30679},
            {65536, 121609, 0}
    };

    static constexpr inline int32 SEPIA_I32[3][3] = { // gotta right shift it by 16 in the end
            {25756, 50397, 12386},
            {22872, 44958, 11010},
            {17826, 34996, 8585}
    };
};

//===============================================================================================================================================================================================================

class xSepiaBase : public xCoeffs {
protected:
    int32 m_NumThreads = 0;
    int32 m_VerboseLevel = 0;
    int32 m_NumComponents = 3;

public:
    void setNumThreads(int32 NumThreads) { m_NumThreads = NumThreads; }

    void setVerboseLevel(int32 VerboseLevel) { m_VerboseLevel = VerboseLevel; }
};

//===============================================================================================================================================================================================================

class xSepia_STD : public xSepiaBase {
public:
    void testCopyContent(xPic &Dst, const xPic &Src);

    void testYUVtoRGBtoYUV_FLT(xPic &Dst, const xPic &Src);

    void testYUVtoRGBtoYUV_INT(xPic &Dst, const xPic &Src);

    void applySepiaEffect_FLT(xPic &Dst, const xPic &Src);

    void applySepiaEffect_INT(xPic &Dst, const xPic &Src);
};

//===============================================================================================================================================================================================================

