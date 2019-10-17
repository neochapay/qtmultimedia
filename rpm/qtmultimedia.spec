Name:       qt5-qtmultimedia
Summary:    Qt Multimedia module
Version:    5.6.2
Release:    1%{?dist}
Group:      Qt/Qt
License:    LGPLv2.1 with exception or GPLv3
URL:        http://qt.nokia.com
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  qt5-qtcore-devel
BuildRequires:  qt5-qtgui-devel
BuildRequires:  qt5-qtnetwork-devel
BuildRequires:  qt5-qtdeclarative-devel
BuildRequires:  qt5-qttest-devel
BuildRequires:  qt5-qtdeclarative-qtquick-devel
BuildRequires:  qt5-qmake
BuildRequires:  pkgconfig(alsa)
BuildRequires:  fdupes
BuildRequires:  pkgconfig(libpulse)
BuildRequires:  pkgconfig(libpulse-mainloop-glib)
BuildRequires:  pkgconfig(gstreamer-1.0)
BuildRequires:  pkgconfig(gstreamer-base-1.0)
BuildRequires:  pkgconfig(gstreamer-audio-1.0)
BuildRequires:  pkgconfig(gstreamer-video-1.0)
BuildRequires:  pkgconfig(gstreamer-pbutils-1.0)
BuildRequires:  pkgconfig(gstreamer-app-1.0)
BuildRequires:  pkgconfig(gstreamer-plugins-bad-1.0) >= 1.14.1
BuildRequires:  pkgconfig(libresourceqt5) >= 1.29
BuildRequires:  pkgconfig(Qt5QuickTest)

%description
Qt is a cross-platform application and UI framework. Using Qt, you can
write web-enabled applications once and deploy them across desktop,
mobile and embedded systems without rewriting the source code.
.
This package contains the QtMultimedia module


%package devel
Summary:    Qt Multimedia - development files
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}
Requires:   qt5-qtmultimedia-plugin-audio-alsa = %{version}-%{release}
Requires:   qt5-qtmultimedia-plugin-audio-pulseaudio = %{version}-%{release}
Requires:   qt5-qtmultimedia-plugin-mediaservice-gstaudiodecoder = %{version}-%{release}
Requires:   qt5-qtmultimedia-plugin-mediaservice-gstcamerabin = %{version}-%{release}
Requires:   qt5-qtmultimedia-plugin-mediaservice-gstmediacapture = %{version}-%{release}
Requires:   qt5-qtmultimedia-plugin-mediaservice-gstmediaplayer = %{version}-%{release}
Requires:   qt5-qtmultimedia-plugin-playlistformats-m3u = %{version}-%{release}
Requires:   qt5-qtmultimedia-plugin-resourcepolicy-resourceqt = %{version}-%{release}

%description devel
Qt is a cross-platform application and UI framework. Using Qt, you can
write web-enabled applications once and deploy them across desktop,
mobile and embedded systems without rewriting the source code.
.
This package contains the QtMultimedia module development files

%package -n qt5-qtdeclarative-import-multimedia
Summary:    QtQml multimedia import
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}
Requires:   qt5-qtdeclarative

%description -n qt5-qtdeclarative-import-multimedia
This package contains the Multimedia import for QtQml

%package gsttools
Summary:    Qt Multimedia - Utility library for GStreamer media services
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}

%description gsttools
This package contains a shared library for the GStreamer QtMultimedia media services

%package plugin-mediaservice-gstaudiodecoder
Summary:    Qt Multimedia - GStreamer audio decoder media service
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}
Requires:   qt5-qtmultimedia-gsttools = %{version}-%{release}

%description plugin-mediaservice-gstaudiodecoder
This package contains the GStreamer audio decoder plugin for QtMultimedia

%package plugin-resourcepolicy-resourceqt
Summary:    Qt Multimedia - libresourceqt resource policy plugin
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}

%description plugin-resourcepolicy-resourceqt
This package contains the libresourceqt resource policy plugin for QtMultimedia

%package plugin-mediaservice-gstcamerabin
Summary:    Qt Multimedia - GStreamer camerabin video capture media service
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}
Requires:   qt5-qtmultimedia-gsttools = %{version}-%{release}

%description plugin-mediaservice-gstcamerabin
This package contains the GStreamer camerabin video capture plugin for QtMultimedia

%package plugin-mediaservice-gstmediacapture
Summary:    Qt Multimedia - GStreamer video4linux2 video capture media service
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}
Requires:   qt5-qtmultimedia-gsttools = %{version}-%{release}

