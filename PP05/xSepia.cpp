#include "xSepia.h"

//===============================================================================================================================================================================================================
void xSepia_STD::createTasks(const xPic &Src, xPic &Dst, const int &Height, const int &Width, const int &TileSize, const bool &Mode){
    int32 TSize = TileSize;
    int32 x = 0, y = 0, tasks = 0;
    std::stack<int32> xstack, ystack;
    xstack.push(0);
    ystack.push(0);
    while (y <= Height) {
        TSize = getTileSize(Height, TileSize, y);
        x = xstack.top();
        xstack.pop();
        y = ystack.top();
        ystack.pop();
        while (x < Width) {
            TSize = getTileSize(Width, TSize, x);
            if (not ystack.empty()){
                if ((y + TSize) != ystack.top()) {
                    xstack.push(x);
                    ystack.push(y + TSize);
                }
            } else {
                xstack.push(x);
                ystack.push(y + TSize);
            }
            if (Mode) { //due to my laziness i wont make an enum or even more wont pass a method to execute, satisfy yself with this shit. enjoy.
                m_ThreadPool->addWaitingTask([&Dst, &Src, x, y, TSize](int32 /*ThreadIdx*/) {
                    xTestYUVtoRGBtoYUV_FLT(Dst, Src, y, y + TSize, x, x + TSize);
                });
            } else {
                m_ThreadPool->addWaitingTask([&Dst, &Src, x, y, TSize](int32 /*ThreadIdx*/) {
                    xApplySepiaEffect_FLT(Dst, Src, y, y + TSize, x, x + TSize);
                });
            }
            x += TSize;
            tasks += 1;
        }
        y += TSize;
        if (x == Width and y == Height) {
            m_ThreadPool->waitUntilTasksFinished(tasks);
            break;
        }
    }
}

//this time its not my laziness but simply for code to be divided into FLOAT and INTEGER part as it was since now.
void xSepia_STD::createTasks_butINT(const xPic &Src, xPic &Dst, const int &Height, const int &Width, const int &TileSize, const bool &Mode){
    int32 TSize = TileSize;
    int32 x = 0, y = 0, tasks = 0;
    std::stack<int32> xstack, ystack;
    xstack.push(0);
    ystack.push(0);
    while (y <= Height) {
        TSize = getTileSize(Height, TileSize, y);
        x = xstack.top();
        xstack.pop();
        y = ystack.top();
        ystack.pop();
        while (x < Width) {
            TSize = getTileSize(Width, TSize, x);
            if (not ystack.empty()){
                if ((y + TSize) != ystack.top()) {
                    xstack.push(x);
                    ystack.push(y + TSize);
                }
            } else {
                xstack.push(x);
                ystack.push(y + TSize);
            }
            if (Mode) { //due to my laziness i wont make an enum or even more wont pass a method to execute, satisfy yself with this shit. enjoy.
                m_ThreadPool->addWaitingTask([&Dst, &Src, x, y, TSize](int32 /*ThreadIdx*/) {
                    xTestYUVtoRGBtoYUV_INT(Dst, Src, y, y + TSize, x, x + TSize);
                });
            } else {
                m_ThreadPool->addWaitingTask([&Dst, &Src, x, y, TSize](int32 /*ThreadIdx*/) {
                    xApplySepiaEffect_INT(Dst, Src, y, y + TSize, x, x + TSize);
                });
            }
            x += TSize;
            tasks += 1;
        }
        y += TSize;
        if (x == Width and y == Height) {
            m_ThreadPool->waitUntilTasksFinished(tasks);
            break;
        }
    }
}

