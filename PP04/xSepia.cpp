#include "xSepia.h"

//===============================================================================================================================================================================================================
void print128_num(__m128i var) {
    uint32_t val[4];
    memcpy(val, &var, sizeof(val));
    printf("Numerical: %i %i %i %i \n",
           val[0], val[1], val[2], val[3]);
}

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

void xSepia_STD::testYUVtoRGBtoYUV_INT(xPic &Dst, const xPic &Src) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());
    //NOTHING YET
    assert(0);
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

void xSepia_STD::applySepiaEffect_INT(xPic &Dst, const xPic &Src) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());
    //WRITE ME!
    assert(0);
}

//===============================================================================================================================================================================================================

#if X_SSE_ALL

void xSepia_SSE::testCopyContent(xPic &Dst, const xPic &Src) {
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
    if (((uint32) Width & (uint32) c_RemainderMask8) == 0) //Width%8==0
    {
        for (int32 y = 0; y < Height; y++) {
            for (int32 x = 0; x < Width; x += 8) {
                //load
                __m128i SrcLmV = _mm_loadu_si128((__m128i *) (SrcPtrLm + x));
                __m128i SrcCbV = _mm_loadu_si128((__m128i *) (SrcPtrCb + x));
                __m128i SrcCrV = _mm_loadu_si128((__m128i *) (SrcPtrCr + x));
                //store
                _mm_storeu_si128((__m128i *) (DstPtrLm + x), SrcLmV);
                _mm_storeu_si128((__m128i *) (DstPtrCb + x), SrcCbV);
                _mm_storeu_si128((__m128i *) (DstPtrCr + x), SrcCrV);
            }
            DstPtrLm += DstStride;
            DstPtrCb += DstStride;
            DstPtrCr += DstStride;
            SrcPtrLm += SrcStride;
            SrcPtrCb += SrcStride;
            SrcPtrCr += SrcStride;
        }
    } else {
        const int32 Width8 = (int32) ((uint32) Width & c_MultipleMask8);
        const int32 Width4 = (int32) ((uint32) Width & c_MultipleMask4);

        for (int32 y = 0; y < Height; y++) {
            for (int32 x = 0; x < Width8; x += 8) {
                //load
                __m128i SrcLmV = _mm_loadu_si128((__m128i *) (SrcPtrLm + x));
                __m128i SrcCbV = _mm_loadu_si128((__m128i *) (SrcPtrCb + x));
                __m128i SrcCrV = _mm_loadu_si128((__m128i *) (SrcPtrCr + x));
                //store
                _mm_storeu_si128((__m128i *) (DstPtrLm + x), SrcLmV);
                _mm_storeu_si128((__m128i *) (DstPtrCb + x), SrcCbV);
                _mm_storeu_si128((__m128i *) (DstPtrCr + x), SrcCrV);
            }
            for (int32 x = Width8; x < Width4; x += 4) {
                //load
                __m128i SrcLmV = _mm_loadl_epi64((__m128i *) (DstPtrLm + x));
                __m128i SrcCbV = _mm_loadl_epi64((__m128i *) (DstPtrCb + x));
                __m128i SrcCrV = _mm_loadl_epi64((__m128i *) (DstPtrCr + x));
                //store
                _mm_storel_epi64((__m128i *) (DstPtrLm + x), SrcLmV);
                _mm_storel_epi64((__m128i *) (DstPtrCb + x), SrcCbV);
                _mm_storel_epi64((__m128i *) (DstPtrCr + x), SrcCrV);
            }
            for (int32 x = Width4; x < Width; x++) {
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
}

void xSepia_SSE::testYUVtoRGBtoYUV_FLT(xPic &Dst, const xPic &Src) {
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
    const flt32 DstMaxValue = flt32((1 << Dst.getBitDepth()) - 1);

    const __m128i SrcMidValueV_I32 = _mm_set1_epi32(SrcMidValue);
    const __m128i DstMidValueV_I32 = _mm_set1_epi32(DstMidValue);
    const __m128i DstMaxValueV_U16 = _mm_set1_epi16((uint16) DstMaxValue);

    //WARNING - for demonstration purposes only - it isn't the fastest way to copy data
    if (((uint32) Width & (uint32) c_RemainderMask8) == 0) //Width%8==0
    {
        for (int32 y = 0; y < Height; y++) {
            for (int32 x = 0; x < Width; x += 8) {
                //load
                __m128i SrcLmV_U16 = _mm_loadu_si128((__m128i *) (SrcPtrLm + x));
                __m128i SrcCbV_U16 = _mm_loadu_si128((__m128i *) (SrcPtrCb + x));
                __m128i SrcCrV_U16 = _mm_loadu_si128((__m128i *) (SrcPtrCr + x));

                //change data format (and remove chroma offset)
                __m128i SrcLm0_I32 = _mm_cvtepi16_epi32(SrcLmV_U16);
                __m128i SrcLm1_I32 = _mm_cvtepi16_epi32(_mm_srli_si128(SrcLmV_U16, 8));
                __m128i SrcCb0_I32 = _mm_cvtepi16_epi32(SrcCbV_U16);
                __m128i SrcCb1_I32 = _mm_cvtepi16_epi32(_mm_srli_si128(SrcCbV_U16, 8));
                __m128i SrcCr0_I32 = _mm_cvtepi16_epi32(SrcCrV_U16);
                __m128i SrcCr1_I32 = _mm_cvtepi16_epi32(_mm_srli_si128(SrcCrV_U16, 8));

                __m128 SrcLm0_F32 = _mm_cvtepi32_ps(SrcLm0_I32);
                __m128 SrcLm1_F32 = _mm_cvtepi32_ps(SrcLm1_I32);
                __m128 SrcCb0_F32 = _mm_cvtepi32_ps(_mm_sub_epi32(SrcCb0_I32, SrcMidValueV_I32));
                __m128 SrcCb1_F32 = _mm_cvtepi32_ps(_mm_sub_epi32(SrcCb1_I32, SrcMidValueV_I32));
                __m128 SrcCr0_F32 = _mm_cvtepi32_ps(_mm_sub_epi32(SrcCr0_I32, SrcMidValueV_I32));
                __m128 SrcCr1_F32 = _mm_cvtepi32_ps(_mm_sub_epi32(SrcCr1_I32, SrcMidValueV_I32));

                //convert YCbCr --> RGB (BT.709)
                __m128 SrcR0_F32 = _mm_add_ps(SrcLm0_F32, _mm_mul_ps(SrcCr0_F32, _mm_set1_ps(YCbCr2RGB_F32[0][2])));
                __m128 SrcR1_F32 = _mm_add_ps(SrcLm1_F32, _mm_mul_ps(SrcCr1_F32, _mm_set1_ps(YCbCr2RGB_F32[0][2])));
                __m128 SrcG0_F32 = _mm_add_ps(SrcLm0_F32,
                                              _mm_add_ps(_mm_mul_ps(SrcCb0_F32, _mm_set1_ps(YCbCr2RGB_F32[1][1])),
                                                         _mm_mul_ps(SrcCr0_F32, _mm_set1_ps(YCbCr2RGB_F32[1][2]))));
                __m128 SrcG1_F32 = _mm_add_ps(SrcLm1_F32,
                                              _mm_add_ps(_mm_mul_ps(SrcCb1_F32, _mm_set1_ps(YCbCr2RGB_F32[1][1])),
                                                         _mm_mul_ps(SrcCr1_F32, _mm_set1_ps(YCbCr2RGB_F32[1][2]))));
                __m128 SrcB0_F32 = _mm_add_ps(SrcLm0_F32, _mm_mul_ps(SrcCb0_F32, _mm_set1_ps(YCbCr2RGB_F32[2][1])));
                __m128 SrcB1_F32 = _mm_add_ps(SrcLm1_F32, _mm_mul_ps(SrcCb1_F32, _mm_set1_ps(YCbCr2RGB_F32[2][1])));

                //copy RGB --> RGB
                __m128 DstR0_F32 = SrcR0_F32;
                __m128 DstR1_F32 = SrcR1_F32;
                __m128 DstG0_F32 = SrcG0_F32;
                __m128 DstG1_F32 = SrcG1_F32;
                __m128 DstB0_F32 = SrcB0_F32;
                __m128 DstB1_F32 = SrcB1_F32;

                //convert RGB --> YCbCr (BT.709)
                __m128 DstLm0_F32 = _mm_add_ps(_mm_mul_ps(DstR0_F32, _mm_set1_ps(RGB2YCbCr_F32[0][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG0_F32, _mm_set1_ps(RGB2YCbCr_F32[0][1])),
                                                          _mm_mul_ps(DstB0_F32, _mm_set1_ps(RGB2YCbCr_F32[0][2]))));
                __m128 DstLm1_F32 = _mm_add_ps(_mm_mul_ps(DstR1_F32, _mm_set1_ps(RGB2YCbCr_F32[0][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG1_F32, _mm_set1_ps(RGB2YCbCr_F32[0][1])),
                                                          _mm_mul_ps(DstB1_F32, _mm_set1_ps(RGB2YCbCr_F32[0][2]))));
                __m128 DstCb0_F32 = _mm_add_ps(_mm_mul_ps(DstR0_F32, _mm_set1_ps(RGB2YCbCr_F32[1][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG0_F32, _mm_set1_ps(RGB2YCbCr_F32[1][1])),
                                                          _mm_mul_ps(DstB0_F32, _mm_set1_ps(RGB2YCbCr_F32[1][2]))));
                __m128 DstCb1_F32 = _mm_add_ps(_mm_mul_ps(DstR1_F32, _mm_set1_ps(RGB2YCbCr_F32[1][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG1_F32, _mm_set1_ps(RGB2YCbCr_F32[1][1])),
                                                          _mm_mul_ps(DstB1_F32, _mm_set1_ps(RGB2YCbCr_F32[1][2]))));
                __m128 DstCr0_F32 = _mm_add_ps(_mm_mul_ps(DstR0_F32, _mm_set1_ps(RGB2YCbCr_F32[2][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG0_F32, _mm_set1_ps(RGB2YCbCr_F32[2][1])),
                                                          _mm_mul_ps(DstB0_F32, _mm_set1_ps(RGB2YCbCr_F32[2][2]))));
                __m128 DstCr1_F32 = _mm_add_ps(_mm_mul_ps(DstR1_F32, _mm_set1_ps(RGB2YCbCr_F32[2][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG1_F32, _mm_set1_ps(RGB2YCbCr_F32[2][1])),
                                                          _mm_mul_ps(DstB1_F32, _mm_set1_ps(RGB2YCbCr_F32[2][2]))));

                //change data format (and apply chroma offset) + clip to range 0-Max
                __m128i DstLm0_I32 = _mm_cvtps_epi32(DstLm0_F32);
                __m128i DstLm1_I32 = _mm_cvtps_epi32(DstLm1_F32);
                __m128i DstCb0_I32 = _mm_add_epi32(_mm_cvtps_epi32(DstCb0_F32), DstMidValueV_I32);
                __m128i DstCb1_I32 = _mm_add_epi32(_mm_cvtps_epi32(DstCb1_F32), DstMidValueV_I32);
                __m128i DstCr0_I32 = _mm_add_epi32(_mm_cvtps_epi32(DstCr0_F32), DstMidValueV_I32);
                __m128i DstCr1_I32 = _mm_add_epi32(_mm_cvtps_epi32(DstCr1_F32), DstMidValueV_I32);

                __m128i DstLm_I16 = _mm_packs_epi32(DstLm0_I32, DstLm1_I32);
                __m128i DstCb_I16 = _mm_packs_epi32(DstCb0_I32, DstCb1_I32);
                __m128i DstCr_I16 = _mm_packs_epi32(DstCr0_I32, DstCr1_I32);

                //clip
                const __m128i ZeroV_U16 = _mm_setzero_si128();
                __m128i DstLm_U16 = _mm_max_epi16(_mm_min_epi16(DstLm_I16, DstMaxValueV_U16), ZeroV_U16);
                __m128i DstCb_U16 = _mm_max_epi16(_mm_min_epi16(DstCb_I16, DstMaxValueV_U16), ZeroV_U16);
                __m128i DstCr_U16 = _mm_max_epi16(_mm_min_epi16(DstCr_I16, DstMaxValueV_U16), ZeroV_U16);

                //store
                _mm_storeu_si128((__m128i *) (DstPtrLm + x), DstLm_U16);
                _mm_storeu_si128((__m128i *) (DstPtrCb + x), DstCb_U16);
                _mm_storeu_si128((__m128i *) (DstPtrCr + x), DstCr_U16);
            }
            DstPtrLm += DstStride;
            DstPtrCb += DstStride;
            DstPtrCr += DstStride;
            SrcPtrLm += SrcStride;
            SrcPtrCb += SrcStride;
            SrcPtrCr += SrcStride;
        }
    } else {
        assert(0);
    }
}

void xSepia_SSE::testYUVtoRGBtoYUV_INT(xPic &Dst, const xPic &Src) {
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
    const flt32 DstMaxValue = flt32((1 << Dst.getBitDepth()) - 1);

    const __m128i SrcMidValueV_I32 = _mm_set1_epi32(SrcMidValue);
    const __m128i DstMidValueV_I32 = _mm_set1_epi32(DstMidValue);
    const __m128i DstMaxValueV_U16 = _mm_set1_epi16((uint16) DstMaxValue);

    //WARNING - for demonstration purposes only - it isn't the fastest way to copy data
    if (((uint32) Width & (uint32) c_RemainderMask8) == 0) //Width%8==0
    {
        for (int32 y = 0; y < Height; y++) {
            for (int32 x = 0; x < Width; x += 8) {
                //load
                __m128i SrcLmV_U16 = _mm_loadu_si128((__m128i *) (SrcPtrLm + x));
                __m128i SrcCbV_U16 = _mm_loadu_si128((__m128i *) (SrcPtrCb + x));
                __m128i SrcCrV_U16 = _mm_loadu_si128((__m128i *) (SrcPtrCr + x));

                //change data format (and remove chroma offset)
                __m128i SrcLm0_I32 = _mm_cvtepi16_epi32(SrcLmV_U16);
                __m128i SrcLm1_I32 = _mm_cvtepi16_epi32(_mm_srli_si128(SrcLmV_U16, 8));
                __m128i SrcCb0_I32 = _mm_cvtepi16_epi32(SrcCbV_U16);
                __m128i SrcCb1_I32 = _mm_cvtepi16_epi32(_mm_srli_si128(SrcCbV_U16, 8));
                __m128i SrcCr0_I32 = _mm_cvtepi16_epi32(SrcCrV_U16);
                __m128i SrcCr1_I32 = _mm_cvtepi16_epi32(_mm_srli_si128(SrcCrV_U16, 8));

                // bright it up
                SrcLm0_I32 = _mm_mul_epi32(SrcLm0_I32, _mm_set1_epi32(32768));
                SrcLm1_I32 = _mm_mul_epi32(SrcLm1_I32, _mm_set1_epi32(32768));

                // removing chroma offset
                SrcCb0_I32 = _mm_sub_epi32(SrcCb0_I32, SrcMidValueV_I32);
                SrcCb1_I32 = _mm_sub_epi32(SrcCb1_I32, SrcMidValueV_I32);
                SrcCr0_I32 = _mm_sub_epi32(SrcCr0_I32, SrcMidValueV_I32);
                SrcCr1_I32 = _mm_sub_epi32(SrcCr1_I32, SrcMidValueV_I32);

                //convert YCbCr --> RGB (BT.709)
                __m128i SrcR0_I32 = _mm_add_epi32(
                        SrcLm0_I32,
                        _mm_mul_epi32(
                                SrcCr0_I32,
                                _mm_set1_epi32(YCbCr2RGB_I32[0][2])));

                __m128i SrcR1_I32 = _mm_add_epi32(
                        SrcLm1_I32,
                        _mm_mul_epi32(
                                SrcCr1_I32,
                                _mm_set1_epi32(YCbCr2RGB_I32[0][2])));

                __m128i SrcG0_I32 = _mm_add_epi32(
                        SrcLm0_I32,
                        _mm_add_epi32(
                                _mm_mul_epi32(
                                        SrcCb0_I32,
                                        _mm_set1_epi32(YCbCr2RGB_I32[1][1])),
                                _mm_mul_epi32(
                                        SrcCr0_I32,
                                        _mm_set1_epi32(YCbCr2RGB_I32[1][2]))));

                __m128i SrcG1_I32 = _mm_add_epi32(
                        SrcLm1_I32,
                        _mm_add_epi32(
                                _mm_mul_epi32(
                                        SrcCb1_I32,
                                        _mm_set1_epi32(YCbCr2RGB_I32[1][1])),
                                _mm_mul_epi32(
                                        SrcCr1_I32,
                                        _mm_set1_epi32(YCbCr2RGB_I32[1][2]))));

                __m128i SrcB0_I32 = _mm_add_epi32(
                        SrcLm0_I32,
                        _mm_mul_epi32(
                                SrcCb0_I32,
                                _mm_set1_epi32(YCbCr2RGB_I32[2][1])));

                __m128i SrcB1_I32 = _mm_add_epi32(
                        SrcLm1_I32,
                        _mm_mul_epi32(
                                SrcCb1_I32,
                                _mm_set1_epi32(YCbCr2RGB_I32[2][1])));

                //add round value and right shift by 16
                SrcR0_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                SrcR0_I32,
                                _mm_set1_epi32(32768)),
                        16);
                SrcR1_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                SrcR1_I32,
                                _mm_set1_epi32(32768)),
                        16);
                SrcG0_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                SrcG0_I32,
                                _mm_set1_epi32(32768)),
                        16);
                SrcG1_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                SrcG1_I32,
                                _mm_set1_epi32(32768)),
                        16);
                SrcB0_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                SrcB0_I32,
                                _mm_set1_epi32(32768)),
                        16);
                SrcB1_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                SrcB1_I32,
                                _mm_set1_epi32(32768)),
                        16);


                //copy RGB --> RGB
                __m128i DstR0_I32 = SrcR0_I32;
                __m128i DstR1_I32 = SrcR1_I32;
                __m128i DstG0_I32 = SrcG0_I32;
                __m128i DstG1_I32 = SrcG1_I32;
                __m128i DstB0_I32 = SrcB0_I32;
                __m128i DstB1_I32 = SrcB1_I32;

//                //print section for debugging purposes (debugger sucks..)
//                print128_num(DstR0_I32);
//                print128_num(DstR1_I32);
//                print128_num(DstG0_I32);
//                print128_num(DstG1_I32);
//                print128_num(DstB0_I32);
//                print128_num(DstB1_I32);
//                printf("\n\n");

                //convert RGB --> YCbCr (BT.709)
                __m128i DstLm0_I32 = _mm_add_epi32(
                        _mm_mul_epi32(
                                DstR0_I32,
                                _mm_set1_epi32(RGB2YCbCr_I32[0][0])),
                        _mm_add_epi32(
                                _mm_mul_epi32(
                                        DstG0_I32,
                                        _mm_set1_epi32(RGB2YCbCr_I32[0][1])),
                                _mm_mul_epi32(
                                        DstB0_I32,
                                        _mm_set1_epi32(RGB2YCbCr_I32[0][2]))));

                __m128i DstLm1_I32 = _mm_add_epi32(
                        _mm_mul_epi32(
                                DstR1_I32,
                                _mm_set1_epi32(RGB2YCbCr_I32[0][0])),
                        _mm_add_epi32(
                                _mm_mul_epi32(
                                        DstG1_I32,
                                        _mm_set1_epi32(RGB2YCbCr_I32[0][1])),
                                _mm_mul_epi32(
                                        DstB1_I32,
                                        _mm_set1_epi32(RGB2YCbCr_I32[0][2]))));

                __m128i DstCb0_I32 = _mm_add_epi32(
                        _mm_mul_epi32(
                                DstR0_I32,
                                _mm_set1_epi32(RGB2YCbCr_I32[1][0])),
                        _mm_add_epi32(_mm_mul_epi32(
                                              DstG0_I32,
                                              _mm_set1_epi32(
                                                      RGB2YCbCr_I32[1][1])),
                                      _mm_mul_epi32(
                                              DstB0_I32,
                                              _mm_set1_epi32(RGB2YCbCr_I32[1][2]))));

                __m128i DstCb1_I32 = _mm_add_epi32(
                        _mm_mul_epi32(
                                DstR1_I32,
                                _mm_set1_epi32(RGB2YCbCr_I32[1][0])),
                        _mm_add_epi32(
                                _mm_mul_epi32(
                                        DstG1_I32,
                                        _mm_set1_epi32(
                                                RGB2YCbCr_I32[1][1])),
                                _mm_mul_epi32(
                                        DstB1_I32,
                                        _mm_set1_epi32(RGB2YCbCr_I32[1][2]))));

                __m128i DstCr0_I32 = _mm_add_epi32(
                        _mm_mul_epi32(
                                DstR0_I32,
                                _mm_set1_epi32(
                                        RGB2YCbCr_I32[2][0])),
                        _mm_add_epi32(
                                _mm_mul_epi32(
                                        DstG0_I32,
                                        _mm_set1_epi32(
                                                RGB2YCbCr_I32[2][1])),
                                _mm_mul_epi32(
                                        DstB0_I32,
                                        _mm_set1_epi32(RGB2YCbCr_I32[2][2]))));

                __m128i DstCr1_I32 = _mm_add_epi32(
                        _mm_mul_epi32(
                                DstR1_I32,
                                _mm_set1_epi32(
                                        RGB2YCbCr_I32[2][0])),
                        _mm_add_epi32(
                                _mm_mul_epi32(
                                        DstG1_I32,
                                        _mm_set1_epi32(
                                                RGB2YCbCr_I32[2][1])),
                                _mm_mul_epi32(
                                        DstB1_I32,
                                        _mm_set1_epi32(RGB2YCbCr_I32[2][2]))));

                //add round value and right shift by 16
                DstLm0_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                DstLm0_I32,
                                _mm_set1_epi32(32768)),
                        16);
                DstLm1_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                DstLm1_I32,
                                _mm_set1_epi32(32768)),
                        16);
                DstCb0_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                DstCb0_I32,
                                _mm_set1_epi32(32768)),
                        16);

                DstCb1_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                DstCb1_I32,
                                _mm_set1_epi32(32768)),
                        16);
                DstCr0_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                DstCr0_I32,
                                _mm_set1_epi32(32768)),
                        16);
                DstCr1_I32 = _mm_srai_epi32(
                        _mm_add_epi32(
                                DstCr1_I32,
                                _mm_set1_epi32(32768)),
                        16);



                //skipping converting luma cuz we work on ints
                //change data format (and apply chroma offset) + clip to range 0-Max
