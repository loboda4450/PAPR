#include "xSepia.h"

//===============================================================================================================================================================================================================

void xSepia_STD::testCopyContent(xPic &Dst, const xPic &Src) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());

    const int32 Width = Dst.getWidth();
    const int32 Height = Dst.getHeight();
    uint16 *restrict DstPtrLm = Dst.getAddr(0);
    uint16 *restrict DstPtrCb = Dst.getAddr(1);
    uint16 *restrict DstPtrCr = Dst.getAddr(2);
    const uint16 *SrcPtrLm = Src.getAddr(0);
    const uint16 *SrcPtrCb = Src.getAddr(1);
    const uint16 *SrcPtrCr = Src.getAddr(2);
    const int32 DstStride = Dst.getStride();
    const int32 SrcStride = Src.getStride();

    //WARNING - for demonstration purposes only - it isn't the fastest way to copy data
    for (int32 y = 0; y < Height; y++) {
        for (int32 x = 0; x < Width; x++) {
            DstPtrLm[x] = SrcPtrLm[x];
            DstPtrCb[x] = SrcPtrCb[x];
            DstPtrCr[x] = SrcPtrCr[x];
        }
        DstPtrLm += DstStride;
        DstPtrCb += DstStride;
        DstPtrCr += DstStride;
        SrcPtrLm += SrcStride;
        SrcPtrCb += SrcStride;
        SrcPtrCr += SrcStride;
    }
}

void xSepia_STD::testYUVtoRGBtoYUV_FLT(xPic &Dst, const xPic &Src) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());

    const int32 Width = Dst.getWidth();
    const int32 Height = Dst.getHeight();
    uint16 *restrict DstPtrLm = Dst.getAddr(0);
    uint16 *restrict DstPtrCb = Dst.getAddr(1);
    uint16 *restrict DstPtrCr = Dst.getAddr(2);
    const uint16 *SrcPtrLm = Src.getAddr(0);
    const uint16 *SrcPtrCb = Src.getAddr(1);
    const uint16 *SrcPtrCr = Src.getAddr(2);
    const int32 DstStride = Dst.getStride();
    const int32 SrcStride = Src.getStride();

    const flt32 SrcMidValue = flt32(1 << (Src.getBitDepth() - 1));
    const flt32 DstMidValue = flt32(1 << (Dst.getBitDepth() - 1));
    const flt32 DstMaxValue = flt32((1 << Dst.getBitDepth()) - 1);

    for (int32 y = 0; y < Height; y++) {
        for (int32 x = 0; x < Width; x++) {
            //load
            const uint16V4 SrcPixelYUV_U16 = {SrcPtrLm[x], SrcPtrCb[x], SrcPtrCr[x], 0};

            //change data format (and remove chroma offset)
            const flt32V4 SrcPixelYUV_F32 = ((flt32V4) SrcPixelYUV_U16 - flt32V4(0, SrcMidValue, SrcMidValue, 0));

            //convert YCbCr --> RGB (BT.709)
            const flt32V4 SrcPixelRGB_F32 =
                    {
                            SrcPixelYUV_F32[0] + SrcPixelYUV_F32[2] * YCbCr2RGB_F32[0][2],
                            SrcPixelYUV_F32[0] + SrcPixelYUV_F32[1] * YCbCr2RGB_F32[1][1] +
                            SrcPixelYUV_F32[2] * YCbCr2RGB_F32[1][2],
                            SrcPixelYUV_F32[0] + SrcPixelYUV_F32[1] * YCbCr2RGB_F32[2][1],
                            0
                    };

            //copy RGB --> RGB
            const flt32V4 DstPixelRGB_F32 = SrcPixelRGB_F32;

            //convert RGB --> YCbCr (BT.709)
            const flt32V4 DstPixelYUV_F32 =
                    {
                            DstPixelRGB_F32[0] * RGB2YCbCr_F32[0][0] + DstPixelRGB_F32[1] * RGB2YCbCr_F32[0][1] +
                            DstPixelRGB_F32[2] * RGB2YCbCr_F32[0][2],
                            DstPixelRGB_F32[0] * RGB2YCbCr_F32[1][0] + DstPixelRGB_F32[1] * RGB2YCbCr_F32[1][1] +
                            DstPixelRGB_F32[2] * RGB2YCbCr_F32[1][2],
                            DstPixelRGB_F32[0] * RGB2YCbCr_F32[2][0] + DstPixelRGB_F32[1] * RGB2YCbCr_F32[2][1] +
                            DstPixelRGB_F32[2] * RGB2YCbCr_F32[2][2],
                            0
                    };

            //change data format (and apply chroma offset) + clip to range 0-Max
            flt32V4 TmpPixelYUV_F32 = DstPixelYUV_F32 + flt32V4(0, DstMidValue, DstMidValue, 0);
            TmpPixelYUV_F32.clip(xMakeVec4<flt32>(0), xMakeVec4<flt32>(DstMaxValue));
            const uint16V4 DstPixelYUV_U16 = uint16V4(TmpPixelYUV_F32);

            //store
            DstPtrLm[x] = DstPixelYUV_U16[0];
            DstPtrCb[x] = DstPixelYUV_U16[1];
            DstPtrCr[x] = DstPixelYUV_U16[2];
        }
        DstPtrLm += DstStride;
        DstPtrCb += DstStride;
        DstPtrCr += DstStride;
        SrcPtrLm += SrcStride;
        SrcPtrCb += SrcStride;
        SrcPtrCr += SrcStride;
    }
}

