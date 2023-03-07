
TEMPLATE = app
QT += widgets
QT += concurrent

# Input
HEADERS += \
    src/h2rename.h
FORMS += \
    src/h2rename.ui \
    src/ReadDirProgress.ui

SOURCES += \
    src/h2rename.cpp \
    src/main.cpp

OTHER_FILES += \
    h2rename.nsi \
    license.txt

RC_FILE = \
    h2rename.rc

TRANSLATIONS = \
    lang/h2rename.ts \
	lang/h2rename_de.ts \
	lang/h2rename_nl.ts
# TODO: update ts to qm

RESOURCES += \
    h2rename.qrc