//                __m128i DstLm0_I32 = _mm_cvtps_epi32(DstLm0_I32);
//                __m128i DstLm1_I32 = _mm_cvtps_epi32(DstLm1_I32);
                DstCb0_I32 = _mm_add_epi32(DstCb0_I32, DstMidValueV_I32);
                DstCb1_I32 = _mm_add_epi32(DstCb1_I32, DstMidValueV_I32);
                DstCr0_I32 = _mm_add_epi32(DstCr0_I32, DstMidValueV_I32);
                DstCr1_I32 = _mm_add_epi32(DstCr1_I32, DstMidValueV_I32);

                __m128i DstLm_I16 = _mm_packs_epi32(DstLm0_I32, DstLm1_I32);
                __m128i DstCb_I16 = _mm_packs_epi32(DstCb0_I32, DstCb1_I32);
                __m128i DstCr_I16 = _mm_packs_epi32(DstCr0_I32, DstCr1_I32);

                //clip
                const __m128i ZeroV_U16 = _mm_setzero_si128();
                __m128i DstLm_U16 = _mm_max_epi16(
                        _mm_min_epi16(
                                DstLm_I16,
                                DstMaxValueV_U16),
                        ZeroV_U16);
                __m128i DstCb_U16 = _mm_max_epi16(
                        _mm_min_epi16(
                                DstCb_I16,
                                DstMaxValueV_U16),
                        ZeroV_U16);
                __m128i DstCr_U16 = _mm_max_epi16(
                        _mm_min_epi16(
                                DstCr_I16,
                                DstMaxValueV_U16),
                        ZeroV_U16);

                //store
                _mm_storeu_si128((__m128i *) (DstPtrLm + x), DstLm_U16);
                _mm_storeu_si128((__m128i *) (DstPtrCb + x), DstCb_U16);
                _mm_storeu_si128((__m128i *) (DstPtrCr + x), DstCr_U16);
            }
            DstPtrLm += DstStride;
            DstPtrCb += DstStride;
            DstPtrCr += DstStride;
            SrcPtrLm += SrcStride;
            SrcPtrCb += SrcStride;
            SrcPtrCr += SrcStride;
        }
    } else {
        assert(0);
    }
}