void xSepia_STD::testYUVtoRGBtoYUV_INT(xPic &Dst, const xPic &Src) { // idk sth weird happens to colors..
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());
    const int32 Width = Dst.getWidth();
    const int32 Height = Dst.getHeight();
    uint16 *restrict DstPtrLm = Dst.getAddr(0);
    uint16 *restrict DstPtrCb = Dst.getAddr(1);
    uint16 *restrict DstPtrCr = Dst.getAddr(2);
    const uint16 *SrcPtrLm = Src.getAddr(0);
    const uint16 *SrcPtrCb = Src.getAddr(1);
    const uint16 *SrcPtrCr = Src.getAddr(2);
    const int32 DstStride = Dst.getStride();
    const int32 SrcStride = Src.getStride();

    const int32 SrcMidValue = int32(1 << (Src.getBitDepth() - 1));
    const int32 DstMidValue = int32(1 << (Dst.getBitDepth() - 1));
    const int32 DstMaxValue = int32((1 << Dst.getBitDepth()) - 1);

    for (int32 y = 0; y < Height; y++) {
        for (int32 x = 0; x < Width; x++) {
            //load
            const uint16V4 SrcPixelYUV_U16 = {SrcPtrLm[x], SrcPtrCb[x], SrcPtrCr[x], 0};

            //change data format (and remove chroma offset)
            const int32V4 SrcPixelYUV_I32 = ((int32V4) SrcPixelYUV_U16 - int32V4(0, SrcMidValue, SrcMidValue, 0));

            //convert YCbCr --> RGB (BT.709)
            const int32V4 SrcPixelRGB_I32 = {
                    SrcPixelYUV_I32[0] + SrcPixelYUV_I32[2] * YCbCr2RGB_I32[0][2],
                    SrcPixelYUV_I32[0] + SrcPixelYUV_I32[1] * YCbCr2RGB_I32[1][1] +
                    SrcPixelYUV_I32[2] * YCbCr2RGB_I32[1][2],
                    SrcPixelYUV_I32[0] + SrcPixelYUV_I32[1] * YCbCr2RGB_I32[2][1],
                    0
            };

            //copy RGB --> RGB
            const int32V4 DstPixelRGB_I32 = SrcPixelRGB_I32;

            //convert RGB --> YCbCr (BT.709)
            const int32V4 DstPixelYUV_I32 = {
                    (int32) (((DstPixelRGB_I32[0] * RGB2YCbCr_I32[0][0] + DstPixelRGB_I32[1] * RGB2YCbCr_I32[0][1] +
                              DstPixelRGB_I32[2] * RGB2YCbCr_I32[0][2]) + 1) >> 16),
                    (int32) (((DstPixelRGB_I32[0] * RGB2YCbCr_I32[1][0] + DstPixelRGB_I32[1] * RGB2YCbCr_I32[1][1] +
                              DstPixelRGB_I32[2] * RGB2YCbCr_I32[1][2]) + 1) >> 16),
                    (int32) (((DstPixelRGB_I32[0] * RGB2YCbCr_I32[2][0] + DstPixelRGB_I32[1] * RGB2YCbCr_I32[2][1] +
                              DstPixelRGB_I32[2] * RGB2YCbCr_I32[2][2]) + 1) >> 16),
                    0
            };

            //change data format (and apply chroma offset) + clip to range 0-Max
            int32V4 TmpPixelYUV_F32 = DstPixelYUV_I32 + int32V4 (0, (int32) (DstMidValue), (int32) (DstMidValue), 0);
            TmpPixelYUV_F32.clip(xMakeVec4<int32>(0), xMakeVec4<int32>((int32) (DstMaxValue)));
            const uint16V4 DstPixelYUV_U16 = uint16V4(TmpPixelYUV_F32);

            //store
            DstPtrLm[x] = DstPixelYUV_U16[0];
            DstPtrCb[x] = DstPixelYUV_U16[1];
            DstPtrCr[x] = DstPixelYUV_U16[2];
        }
        DstPtrLm += DstStride;
        DstPtrCb += DstStride;
        DstPtrCr += DstStride;
        SrcPtrLm += SrcStride;
        SrcPtrCb += SrcStride;
        SrcPtrCr += SrcStride;
    }
}