%description plugin-mediaservice-gstmediacapture
This package contains the GStreamer video4linux2 video capture plugin for QtMultimedia

%package plugin-mediaservice-gstmediaplayer
Summary:    Qt Multimedia - GStreamer playback media service
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}
Requires:   qt5-qtmultimedia-gsttools = %{version}-%{release}

%description plugin-mediaservice-gstmediaplayer
This package contains the GStreamer media playback plugin for QtMultimedia

%package plugin-playlistformats-m3u
Summary:    Qt Multimedia - M3U playlist support
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}
Requires:   qt5-qtmultimedia-gsttools = %{version}-%{release}

%description plugin-playlistformats-m3u
This package contains the M3U playlist support

%package plugin-audio-pulseaudio
Summary:    Qt Multimedia - Pulse Audio plugin
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}

%description plugin-audio-pulseaudio
This package contains the pulse audio sound effect support.

%package plugin-audio-alsa
Summary:    Qt Multimedia - Alsa audio plugin
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}

%description plugin-audio-alsa
This package contains the alsa sound effect support.

%package plugin-video-eglvideonode
Summary:    Qt Multimedia - eglvideonode plugin
Group:      Qt/Qt
Requires:   %{name} = %{version}-%{release}

%description plugin-video-eglvideonode
This package contains the egl video node plugin.

%prep
%setup -q -n %{name}-%{version}

%build
export QTDIR=/usr/share/qt5
touch .git

%qmake5 GST_VERSION=1.0 QT.widgets.name= DEFINES+=QT_NO_WIDGETS -r "DEFINES+=NEMO_AUDIO" CONFIG+=config_pulseaudio
make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install
# Remove unneeded .la files
rm -f %{buildroot}/%{_libdir}/*.la
# Fix wrong path in pkgconfig files
find %{buildroot}%{_libdir}/pkgconfig -type f -name '*.pc' \
-exec perl -pi -e "s, -L%{_builddir}/?\S+,,g" {} \;
# Fix wrong path in prl files
find %{buildroot}%{_libdir} -type f -name '*.prl' \
-exec sed -i -e "/^QMAKE_PRL_BUILD_DIR/d;s/\(QMAKE_PRL_LIBS =\).*/\1/" {} \;
#
# We don't need qt5/Qt/
rm -rf %{buildroot}/%{_includedir}/qt5/Qt



%fdupes %{buildroot}/%{_includedir}





%post
/sbin/ldconfig
%postun
/sbin/ldconfig

%post gsttools
/sbin/ldconfig
%postun gsttools
/sbin/ldconfig





%files
%defattr(-,root,root,-)
%{_libdir}/libQt5Multimedia.so.5
%{_libdir}/libQt5Multimedia.so.5.*
%{_libdir}/libQt5MultimediaQuick_p.so.5
%{_libdir}/libQt5MultimediaQuick_p.so.5.*

%files devel
%defattr(-,root,root,-)
%{_libdir}/libQt5Multimedia.so
%{_libdir}/libQt5MultimediaQuick_p.so
%{_libdir}/libqgsttools_p.so
%{_libdir}/libQt5Multimedia.prl
%{_libdir}/libQt5MultimediaQuick_p.prl
%{_libdir}/libqgsttools_p.prl
%{_libdir}/pkgconfig/*
%{_includedir}/qt5/*
%{_datadir}/qt5/mkspecs/
%{_libdir}/cmake/


%files -n qt5-qtdeclarative-import-multimedia
%defattr(-,root,root,-)
%{_libdir}/qt5/qml/QtMultimedia/

%files gsttools
%defattr(-,root,root,-)
%{_libdir}/libqgsttools_p.so.1
%{_libdir}/libqgsttools_p.so.1.*

%files plugin-mediaservice-gstaudiodecoder
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/mediaservice/libgstaudiodecoder.so

%files plugin-mediaservice-gstcamerabin
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/mediaservice/libgstcamerabin.so

%files plugin-mediaservice-gstmediacapture
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/mediaservice/libgstmediacapture.so

%files plugin-mediaservice-gstmediaplayer
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/mediaservice/libgstmediaplayer.so

%files plugin-playlistformats-m3u
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/playlistformats/libqtmultimedia_m3u.so

%files plugin-resourcepolicy-resourceqt
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/resourcepolicy/libresourceqt.so

%files plugin-audio-pulseaudio
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/audio/libqtmedia_pulse.so

%files plugin-audio-alsa
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/audio/libqtaudio_alsa.so

%files plugin-video-eglvideonode
%defattr(-,root,root,-)
%{_libdir}/qt5/plugins/video/videonode/libeglvideonode.so