void xSepia_SSE::applySepiaEffect_FLT(xPic &Dst, const xPic &Src) {
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
    const flt32 DstMaxValue = flt32((1 << Dst.getBitDepth()) - 1);

    const __m128i SrcMidValueV_I32 = _mm_set1_epi32(SrcMidValue);
    const __m128i DstMidValueV_I32 = _mm_set1_epi32(DstMidValue);
    const __m128i DstMaxValueV_U16 = _mm_set1_epi16((uint16) DstMaxValue);

    //WARNING - for demonstration purposes only - it isn't the fastest way to copy data
    if (((uint32) Width & (uint32) c_RemainderMask8) == 0) //Width%8==0
    {
        for (int32 y = 0; y < Height; y++) {
            for (int32 x = 0; x < Width; x += 8) {
                //load
                __m128i SrcLmV_U16 = _mm_loadu_si128((__m128i *) (SrcPtrLm + x));
                __m128i SrcCbV_U16 = _mm_loadu_si128((__m128i *) (SrcPtrCb + x));
                __m128i SrcCrV_U16 = _mm_loadu_si128((__m128i *) (SrcPtrCr + x));

                //change data format (and remove chroma offset)
                __m128i SrcLm0_I32 = _mm_cvtepi16_epi32(SrcLmV_U16);
                __m128i SrcLm1_I32 = _mm_cvtepi16_epi32(_mm_srli_si128(SrcLmV_U16, 8));
                __m128i SrcCb0_I32 = _mm_cvtepi16_epi32(SrcCbV_U16);
                __m128i SrcCb1_I32 = _mm_cvtepi16_epi32(_mm_srli_si128(SrcCbV_U16, 8));
                __m128i SrcCr0_I32 = _mm_cvtepi16_epi32(SrcCrV_U16);
                __m128i SrcCr1_I32 = _mm_cvtepi16_epi32(_mm_srli_si128(SrcCrV_U16, 8));

                __m128 SrcLm0_F32 = _mm_cvtepi32_ps(SrcLm0_I32);
                __m128 SrcLm1_F32 = _mm_cvtepi32_ps(SrcLm1_I32);
                __m128 SrcCb0_F32 = _mm_cvtepi32_ps(_mm_sub_epi32(SrcCb0_I32, SrcMidValueV_I32));
                __m128 SrcCb1_F32 = _mm_cvtepi32_ps(_mm_sub_epi32(SrcCb1_I32, SrcMidValueV_I32));
                __m128 SrcCr0_F32 = _mm_cvtepi32_ps(_mm_sub_epi32(SrcCr0_I32, SrcMidValueV_I32));
                __m128 SrcCr1_F32 = _mm_cvtepi32_ps(_mm_sub_epi32(SrcCr1_I32, SrcMidValueV_I32));

                //convert YCbCr --> RGB (BT.709)
                __m128 SrcR0_F32 = _mm_add_ps(SrcLm0_F32, _mm_mul_ps(SrcCr0_F32, _mm_set1_ps(YCbCr2RGB_F32[0][2])));
                __m128 SrcR1_F32 = _mm_add_ps(SrcLm1_F32, _mm_mul_ps(SrcCr1_F32, _mm_set1_ps(YCbCr2RGB_F32[0][2])));
                __m128 SrcG0_F32 = _mm_add_ps(SrcLm0_F32,
                                              _mm_add_ps(_mm_mul_ps(SrcCb0_F32, _mm_set1_ps(YCbCr2RGB_F32[1][1])),
                                                         _mm_mul_ps(SrcCr0_F32, _mm_set1_ps(YCbCr2RGB_F32[1][2]))));
                __m128 SrcG1_F32 = _mm_add_ps(SrcLm1_F32,
                                              _mm_add_ps(_mm_mul_ps(SrcCb1_F32, _mm_set1_ps(YCbCr2RGB_F32[1][1])),
                                                         _mm_mul_ps(SrcCr1_F32, _mm_set1_ps(YCbCr2RGB_F32[1][2]))));
                __m128 SrcB0_F32 = _mm_add_ps(SrcLm0_F32, _mm_mul_ps(SrcCb0_F32, _mm_set1_ps(YCbCr2RGB_F32[2][1])));
                __m128 SrcB1_F32 = _mm_add_ps(SrcLm1_F32, _mm_mul_ps(SrcCb1_F32, _mm_set1_ps(YCbCr2RGB_F32[2][1])));

                //apply sepia effect
                __m128 DstR0_F32 = _mm_add_ps(
                        _mm_mul_ps(
                                SrcR0_F32,
                                _mm_set1_ps(SEPIA_F32[0][0])),
                        _mm_add_ps(
                                _mm_mul_ps(
                                        SrcG0_F32,
                                        _mm_set1_ps(SEPIA_F32[0][1])),
                                _mm_mul_ps(
                                        SrcB0_F32,
                                        _mm_set1_ps(SEPIA_F32[0][2]))));

                __m128 DstR1_F32 = _mm_add_ps(
                        _mm_mul_ps(
                                SrcR1_F32,
                                _mm_set1_ps(SEPIA_F32[0][0])),
                        _mm_add_ps(
                                _mm_mul_ps(
                                        SrcG1_F32,
                                        _mm_set1_ps(SEPIA_F32[0][1])),
                                _mm_mul_ps(
                                        SrcB1_F32,
                                        _mm_set1_ps(SEPIA_F32[0][2]))));

                __m128 DstG0_F32 = _mm_add_ps(
                        _mm_mul_ps(
                                SrcR0_F32,
                                _mm_set1_ps(SEPIA_F32[1][0])),
                        _mm_add_ps(
                                _mm_mul_ps(
                                        SrcG0_F32,
                                        _mm_set1_ps(SEPIA_F32[1][1])),
                                _mm_mul_ps(
                                        SrcB0_F32,
                                        _mm_set1_ps(SEPIA_F32[1][2]))));

                __m128 DstG1_F32 = _mm_add_ps(
                        _mm_mul_ps(
                                SrcR1_F32,
                                _mm_set1_ps(SEPIA_F32[1][0])),
                        _mm_add_ps(
                                _mm_mul_ps(
                                        SrcG1_F32,
                                        _mm_set1_ps(SEPIA_F32[1][1])),
                                _mm_mul_ps(
                                        SrcB1_F32,
                                        _mm_set1_ps(SEPIA_F32[1][2]))));

                __m128 DstB0_F32 = _mm_add_ps(
                        _mm_mul_ps(
                                SrcR0_F32,
                                _mm_set1_ps(SEPIA_F32[2][0])),
                        _mm_add_ps(
                                _mm_mul_ps(
                                        SrcG0_F32,
                                        _mm_set1_ps(SEPIA_F32[2][1])),
                                _mm_mul_ps(
                                        SrcB0_F32,
                                        _mm_set1_ps(SEPIA_F32[2][2]))));

                __m128 DstB1_F32 = _mm_add_ps(
                        _mm_mul_ps(
                                SrcR1_F32,
                                _mm_set1_ps(SEPIA_F32[2][0])),
                        _mm_add_ps(
                                _mm_mul_ps(
                                        SrcG1_F32,
                                        _mm_set1_ps(SEPIA_F32[2][1])),
                                _mm_mul_ps(
                                        SrcB1_F32,
                                        _mm_set1_ps(SEPIA_F32[2][2]))));

                //convert RGB --> YCbCr (BT.709)
                __m128 DstLm0_F32 = _mm_add_ps(_mm_mul_ps(DstR0_F32, _mm_set1_ps(RGB2YCbCr_F32[0][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG0_F32, _mm_set1_ps(RGB2YCbCr_F32[0][1])),
                                                          _mm_mul_ps(DstB0_F32, _mm_set1_ps(RGB2YCbCr_F32[0][2]))));
                __m128 DstLm1_F32 = _mm_add_ps(_mm_mul_ps(DstR1_F32, _mm_set1_ps(RGB2YCbCr_F32[0][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG1_F32, _mm_set1_ps(RGB2YCbCr_F32[0][1])),
                                                          _mm_mul_ps(DstB1_F32, _mm_set1_ps(RGB2YCbCr_F32[0][2]))));
                __m128 DstCb0_F32 = _mm_add_ps(_mm_mul_ps(DstR0_F32, _mm_set1_ps(RGB2YCbCr_F32[1][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG0_F32, _mm_set1_ps(RGB2YCbCr_F32[1][1])),
                                                          _mm_mul_ps(DstB0_F32, _mm_set1_ps(RGB2YCbCr_F32[1][2]))));
                __m128 DstCb1_F32 = _mm_add_ps(_mm_mul_ps(DstR1_F32, _mm_set1_ps(RGB2YCbCr_F32[1][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG1_F32, _mm_set1_ps(RGB2YCbCr_F32[1][1])),
                                                          _mm_mul_ps(DstB1_F32, _mm_set1_ps(RGB2YCbCr_F32[1][2]))));
                __m128 DstCr0_F32 = _mm_add_ps(_mm_mul_ps(DstR0_F32, _mm_set1_ps(RGB2YCbCr_F32[2][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG0_F32, _mm_set1_ps(RGB2YCbCr_F32[2][1])),
                                                          _mm_mul_ps(DstB0_F32, _mm_set1_ps(RGB2YCbCr_F32[2][2]))));
                __m128 DstCr1_F32 = _mm_add_ps(_mm_mul_ps(DstR1_F32, _mm_set1_ps(RGB2YCbCr_F32[2][0])),
                                               _mm_add_ps(_mm_mul_ps(DstG1_F32, _mm_set1_ps(RGB2YCbCr_F32[2][1])),
                                                          _mm_mul_ps(DstB1_F32, _mm_set1_ps(RGB2YCbCr_F32[2][2]))));

                //change data format (and apply chroma offset) + clip to range 0-Max
                __m128i DstLm0_I32 = _mm_cvtps_epi32(DstLm0_F32);
                __m128i DstLm1_I32 = _mm_cvtps_epi32(DstLm1_F32);
                __m128i DstCb0_I32 = _mm_add_epi32(_mm_cvtps_epi32(DstCb0_F32), DstMidValueV_I32);
                __m128i DstCb1_I32 = _mm_add_epi32(_mm_cvtps_epi32(DstCb1_F32), DstMidValueV_I32);
                __m128i DstCr0_I32 = _mm_add_epi32(_mm_cvtps_epi32(DstCr0_F32), DstMidValueV_I32);
                __m128i DstCr1_I32 = _mm_add_epi32(_mm_cvtps_epi32(DstCr1_F32), DstMidValueV_I32);

                __m128i DstLm_I16 = _mm_packs_epi32(DstLm0_I32, DstLm1_I32);
                __m128i DstCb_I16 = _mm_packs_epi32(DstCb0_I32, DstCb1_I32);
                __m128i DstCr_I16 = _mm_packs_epi32(DstCr0_I32, DstCr1_I32);

                //clip
                const __m128i ZeroV_U16 = _mm_setzero_si128();
                __m128i DstLm_U16 = _mm_max_epi16(_mm_min_epi16(DstLm_I16, DstMaxValueV_U16), ZeroV_U16);
                __m128i DstCb_U16 = _mm_max_epi16(_mm_min_epi16(DstCb_I16, DstMaxValueV_U16), ZeroV_U16);
                __m128i DstCr_U16 = _mm_max_epi16(_mm_min_epi16(DstCr_I16, DstMaxValueV_U16), ZeroV_U16);

                //store
                _mm_storeu_si128((__m128i *) (DstPtrLm + x), DstLm_U16);
                _mm_storeu_si128((__m128i *) (DstPtrCb + x), DstCb_U16);
                _mm_storeu_si128((__m128i *) (DstPtrCr + x), DstCr_U16);
            }
            DstPtrLm += DstStride;
            DstPtrCb += DstStride;
            DstPtrCr += DstStride;
            SrcPtrLm += SrcStride;
            SrcPtrCb += SrcStride;
            SrcPtrCr += SrcStride;
        }
    } else {
        assert(0);
    }
}

void xSepia_SSE::applySepiaEffect_INT(xPic &Dst, const xPic &Src) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());
    //WAITING...
    assert(0);
}

#endif //X_SSE_ALL

//===============================================================================================================================================================================================================
