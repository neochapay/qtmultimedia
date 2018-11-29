/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
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
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "camerabincontrol.h"
#include "camerabincontainer.h"
#include "camerabinaudioencoder.h"
#include "camerabinvideoencoder.h"
#include "camerabinimageencoder.h"
#include "camerabinresourcepolicy.h"

#include <QtCore/qdebug.h>
#include <QtCore/qfile.h>
#include <QtCore/qmetaobject.h>

QT_BEGIN_NAMESPACE

//#define CAMEABIN_DEBUG 1
#define ENUM_NAME(c,e,v) (c::staticMetaObject.enumerator(c::staticMetaObject.indexOfEnumerator(e)).valueToKey((v)))

CameraBinControl::CameraBinControl(CameraBinSession *session)
    :QCameraControl(session),
    m_session(session)
{
    connect(m_session, SIGNAL(statusChanged(QCamera::Status)),
            this, SIGNAL(statusChanged(QCamera::Status)));
}

CameraBinControl::~CameraBinControl()
{
}

QCamera::CaptureModes CameraBinControl::captureMode() const
{
    return m_session->captureMode();
}

void CameraBinControl::setCaptureMode(QCamera::CaptureModes mode)
{
    m_session->setCaptureMode(mode);
}

bool CameraBinControl::isCaptureModeSupported(QCamera::CaptureModes mode) const
{
    return mode == QCamera::CaptureStillImage || mode == QCamera::CaptureVideo;
}

void CameraBinControl::setState(QCamera::State state)
{
#ifdef CAMEABIN_DEBUG
    qDebug() << Q_FUNC_INFO << ENUM_NAME(QCamera, "State", state);
#endif
    m_session->setState(state);
}

QCamera::State CameraBinControl::state() const
{
    return m_session->requestedState();
}

QCamera::Status CameraBinControl::status() const
{
    return m_session->status();
}

bool CameraBinControl::canChangeProperty(PropertyChangeType changeType, QCamera::Status status) const
{
    Q_UNUSED(status);

    switch (changeType) {
    case QCameraControl::Viewfinder:
        return true;
    case QCameraControl::CaptureMode:
    case QCameraControl::ImageEncodingSettings:
    case QCameraControl::VideoEncodingSettings:
    case QCameraControl::ViewfinderSettings:
    default:
        return status != QCamera::ActiveStatus;
    }
}

#define VIEWFINDER_COLORSPACE_CONVERSION 0x00000004

bool CameraBinControl::viewfinderColorSpaceConversion() const
{
    gint flags = 0;
    g_object_get(G_OBJECT(m_session->cameraBin()), "flags", &flags, NULL);

    return flags & VIEWFINDER_COLORSPACE_CONVERSION;
}

void CameraBinControl::setViewfinderColorSpaceConversion(bool enabled)
{
    gint flags = 0;
    g_object_get(G_OBJECT(m_session->cameraBin()), "flags", &flags, NULL);

    if (enabled)
        flags |= VIEWFINDER_COLORSPACE_CONVERSION;
    else
        flags &= ~VIEWFINDER_COLORSPACE_CONVERSION;

    g_object_set(G_OBJECT(m_session->cameraBin()), "flags", flags, NULL);
}

QT_END_NAMESPACE