void xSepia_STD::applySepiaEffect_FLT(xPic &Dst, const xPic &Src) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());

    const int32 Width = Dst.getWidth();
    const int32 Height = Dst.getHeight();
    uint16 *restrict DstPtrLm = Dst.getAddr(0);
    uint16 *restrict DstPtrCb = Dst.getAddr(1);
    uint16 *restrict DstPtrCr = Dst.getAddr(2);
    const uint16 *SrcPtrLm = Src.getAddr(0);
    const uint16 *SrcPtrCb = Src.getAddr(1);
    const uint16 *SrcPtrCr = Src.getAddr(2);
    const int32 DstStride = Dst.getStride();
    const int32 SrcStride = Src.getStride();

    const flt32 SrcMidValue = flt32(1 << (Src.getBitDepth() - 1));
    const flt32 DstMidValue = flt32(1 << (Dst.getBitDepth() - 1));
    const flt32 DstMaxValue = flt32((1 << Dst.getBitDepth()) - 1);

    for (int32 y = 0; y < Height; y++) {
        for (int32 x = 0; x < Width; x++) {
            //load
            const uint16V4 SrcPixelYUV_U16 = {SrcPtrLm[x], SrcPtrCb[x], SrcPtrCr[x], 0};

            //change data format (and remove chroma offset)
            const flt32V4 SrcPixelYUV_F32 = ((flt32V4) SrcPixelYUV_U16 - flt32V4(0, SrcMidValue, SrcMidValue, 0));

            //convert YCbCr --> RGB (BT.709)
            const flt32V4 SrcPixelRGB_F32 =
                    {
                            SrcPixelYUV_F32[0] + SrcPixelYUV_F32[2] * YCbCr2RGB_F32[0][2],
                            SrcPixelYUV_F32[0] + SrcPixelYUV_F32[1] * YCbCr2RGB_F32[1][1] +
                            SrcPixelYUV_F32[2] * YCbCr2RGB_F32[1][2],
                            SrcPixelYUV_F32[0] + SrcPixelYUV_F32[1] * YCbCr2RGB_F32[2][1],
                            0
                    };

            //apply sepia effect
            const flt32V4 DstPixelRGB_F32 =
                    {
                            SrcPixelRGB_F32[0] * SEPIA_F32[0][0] + SrcPixelRGB_F32[1] * SEPIA_F32[0][1] +
                            SrcPixelRGB_F32[2] * SEPIA_F32[0][2],
                            SrcPixelRGB_F32[0] * SEPIA_F32[1][0] + SrcPixelRGB_F32[1] * SEPIA_F32[1][1] +
                            SrcPixelRGB_F32[2] * SEPIA_F32[1][2],
                            SrcPixelRGB_F32[0] * SEPIA_F32[2][0] + SrcPixelRGB_F32[1] * SEPIA_F32[2][1] +
                            SrcPixelRGB_F32[2] * SEPIA_F32[2][2],
                            0
                    };

            //convert RGB --> YCbCr (BT.709)
            const flt32V4 DstPixelYUV_F32 = {
                    DstPixelRGB_F32[0] * RGB2YCbCr_I32[0][0] + DstPixelRGB_F32[1] * RGB2YCbCr_I32[0][1] +
                    DstPixelRGB_F32[2] * RGB2YCbCr_I32[0][2],
                    DstPixelRGB_F32[0] * RGB2YCbCr_I32[1][0] + DstPixelRGB_F32[1] * RGB2YCbCr_I32[1][1] +
                    DstPixelRGB_F32[2] * RGB2YCbCr_I32[1][2],
                    DstPixelRGB_F32[0] * RGB2YCbCr_I32[2][0] + DstPixelRGB_F32[1] * RGB2YCbCr_I32[2][1] +
                    DstPixelRGB_F32[2] * RGB2YCbCr_I32[2][2],
                    0
            };

            //change data format (and apply chroma offset) + clip to range 0-Max
            flt32V4 TmpPixelYUV_F32 = DstPixelYUV_F32 + flt32V4(0, DstMidValue, DstMidValue, 0);
            TmpPixelYUV_F32.clip(xMakeVec4<flt32>(0), xMakeVec4<flt32>(DstMaxValue));
            const uint16V4 DstPixelYUV_U16 = uint16V4(TmpPixelYUV_F32);

            //store
            DstPtrLm[x] = DstPixelYUV_U16[0];
            DstPtrCb[x] = DstPixelYUV_U16[1];
            DstPtrCr[x] = DstPixelYUV_U16[2];
        }
        DstPtrLm += DstStride;
        DstPtrCb += DstStride;
        DstPtrCr += DstStride;
        SrcPtrLm += SrcStride;
        SrcPtrCb += SrcStride;
        SrcPtrCr += SrcStride;
    }

}

void xSepia_STD::applySepiaEffect_INT(xPic &Dst, const xPic &Src) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());
    //WRITE ME!
    assert(0);
}

//===============================================================================================================================================================================================================
