CONFIG += release

isEmpty(PSISDK) {
    include(../../psiplugin.pri)
} else {
    include($$PSISDK/plugins/psiplugin.pri)
}

RESOURCES = resources/xep0313.qrc

HEADERS += src/xep0313.h src/message_type.h src/history_widget.h
SOURCES += src/xep0313.cpp src/history_widget.cpp
