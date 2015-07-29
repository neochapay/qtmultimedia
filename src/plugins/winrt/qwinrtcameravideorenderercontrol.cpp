/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd and/or its subsidiary(-ies).
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwinrtcameravideorenderercontrol.h"

#include <QtCore/qfunctions_winrt.h>
#include <QtCore/QSize>
#include <QtCore/QVector>
#include <QVideoFrame>

#include <d3d11.h>
#include <mfapi.h>
#include <wrl.h>
using namespace Microsoft::WRL;

QT_BEGIN_NAMESPACE

class QWinRTCameraVideoBuffer : public QAbstractVideoBuffer
{
public:
    QWinRTCameraVideoBuffer(IMF2DBuffer *buffer, int size)
        : QAbstractVideoBuffer(NoHandle)
        , currentMode(NotMapped)
        , buffer(buffer)
        , size(size)
    {
    }

    ~QWinRTCameraVideoBuffer()
    {
        unmap();
    }

    MapMode mapMode() const Q_DECL_OVERRIDE
    {
        return currentMode;
    }

    uchar *map(MapMode mode, int *numBytes, int *bytesPerLine) Q_DECL_OVERRIDE
    {
        if (currentMode != NotMapped || mode == NotMapped)
            return nullptr;

        BYTE *bytes;
        LONG stride;
        HRESULT hr = buffer->Lock2D(&bytes, &stride);
        RETURN_IF_FAILED("Failed to lock camera frame buffer", nullptr);

        if (bytesPerLine)
            *bytesPerLine = stride;
        if (numBytes)
            *numBytes = size;
        currentMode = mode;
        return bytes;
    }

    void unmap() Q_DECL_OVERRIDE
    {
        if (currentMode == NotMapped)
            return;
        HRESULT hr = buffer->Unlock2D();
        RETURN_VOID_IF_FAILED("Failed to unlock camera frame buffer");
        currentMode = NotMapped;
    }

private:
    ComPtr<IMF2DBuffer> buffer;
    MapMode currentMode;
    int size;
};

class D3DVideoBlitter
{
public:
    D3DVideoBlitter(ID3D11Device *device, ID3D11Texture2D *target)
        : m_d3dDevice(device), m_target(target)
    {
        HRESULT hr;
        ComPtr<IDXGIResource> targetResource;
        hr = target->QueryInterface(IID_PPV_ARGS(&targetResource));
        Q_ASSERT_SUCCEEDED(hr);
        HANDLE sharedHandle;
        hr = targetResource->GetSharedHandle(&sharedHandle);
        Q_ASSERT_SUCCEEDED(hr);
        hr = m_d3dDevice->OpenSharedResource(sharedHandle, IID_PPV_ARGS(&m_targetTexture));
        Q_ASSERT_SUCCEEDED(hr);
        hr = m_d3dDevice.As(&m_videoDevice);
        Q_ASSERT_SUCCEEDED(hr);
    }

    ID3D11Device *device() const
    {
        return m_d3dDevice.Get();
    }

    ID3D11Texture2D *target() const
    {
        return m_target;
    }

