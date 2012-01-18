/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qdeclarative_audiocategory_p.h"
#include "qdebug.h"

#define DEBUG_AUDIOENGINE

QT_USE_NAMESPACE

/*!
    \qmlclass AudioCategory QDeclarativeAudioCategory
    \since 5.0
    \brief The AudioCategory element allows you to control all active sound instances by group
    \ingroup qml-multimedia
    \inherits Item

    This element is part of the \bold{QtAudioEngine 1.0} module.

    AudioCategory element can be accessed through AudioEngine::categories with its unique name and
    must be defined inside AudioEngine.

    \qml
    import QtQuick 2.0
    import QtAudioEngine 1.0

    Rectangle {
        color:"white"
        width: 300
        height: 500

        AudioEngine {
            id:audioengine

            AudioCategory {
                name: "sfx"
                volume: 0.8
            }

            AudioSample {
                name:"explosion"
                source: "explosion-02.wav"
            }

            Sound {
                name:"explosion"
                category: "sfx"
                PlayVariation {
                    sample:"explosion"
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            onPressed: {
                audioengine.categories["sfx"].volume = 0.5;
            }
        }
    }
    \endqml

    Sound elements can be grouped togather by specifying the category property. When you change the
    volume of a category, all audio output from related elements will be affected as well.

    Note: there will always be an AudioCategory named \c default whether you explicitly define it or
    not. If you do not specify any category for a Sound element, it will be grouped into the \c default
    category.

*/
QDeclarativeAudioCategory::QDeclarativeAudioCategory(QObject *parent)
    : QObject(parent)
    , m_complete(false)
    , m_volume(1)
{
}

QDeclarativeAudioCategory::~QDeclarativeAudioCategory()
{
}

void QDeclarativeAudioCategory::classBegin()
{
    if (!parent() || !parent()->inherits("QDeclarativeAudioEngine")) {
        qWarning("AudioCategory must be defined inside AudioEngine!");
        return;
    }
}

void QDeclarativeAudioCategory::componentComplete()
{
    if (m_name.isEmpty()) {
        qWarning("AudioCategory must have a name!");
        return;
    }
    m_complete = true;
}

/*!
    \qmlproperty real AudioCategory::volume

    This property holds the volume of the category and will modulate all audio output from the
    element which belongs to this category.
*/
qreal QDeclarativeAudioCategory::volume() const
{
    return m_volume;
}

void QDeclarativeAudioCategory::setVolume(qreal volume)
{
    if (m_volume == volume)
        return;
    m_volume = volume;
    emit volumeChanged(m_volume);
#ifdef DEBUG_AUDIOENGINE
    qDebug() << "QDeclarativeAudioCategory[" << m_name << "] setVolume(" << volume << ")";
#endif
}

/*!
    \qmlproperty string AudioCategory::name

    This property holds the name of AudioCategory. The name must be unique among all categories and only
    defined once.
*/
void QDeclarativeAudioCategory::setName(const QString& name)
{
    if (m_complete) {
        qWarning("AudioCategory: you can not change name after initialization.");
        return;
    }
    m_name = name;
}

QString QDeclarativeAudioCategory::name() const
{
    return m_name;
}

/*!
    \qmlmethod AudioCategory::stop()

    Stops all active sound instances which belong to this category.
*/
void QDeclarativeAudioCategory::stop()
{
    emit stopped();
}

/*!
    \qmlmethod AudioCategory::pause()

    Pauses all active sound instances which belong to this category.
*/
void QDeclarativeAudioCategory::pause()
{
    emit paused();
}

/*!
    \qmlmethod AudioCategory::pause()

    Resumes all active sound instances from paused state which belong to this category.
*/
void QDeclarativeAudioCategory::resume()
{
    emit resumed();
}