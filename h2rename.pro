
TEMPLATE = app
QT += widgets
QT += concurrent

# Input
HEADERS += \
    src/aboutdialog.h \
    src/mainwindow.h \
    src/directory.h \
    src/h2rename.h \
    src/name.h \
    src/namesfilterproxymodel.h \
    src/namesmodel.h \
    src/readdirprogressdialog.h \
    src/readdirthread.h \
    src/renamerule.h \
    src/renamerulesmodel.h \
    src/version.h

FORMS += \
    src/aboutdialog.ui \
    src/mainwindow.ui \
    src/h2rename.ui \
    src/ReadDirProgress.ui

SOURCES += \
    src/aboutdialog.cpp \
    src/mainwindow.cpp \
    src/h2rename.cpp \
    src/main.cpp \
    src/namesfilterproxymodel.cpp \
    src/namesmodel.cpp \
    src/readdirprogressdialog.cpp \
    src/readdirthread.cpp \
    src/renamerulesmodel.cpp

OTHER_FILES += \
    h2rename.nsi \
    license.txt

# Tell qmake to generate the .qm files in the build directory
TRANSLATIONS_DIR = $$OUT_PWD/lang

TRANSLATIONS = \
    lang/h2rename.ts \
    lang/h2rename_de.ts \
    lang/h2rename_nl.ts \
    lang/h2rename_zh_TW.ts

# Tell qmake to run lupdate
lupdate.target = translations
lupdate.commands = $$[QT_INSTALL_BINS]/lupdate $$PWD -ts $$TRANSLATIONS
QMAKE_EXTRA_TARGETS += lupdate

#QMAKE_PRE_LINK += $$[QT_INSTALL_BINS]/lupdate $$TRANSLATIONS

# Tell qmake to run lrelease after the project is built
QMAKE_POST_LINK += $$[QT_INSTALL_BINS]/lrelease $$TRANSLATIONS

RESOURCES += \
    h2rename.qrc

IMAGES.files +=\
    images/h2rename.png

unix {
    DESKTOP.path = /usr/share/applications/
    DESKTOP.files = h2rename.desktop
    IMAGES.path = /usr/share/pixmaps/
    target.path = /opt/$${TARGET}

    config.path = $$target.path/$${TARGET}
    config.extra = ln -sf $$config.path $(DESTDIR)/usr/bin/$${TARGET}

    INSTALLS += \
        target \
        IMAGES \
        DESKTOP
}

win32 {
    # windows resources
    CONFIG += embed_manifest_exe

    RC_ICONS=$$PWD/images/h2rename.ico #：指定應該被包含進一個.rc檔案中的圖示，僅適用於Windows
    QMAKE_TARGET_COMPANY="coolshou.idv.tw" #：指定項目目標的公司名稱，僅適用於Windows
    QMAKE_TARGET_PRODUCT=$${TARGET} #：指定項目目標的產品名稱，僅適用於Windows
    QMAKE_TARGET_DESCRIPTION="qt base file rename tool" #：指定項目目標的描述資訊，僅適用於Windows
    QMAKE_TARGET_COPYRIGHT="Copyright 2023 coolshou.idv.tw" #：指定項目目標的版權資訊，僅適用於Windows
    #PACKAGE_DOMAIN：
    #PACKAGE_VERSION：
    RC_CODEPAGE=0x04b0 #unicode：指定應該被包含進一個.rc檔案中的字碼頁，僅適用於Windows
    RC_LANG=0x0409 #en_US：指定應該被包含進一個.rc檔案中的語言，僅適用於Windows

    DISTFILES += $$PWD/images/h2rename.ico

    DIST_DIRECTORY =  $$shell_quote($$shell_path($${ROOT_DIRECTORY}/../$${TARGET}_$${QT_ARCH}-$${VERSION}))

    DIST_FILE = $$shell_quote($$shell_path($$DIST_DIRECTORY/$${TARGET}.exe))
    deploy.commands = \
        windeployqt $$DIST_FILE

    QMAKE_EXTRA_TARGETS +=  deploy
}