    void blit(ID3D11Texture2D *texture)
    {
        HRESULT hr;
        D3D11_TEXTURE2D_DESC desc;
        texture->GetDesc(&desc);
        if (!m_videoEnumerator) {
            D3D11_VIDEO_PROCESSOR_CONTENT_DESC videoProcessorDesc = {
                D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE,
                { 0 }, desc.Width, desc.Height,
                { 0 }, desc.Width, desc.Height,
                D3D11_VIDEO_USAGE_PLAYBACK_NORMAL
            };
            hr = m_videoDevice->CreateVideoProcessorEnumerator(&videoProcessorDesc, &m_videoEnumerator);
            RETURN_VOID_IF_FAILED("Failed to create video enumerator");
        }

        if (!m_videoProcessor) {
            hr = m_videoDevice->CreateVideoProcessor(m_videoEnumerator.Get(), 0, &m_videoProcessor);
            RETURN_VOID_IF_FAILED("Failed to create video processor");
        }

        if (!m_outputView) {
            D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC outputDesc = { D3D11_VPOV_DIMENSION_TEXTURE2D };
            hr = m_videoDevice->CreateVideoProcessorOutputView(
                        m_targetTexture.Get(), m_videoEnumerator.Get(), &outputDesc, &m_outputView);
            RETURN_VOID_IF_FAILED("Failed to create video output view");
        }

        D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputViewDesc = {
            0, D3D11_VPIV_DIMENSION_TEXTURE2D, { 0, 0 }
        };
        ComPtr<ID3D11VideoProcessorInputView> inputView;
        hr = m_videoDevice->CreateVideoProcessorInputView(
                    texture, m_videoEnumerator.Get(), &inputViewDesc, &inputView);
        RETURN_VOID_IF_FAILED("Failed to create video input view");

        ComPtr<ID3D11DeviceContext> context;
        ComPtr<ID3D11VideoContext> videoContext;
        m_d3dDevice->GetImmediateContext(&context);
        hr = context.As(&videoContext);
        RETURN_VOID_IF_FAILED("Failed to get video context");

        D3D11_VIDEO_PROCESSOR_STREAM stream = { TRUE };
        stream.pInputSurface = inputView.Get();
        hr = videoContext->VideoProcessorBlt(
                    m_videoProcessor.Get(), m_outputView.Get(), 0, 1, &stream);
        RETURN_VOID_IF_FAILED("Failed to get blit video frame");
    }

private:
    ComPtr<ID3D11Device> m_d3dDevice;
    ComPtr<ID3D11Texture2D> m_targetTexture;
    ID3D11Texture2D *m_target;
    ComPtr<ID3D11VideoDevice> m_videoDevice;
    ComPtr<ID3D11VideoProcessorEnumerator> m_videoEnumerator;
    ComPtr<ID3D11VideoProcessor> m_videoProcessor;
    ComPtr<ID3D11VideoProcessorOutputView> m_outputView;
};

#define CAMERA_SAMPLE_QUEUE_SIZE 5
class QWinRTCameraVideoRendererControlPrivate
{
public:
    QScopedPointer<D3DVideoBlitter> blitter;
    ComPtr<IMF2DBuffer> buffers[CAMERA_SAMPLE_QUEUE_SIZE];
    QAtomicInteger<quint16> writeIndex;
    QAtomicInteger<quint16> readIndex;
    QVideoFrame::PixelFormat cameraSampleformat;
    int cameraSampleSize;
    uint videoProbesCounter;
    bool getCameraSampleInfo(const ComPtr<IMF2DBuffer> &buffer);
    ComPtr<IMF2DBuffer> dequeueBuffer();
};

bool QWinRTCameraVideoRendererControlPrivate::getCameraSampleInfo(const ComPtr<IMF2DBuffer> &buffer)
{
    ComPtr<ID3D11Texture2D> sourceTexture;
    ComPtr<IMFDXGIBuffer> dxgiBuffer;
    HRESULT hr = buffer.As(&dxgiBuffer);
    Q_ASSERT_SUCCEEDED(hr);
    hr = dxgiBuffer->GetResource(IID_PPV_ARGS(&sourceTexture));
    if (FAILED(hr)) {
        qErrnoWarning(hr, "The video frame does not support texture output");
        cameraSampleformat = QVideoFrame::Format_Invalid;
        return false;
    }
    D3D11_TEXTURE2D_DESC desc;
    sourceTexture->GetDesc(&desc);
    switch (desc.Format) {
    case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        cameraSampleformat = QVideoFrame::Format_ARGB32;
        break;
    case DXGI_FORMAT_NV12:
        cameraSampleformat = QVideoFrame::Format_NV12;
        break;
    default:
        cameraSampleformat = QVideoFrame::Format_Invalid;
        qErrnoWarning("Unsupported camera probe format.");
        return false;
    }
    DWORD pcbLength;
    hr = buffer->GetContiguousLength(&pcbLength);
    Q_ASSERT_SUCCEEDED(hr);
    cameraSampleSize = pcbLength;
    return true;
}