void xSepia_STD::createTasks_butSSE(const xPic &Src, xPic &Dst, const int &Height, const int &Width, const int &TileSize, const bool &Mode){
    int32 TSize = TileSize;
    int32 x = 0, y = 0, tasks = 0;
    std::stack<int32> xstack, ystack;
    xstack.push(0);
    ystack.push(0);
    while (y <= Height) {
        TSize = getTileSize(Height, TileSize, y);
        x = xstack.top();
        xstack.pop();
        y = ystack.top();
        ystack.pop();
        while (x < Width) {
            TSize = getTileSize(Width, TSize, x);
            if (not ystack.empty()){
                if ((y + TSize) != ystack.top()) {
                    xstack.push(x);
                    ystack.push(y + TSize);
                }
            } else {
                xstack.push(x);
                ystack.push(y + TSize);
            }
            if (Mode) { //due to my laziness i wont make an enum or even more wont pass a method to execute, satisfy yself with this shit. enjoy.
                m_ThreadPool->addWaitingTask([&Dst, &Src, x, y, TSize](int32 /*ThreadIdx*/) {
                    xTestYUVtoRGBtoYUV_SSE(Dst, Src, y, y + TSize, x, x + TSize);
                });
            } else {
                m_ThreadPool->addWaitingTask([&Dst, &Src, x, y, TSize](int32 /*ThreadIdx*/) {
                    xApplySepiaEffect_SSE(Dst, Src, y, y + TSize, x, x + TSize);
                });
            }
            x += TSize;
            tasks += 1;
        }
        y += TSize;
        if (x == Width and y == Height) {
            m_ThreadPool->waitUntilTasksFinished(tasks);
            break;
        }
    }
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

void xSepia_STD::testYUVtoRGBtoYUV_FLT(xPic &Dst, const xPic &Src, eMode ProcMode) {
    const int32 Width = Dst.getWidth();
    const int32 Height = Dst.getHeight();

    switch (ProcMode) {
        case eMode::SERIAL:
            xTestYUVtoRGBtoYUV_FLT(Dst, Src, 0, Height, 0, Width);
            break;

        case eMode::PIXELS:
            for (int32 y = 0; y < Height; y++) {
                for (int32 x = 0; x < Width; x++) {
                    m_ThreadPool->addWaitingTask([&Dst, &Src, x, y](int32 /*ThreadIdx*/) {
                        xTestYUVtoRGBtoYUV_FLT(Dst, Src, y, y + 1, x, x + 1);
                    });
                }
            }
            m_ThreadPool->waitUntilTasksFinished(Width * Height);
            break;

        case eMode::ROWS:
            for (int32 y = 0; y < Height; y++) {
                m_ThreadPool->addWaitingTask([&Dst, &Src, Width, y](int32 /*ThreadIdx*/) {
                    xTestYUVtoRGBtoYUV_FLT(Dst, Src, y, y + 1, 0, Width);
                });
            }
            m_ThreadPool->waitUntilTasksFinished(Height);
            break;

        case eMode::TILES4: {
            const int32 TileSize = 1 << (int32) ProcMode;
            for (int32 y = 0; y < Height; y += TileSize) {
                for (int32 x = 0; x < Width; x += TileSize) {
                    m_ThreadPool->addWaitingTask([&Dst, &Src, x, y](int32 /*ThreadIdx*/) {
                        xTestYUVtoRGBtoYUV_FLT(Dst, Src, y, y + 4, x, x + 4);
                    });
                }
            }
            m_ThreadPool->waitUntilTasksFinished(Width * Height/16); //4*4
            break;
        }
        case eMode::TILES8:
        case eMode::TILES16:
        case eMode::TILES32:
        case eMode::TILES64:
        case eMode::TILES128:
        case eMode::TILES256:
        case eMode::TILES512:
        case eMode::TILES1024: {
            const int32 TileSize = 1 << (int32) ProcMode;
            createTasks(Src, Dst, Height, Width, TileSize, true);
            break;
        }

        default:
            assert(0);
            break;
    }
}

void xSepia_STD::testYUVtoRGBtoYUV_INT(xPic &Dst, const xPic &Src, eMode ProcMode) {
    const int32 Width = Dst.getWidth();
    const int32 Height = Dst.getHeight();

    switch (ProcMode) {
        case eMode::SERIAL:
            xTestYUVtoRGBtoYUV_INT(Dst, Src, 0, Height, 0, Width);
            break;

        case eMode::PIXELS:
            for (int32 y = 0; y < Height; y++) {
                for (int32 x = 0; x < Width; x++) {
                    m_ThreadPool->addWaitingTask([&Dst, &Src, x, y](int32 /*ThreadIdx*/) {
                        xTestYUVtoRGBtoYUV_INT(Dst, Src, y, y + 1, x, x + 1);
                    });
                }
            }
            m_ThreadPool->waitUntilTasksFinished(Width * Height);
            break;

        case eMode::ROWS:
            for (int32 y = 0; y < Height; y++) {
                m_ThreadPool->addWaitingTask([&Dst, &Src, Width, y](int32 /*ThreadIdx*/) {
                    xTestYUVtoRGBtoYUV_INT(Dst, Src, y, y + 1, 0, Width);
                });
            }
            m_ThreadPool->waitUntilTasksFinished(Height);
            break;

        case eMode::TILES4: {
            const int32 TileSize = 1 << (int32) ProcMode;
            for (int32 y = 0; y < Height; y += TileSize) {
                for (int32 x = 0; x < Width; x += TileSize) {
                    m_ThreadPool->addWaitingTask([&Dst, &Src, x, y](int32 /*ThreadIdx*/) {
                        xTestYUVtoRGBtoYUV_INT(Dst, Src, y, y + 4, x, x + 4);
                    });
                }
            }
            m_ThreadPool->waitUntilTasksFinished(Width * Height/16); //4*4
            break;
        }
        case eMode::TILES8:
        case eMode::TILES16:
        case eMode::TILES32:
        case eMode::TILES64:
        case eMode::TILES128:
        case eMode::TILES256:
        case eMode::TILES512:
        case eMode::TILES1024: {
            const int32 TileSize = 1 << (int32) ProcMode;
            createTasks_butINT(Src, Dst, Height, Width, TileSize, true);
            break;
        }

        default:
            assert(0);
            break;
    }
}

void xSepia_STD::testYUVtoRGBtoYUV_SSE(xPic &Dst, const xPic &Src, eMode ProcMode) {
    const int32 Width = Dst.getWidth();
    const int32 Height = Dst.getHeight();

    switch (ProcMode) {
        case eMode::SERIAL:
            xTestYUVtoRGBtoYUV_SSE(Dst, Src, 0, Height, 0, Width);
            break;

        case eMode::PIXELS:
            for (int32 y = 0; y < Height; y++) {
                for (int32 x = 0; x < Width; x++) {
                    m_ThreadPool->addWaitingTask([&Dst, &Src, x, y](int32 /*ThreadIdx*/) {
                        xTestYUVtoRGBtoYUV_SSE(Dst, Src, y, y + 1, x, x + 1);
                    });
                }
            }
            m_ThreadPool->waitUntilTasksFinished(Width * Height);
            break;

        case eMode::ROWS:
            for (int32 y = 0; y < Height; y++) {
                m_ThreadPool->addWaitingTask([&Dst, &Src, Width, y](int32 /*ThreadIdx*/) {
                    xTestYUVtoRGBtoYUV_SSE(Dst, Src, y, y + 1, 0, Width);
                });
            }
            m_ThreadPool->waitUntilTasksFinished(Height);
            break;

        case eMode::TILES4: {
            const int32 TileSize = 1 << (int32) ProcMode;
            for (int32 y = 0; y < Height; y += TileSize) {
                for (int32 x = 0; x < Width; x += TileSize) {
                    m_ThreadPool->addWaitingTask([&Dst, &Src, x, y](int32 /*ThreadIdx*/) {
                        xTestYUVtoRGBtoYUV_SSE(Dst, Src, y, y + 4, x, x + 4);
                    });
                }
            }
            m_ThreadPool->waitUntilTasksFinished(Width * Height/16); //4*4
            break;
        }
        case eMode::TILES8:
        case eMode::TILES16:
        case eMode::TILES32:
        case eMode::TILES64:
        case eMode::TILES128:
        case eMode::TILES256:
        case eMode::TILES512:
        case eMode::TILES1024: {
            const int32 TileSize = 1 << (int32) ProcMode;
            createTasks_butSSE(Src, Dst, Height, Width, TileSize, true);
            break;
        }

        default:
            assert(0);
            break;
    }
}

void xSepia_STD::applySepiaEffect_FLT(xPic &Dst, const xPic &Src, eMode ProcMode) {
    const int32 Width = Dst.getWidth();
    const int32 Height = Dst.getHeight();

    switch (ProcMode) {
        case eMode::SERIAL:
            xApplySepiaEffect_FLT(Dst, Src, 0, Height, 0, Width);
            break;

        case eMode::PIXELS:
            for (int32 y = 0; y < Height; y++) {
                for (int32 x = 0; x < Width; x++) {
                    m_ThreadPool->addWaitingTask([&Dst, &Src, x, y](int32 /*ThreadIdx*/) {
                        xApplySepiaEffect_FLT(Dst, Src, y, y + 1, x, x + 1);
                    });
                }
            }
            m_ThreadPool->waitUntilTasksFinished(Width * Height);
            break;

        case eMode::ROWS:
            for (int32 y = 0; y < Height; y++) {
                m_ThreadPool->addWaitingTask([&Dst, &Src, Width, y](int32 /*ThreadIdx*/) {
                    xApplySepiaEffect_FLT(Dst, Src, y, y + 1, 0, Width);
                });
            }
            m_ThreadPool->waitUntilTasksFinished(Height);
            break;

        case eMode::TILES4:
        case eMode::TILES8:
        case eMode::TILES16:
        case eMode::TILES32:
        case eMode::TILES64:
        case eMode::TILES128:
        case eMode::TILES256:
        case eMode::TILES512:
        case eMode::TILES1024: {
            const int32 TileSize = 1 << (int32) ProcMode;
            createTasks(Src, Dst, Height, Width, TileSize, false);
            break;
        }
        default:
            assert(0);
            break;
    }
}

void xSepia_STD::applySepiaEffect_INT(xPic &Dst, const xPic &Src, eMode ProcMode) {
    const int32 Width = Dst.getWidth();
    const int32 Height = Dst.getHeight();

    switch (ProcMode) {
        case eMode::SERIAL:
            xApplySepiaEffect_INT(Dst, Src, 0, Height, 0, Width);
            break;

        case eMode::PIXELS:
            for (int32 y = 0; y < Height; y++) {
                for (int32 x = 0; x < Width; x++) {
                    m_ThreadPool->addWaitingTask([&Dst, &Src, x, y](int32 /*ThreadIdx*/) {
                        xApplySepiaEffect_INT(Dst, Src, y, y + 1, x, x + 1);
                    });
                }
            }
            m_ThreadPool->waitUntilTasksFinished(Width * Height);
            break;

        case eMode::ROWS:
            for (int32 y = 0; y < Height; y++) {
                m_ThreadPool->addWaitingTask([&Dst, &Src, Width, y](int32 /*ThreadIdx*/) {
                    xApplySepiaEffect_INT(Dst, Src, y, y + 1, 0, Width);
                });
            }
            m_ThreadPool->waitUntilTasksFinished(Height);
            break;

        case eMode::TILES4:
        case eMode::TILES8:
        case eMode::TILES16:
        case eMode::TILES32:
        case eMode::TILES64:
        case eMode::TILES128:
        case eMode::TILES256:
        case eMode::TILES512:
        case eMode::TILES1024: {
            const int32 TileSize = 1 << (int32) ProcMode;
            createTasks_butINT(Src, Dst, Height, Width, TileSize, false);
            break;
        }
        default:
            assert(0);
            break;
    }
}

void xSepia_STD::applySepiaEffect_SSE(xPic &Dst, const xPic &Src, eMode ProcMode) {
    const int32 Width = Dst.getWidth();
    const int32 Height = Dst.getHeight();

    switch (ProcMode) {
        case eMode::SERIAL:
            xApplySepiaEffect_SSE(Dst, Src, 0, Height, 0, Width);
            break;

        case eMode::PIXELS:
            for (int32 y = 0; y < Height; y++) {
                for (int32 x = 0; x < Width; x++) {
                    m_ThreadPool->addWaitingTask([&Dst, &Src, x, y](int32 /*ThreadIdx*/) {
                        xApplySepiaEffect_SSE(Dst, Src, y, y + 1, x, x + 1);
                    });
                }
            }
            m_ThreadPool->waitUntilTasksFinished(Width * Height);
            break;

        case eMode::ROWS:
            for (int32 y = 0; y < Height; y++) {
                m_ThreadPool->addWaitingTask([&Dst, &Src, Width, y](int32 /*ThreadIdx*/) {
                    xApplySepiaEffect_SSE(Dst, Src, y, y + 1, 0, Width);
                });
            }
            m_ThreadPool->waitUntilTasksFinished(Height);
            break;

        case eMode::TILES4:
        case eMode::TILES8:
        case eMode::TILES16:
        case eMode::TILES32:
        case eMode::TILES64:
        case eMode::TILES128:
        case eMode::TILES256:
        case eMode::TILES512:
        case eMode::TILES1024: {
            const int32 TileSize = 1 << (int32) ProcMode;
            createTasks_butSSE(Src, Dst, Height, Width, TileSize, false);
            break;
        }
        default:
            assert(0);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void xSepia_STD::xTestYUVtoRGBtoYUV_FLT(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());

    const int32 DstStride = Dst.getStride();
    const int32 SrcStride = Src.getStride();
    uint16 *restrict DstPtrLm = Dst.getAddr(0) + BegY * DstStride;
    uint16 *restrict DstPtrCb = Dst.getAddr(1) + BegY * DstStride;
    uint16 *restrict DstPtrCr = Dst.getAddr(2) + BegY * DstStride;
    const uint16 *SrcPtrLm = Src.getAddr(0) + BegY * SrcStride;
    const uint16 *SrcPtrCb = Src.getAddr(1) + BegY * SrcStride;
    const uint16 *SrcPtrCr = Src.getAddr(2) + BegY * SrcStride;

    const flt32 SrcMidValue = flt32(1 << (Src.getBitDepth() - 1));
    const flt32 DstMidValue = flt32(1 << (Dst.getBitDepth() - 1));
    const flt32 DstMaxValue = flt32((1 << Dst.getBitDepth()) - 1);

    for (int32 y = BegY; y < EndY; y++) {
        for (int32 x = BegX; x < EndX; x++) {
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

void xSepia_STD::xTestYUVtoRGBtoYUV_INT(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());

    const int32 DstStride = Dst.getStride();
    const int32 SrcStride = Src.getStride();
    uint16 *restrict DstPtrLm = Dst.getAddr(0) + BegY * DstStride;
    uint16 *restrict DstPtrCb = Dst.getAddr(1) + BegY * DstStride;
    uint16 *restrict DstPtrCr = Dst.getAddr(2) + BegY * DstStride;
    const uint16 *SrcPtrLm = Src.getAddr(0) + BegY * SrcStride;
    const uint16 *SrcPtrCb = Src.getAddr(1) + BegY * SrcStride;
    const uint16 *SrcPtrCr = Src.getAddr(2) + BegY * SrcStride;

    const int32 SrcMidValue = int32(1 << (Src.getBitDepth() - 1));
    const int32 DstMidValue = int32(1 << (Dst.getBitDepth() - 1));
    const int32 DstMaxValue = int32((1 << Dst.getBitDepth()) - 1);

    for (int32 y = BegY; y < EndY; y++) {
        for (int32 x = BegX; x < EndX; x++) {
            //load
            const uint16V4 SrcPixelYUV_U16 = {SrcPtrLm[x], SrcPtrCb[x], SrcPtrCr[x], 0};

            //change data format (and remove chroma offset)
            const int32V4 SrcPixelYUV_I32 = {
                    (int32) (SrcPixelYUV_U16[0] << 16),
                    (int32) (SrcPixelYUV_U16[1]) - SrcMidValue,
                    (int32) (SrcPixelYUV_U16[2]) - SrcMidValue,
                    0
            };

            //convert YCbCr --> RGB (BT.709)
            const int32V4 SrcPixelRGB_I32 = {
                    (SrcPixelYUV_I32[0] + SrcPixelYUV_I32[2] * YCbCr2RGB_I32[0][2] + 32768) >> 16,
                    (SrcPixelYUV_I32[0] + SrcPixelYUV_I32[1] * YCbCr2RGB_I32[1][1] +
                     SrcPixelYUV_I32[2] * YCbCr2RGB_I32[1][2] + 32768) >> 16,
                    (SrcPixelYUV_I32[0] + SrcPixelYUV_I32[1] * YCbCr2RGB_I32[2][1] + 32768) >> 16,
                    0
            };

            //copy RGB --> RGB
            const int32V4 DstPixelRGB_I32 = SrcPixelRGB_I32;

            //convert RGB --> YCbCr (BT.709)
            const int32V4 DstPixelYUV_I32 = {
                    (int32) (((DstPixelRGB_I32[0] * RGB2YCbCr_I32[0][0] + DstPixelRGB_I32[1] * RGB2YCbCr_I32[0][1] +
                               DstPixelRGB_I32[2] * RGB2YCbCr_I32[0][2]) + 32768) >> 16),
                    (int32) (((DstPixelRGB_I32[0] * RGB2YCbCr_I32[1][0] + DstPixelRGB_I32[1] * RGB2YCbCr_I32[1][1] +
                               DstPixelRGB_I32[2] * RGB2YCbCr_I32[1][2]) + 32768) >> 16),
                    (int32) (((DstPixelRGB_I32[0] * RGB2YCbCr_I32[2][0] + DstPixelRGB_I32[1] * RGB2YCbCr_I32[2][1] +
                               DstPixelRGB_I32[2] * RGB2YCbCr_I32[2][2]) + 32768) >> 16),
                    0
            };

            //change data format (and apply chroma offset) + clip to range 0-Max
            int32V4 TmpPixelYUV_I32 = DstPixelYUV_I32 + int32V4(0, DstMidValue, DstMidValue, 0);
            TmpPixelYUV_I32.clip(xMakeVec4<int32>(0), xMakeVec4<int32>(DstMaxValue));
            const uint16V4 DstPixelYUV_U16 = uint16V4(TmpPixelYUV_I32);

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

void xSepia_STD::xTestYUVtoRGBtoYUV_SSE(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());

    const int32 DstStride = Dst.getStride();
    const int32 SrcStride = Src.getStride();
    uint16 *restrict DstPtrLm = Dst.getAddr(0) + BegY * DstStride;
    uint16 *restrict DstPtrCb = Dst.getAddr(1) + BegY * DstStride;
    uint16 *restrict DstPtrCr = Dst.getAddr(2) + BegY * DstStride;
    const uint16 *SrcPtrLm = Src.getAddr(0) + BegY * SrcStride;
    const uint16 *SrcPtrCb = Src.getAddr(1) + BegY * SrcStride;
    const uint16 *SrcPtrCr = Src.getAddr(2) + BegY * SrcStride;

    const int32 SrcMidValue = int32(1 << (Src.getBitDepth() - 1));
    const int32 DstMidValue = int32(1 << (Dst.getBitDepth() - 1));
    const int32 DstMaxValue = int32((1 << Dst.getBitDepth()) - 1);

    const __m128i SrcMidValueV_I32 = _mm_set1_epi32(SrcMidValue);
    const __m128i DstMidValueV_I32 = _mm_set1_epi32(DstMidValue);
    const __m128i DstMaxValueV_U16 = _mm_set1_epi16((uint16) DstMaxValue);

    //WARNING - for demonstration purposes only - it isn't the fastest way to copy data
    for (int32 y = BegY; y < EndY; y++) {
        for (int32 x = BegX; x < EndX; x++) {
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
}

void xSepia_STD::xApplySepiaEffect_FLT(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());

    const int32 DstStride = Dst.getStride();
    const int32 SrcStride = Src.getStride();
    uint16 *restrict DstPtrLm = Dst.getAddr(0) + BegY * DstStride;
    uint16 *restrict DstPtrCb = Dst.getAddr(1) + BegY * DstStride;
    uint16 *restrict DstPtrCr = Dst.getAddr(2) + BegY * DstStride;
    const uint16 *SrcPtrLm = Src.getAddr(0) + BegY * SrcStride;
    const uint16 *SrcPtrCb = Src.getAddr(1) + BegY * SrcStride;
    const uint16 *SrcPtrCr = Src.getAddr(2) + BegY * SrcStride;

    const flt32 SrcMidValue = flt32(1 << (Src.getBitDepth() - 1));
    const flt32 DstMidValue = flt32(1 << (Dst.getBitDepth() - 1));
    const flt32 DstMaxValue = flt32((1 << Dst.getBitDepth()) - 1);

    for (int32 y = BegY; y < EndY; y++) {
        for (int32 x = BegX; x < EndX; x++) {
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

void xSepia_STD::xApplySepiaEffect_INT(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());

    const int32 DstStride = Dst.getStride();
    const int32 SrcStride = Src.getStride();
    uint16 *restrict DstPtrLm = Dst.getAddr(0) + BegY * DstStride;
    uint16 *restrict DstPtrCb = Dst.getAddr(1) + BegY * DstStride;
    uint16 *restrict DstPtrCr = Dst.getAddr(2) + BegY * DstStride;
    const uint16 *SrcPtrLm = Src.getAddr(0) + BegY * SrcStride;
    const uint16 *SrcPtrCb = Src.getAddr(1) + BegY * SrcStride;
    const uint16 *SrcPtrCr = Src.getAddr(2) + BegY * SrcStride;

    const int32 SrcMidValue = int32(1 << (Src.getBitDepth() - 1));
    const int32 DstMidValue = int32(1 << (Dst.getBitDepth() - 1));
    const int32 DstMaxValue = int32((1 << Dst.getBitDepth()) - 1);

    for (int32 y = BegY; y < EndY; y++) {
        for (int32 x = BegX; x < EndX; x++) {
//load
            const uint16V4 SrcPixelYUV_U16 = {SrcPtrLm[x], SrcPtrCb[x], SrcPtrCr[x], 0};

            //change data format (and remove chroma offset)
            const int32V4 SrcPixelYUV_I32 = {
                    (int32) (SrcPixelYUV_U16[0] << 16),
                    (int32) (SrcPixelYUV_U16[1]) - SrcMidValue,
                    (int32) (SrcPixelYUV_U16[2]) - SrcMidValue,
                    0
            };

            //convert YCbCr --> RGB (BT.709)
            const int32V4 SrcPixelRGB_I32 = {
                    (SrcPixelYUV_I32[0] + SrcPixelYUV_I32[2] * YCbCr2RGB_I32[0][2] + 32768) >> 16,
                    (SrcPixelYUV_I32[0] + SrcPixelYUV_I32[1] * YCbCr2RGB_I32[1][1] +
                     SrcPixelYUV_I32[2] * YCbCr2RGB_I32[1][2] + 32768) >> 16,
                    (SrcPixelYUV_I32[0] + SrcPixelYUV_I32[1] * YCbCr2RGB_I32[2][1] + 32768) >> 16,
                    0
            };

            //copy RGB --> RGB
            const int32V4 DstPixelRGB_I32 =
                    {
                            (SrcPixelRGB_I32[0] * SEPIA_I32[0][0] + SrcPixelRGB_I32[1] * SEPIA_I32[0][1] +
                             SrcPixelRGB_I32[2] * SEPIA_I32[0][2] + 32768) >> 16,
                            (SrcPixelRGB_I32[0] * SEPIA_I32[1][0] + SrcPixelRGB_I32[1] * SEPIA_I32[1][1] +
                             SrcPixelRGB_I32[2] * SEPIA_I32[1][2] + 32768) >> 16,
                            (SrcPixelRGB_I32[0] * SEPIA_I32[2][0] + SrcPixelRGB_I32[1] * SEPIA_I32[2][1] +
                             SrcPixelRGB_I32[2] * SEPIA_I32[2][2] + 32768) >> 16,
                            0
                    };

            //convert RGB --> YCbCr (BT.709)
            const int32V4 DstPixelYUV_I32 = {
                    (int32) (((DstPixelRGB_I32[0] * RGB2YCbCr_I32[0][0] + DstPixelRGB_I32[1] * RGB2YCbCr_I32[0][1] +
                               DstPixelRGB_I32[2] * RGB2YCbCr_I32[0][2]) + 32768) >> 16),
                    (int32) (((DstPixelRGB_I32[0] * RGB2YCbCr_I32[1][0] + DstPixelRGB_I32[1] * RGB2YCbCr_I32[1][1] +
                               DstPixelRGB_I32[2] * RGB2YCbCr_I32[1][2]) + 32768) >> 16),
                    (int32) (((DstPixelRGB_I32[0] * RGB2YCbCr_I32[2][0] + DstPixelRGB_I32[1] * RGB2YCbCr_I32[2][1] +
                               DstPixelRGB_I32[2] * RGB2YCbCr_I32[2][2]) + 32768) >> 16),
                    0
            };

            //change data format (and apply chroma offset) + clip to range 0-Max
            int32V4 TmpPixelYUV_I32 = DstPixelYUV_I32 + int32V4(0, DstMidValue, DstMidValue, 0);
            TmpPixelYUV_I32.clip(xMakeVec4<int32>(0), xMakeVec4<int32>(DstMaxValue));
            const uint16V4 DstPixelYUV_U16 = uint16V4(TmpPixelYUV_I32);

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


void xSepia_STD::xApplySepiaEffect_SSE(xPic &Dst, const xPic &Src, int32 BegY, int32 EndY, int32 BegX, int32 EndX) {
    assert(Dst.getWidth() == Src.getWidth() && Dst.getHeight() == Src.getHeight() &&
           Dst.getStride() == Src.getStride());

    const int32 DstStride = Dst.getStride();
    const int32 SrcStride = Src.getStride();
    uint16 *restrict DstPtrLm = Dst.getAddr(0) + BegY * DstStride;
    uint16 *restrict DstPtrCb = Dst.getAddr(1) + BegY * DstStride;
    uint16 *restrict DstPtrCr = Dst.getAddr(2) + BegY * DstStride;
    const uint16 *SrcPtrLm = Src.getAddr(0) + BegY * SrcStride;
    const uint16 *SrcPtrCb = Src.getAddr(1) + BegY * SrcStride;
    const uint16 *SrcPtrCr = Src.getAddr(2) + BegY * SrcStride;

    const int32 SrcMidValue = int32(1 << (Src.getBitDepth() - 1));
    const int32 DstMidValue = int32(1 << (Dst.getBitDepth() - 1));
    const int32 DstMaxValue = int32((1 << Dst.getBitDepth()) - 1);

    const __m128i SrcMidValueV_I32 = _mm_set1_epi32(SrcMidValue);
    const __m128i DstMidValueV_I32 = _mm_set1_epi32(DstMidValue);
    const __m128i DstMaxValueV_U16 = _mm_set1_epi16((uint16) DstMaxValue);

    //WARNING - for demonstration purposes only - it isn't the fastest way to copy data
    for (int32 y = BegY; y < EndY; y++) {
        for (int32 x = BegX; x < EndX; x++) {
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
}
//===============================================================================================================================================================================================================