QWinRTCameraVideoRendererControl::QWinRTCameraVideoRendererControl(const QSize &size, QObject *parent)
    : QWinRTAbstractVideoRendererControl(size, parent), d_ptr(new QWinRTCameraVideoRendererControlPrivate)
{
    Q_D(QWinRTCameraVideoRendererControl);
    d->cameraSampleformat = QVideoFrame::Format_User;
    d->videoProbesCounter = 0;
}

QWinRTCameraVideoRendererControl::~QWinRTCameraVideoRendererControl()
{
    shutdown();
}

bool QWinRTCameraVideoRendererControl::render(ID3D11Texture2D *target)
{
    Q_D(QWinRTCameraVideoRendererControl);
    ComPtr<IMF2DBuffer> buffer = d->dequeueBuffer();
    if (!buffer) {
        emit bufferRequested();
        return false;
    }

    ComPtr<ID3D11Texture2D> sourceTexture;
    ComPtr<IMFDXGIBuffer> dxgiBuffer;
    HRESULT hr = buffer.As(&dxgiBuffer);
    Q_ASSERT_SUCCEEDED(hr);
    hr = dxgiBuffer->GetResource(IID_PPV_ARGS(&sourceTexture));
    if (FAILED(hr)) {
        qErrnoWarning(hr, "The video frame does not support texture output; aborting rendering.");
        return false;
    }

    ComPtr<ID3D11Device> device;
    sourceTexture->GetDevice(&device);
    if (!d->blitter || d->blitter->device() != device.Get() || d->blitter->target() != target)
        d->blitter.reset(new D3DVideoBlitter(device.Get(), target));

    d->blitter->blit(sourceTexture.Get());

    emit bufferRequested();
    return true;
}

void QWinRTCameraVideoRendererControl::queueBuffer(IMF2DBuffer *buffer)
{
    Q_D(QWinRTCameraVideoRendererControl);
    Q_ASSERT(buffer);

    if (d->videoProbesCounter > 0) {
        if (d->cameraSampleformat == QVideoFrame::Format_User)
            d->getCameraSampleInfo(buffer);

        if (d->cameraSampleformat != QVideoFrame::Format_Invalid) {
            QWinRTCameraVideoBuffer *videoBuffer = new QWinRTCameraVideoBuffer(buffer, d->cameraSampleSize);
            QVideoFrame frame(videoBuffer, size(), d->cameraSampleformat);
            emit videoFrameProbed(frame);
        }
    }

    const quint16 writeIndex = (d->writeIndex + 1) % CAMERA_SAMPLE_QUEUE_SIZE;
    if (d->readIndex == writeIndex) // Drop new sample if queue is full
        return;
    d->buffers[d->writeIndex] = buffer;
    d->writeIndex = writeIndex;

    if (!surface()) {
        d->dequeueBuffer();
        emit bufferRequested();
    }
}

ComPtr<IMF2DBuffer> QWinRTCameraVideoRendererControlPrivate::dequeueBuffer()
{
    const quint16 currentReadIndex = readIndex;
    if (currentReadIndex == writeIndex)
        return nullptr;

    ComPtr<IMF2DBuffer> buffer = buffers[currentReadIndex];
    Q_ASSERT(buffer);
    buffers[currentReadIndex].Reset();
    readIndex = (currentReadIndex + 1) % CAMERA_SAMPLE_QUEUE_SIZE;
    return buffer;
}

void QWinRTCameraVideoRendererControl::discardBuffers()
{
    Q_D(QWinRTCameraVideoRendererControl);
    d->writeIndex = d->readIndex = 0;
    for (ComPtr<IMF2DBuffer> &buffer : d->buffers)
        buffer.Reset();
}

void QWinRTCameraVideoRendererControl::incrementProbe()
{
    Q_D(QWinRTCameraVideoRendererControl);
    ++d->videoProbesCounter;
}

void QWinRTCameraVideoRendererControl::decrementProbe()
{
    Q_D(QWinRTCameraVideoRendererControl);
    Q_ASSERT(d->videoProbesCounter > 0);
    --d->videoProbesCounter;
}

QT_END_NAMESPACE
