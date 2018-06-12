QT =
TARGET = dbus-zwave
TEMPLATE = app

# CONFIG += qt-app

CONFIG(release, debug|release) {
    DEFINES += RELEASE
}
CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}

DEFINES += CFG_DBUS
CONFIG += dbus link_pkgconfig
PKGCONFIG = dbus-1 libopenzwave
LIBS += -levent

QMAKE_CXXFLAGS += -std=c++11

VELIB = ext/velib

include($${VELIB}/projects/common.pri)

INCLUDEPATH += inc $${VELIB}/inc

SOURCES += \
    src/dz_item.cpp \
    src/dz_driver.cpp \
    src/dz_node.cpp \
    src/dz_nodename.cpp \
    src/dz_value.cpp \
    src/dz_commandclass.cpp \
    src/dz_constvalue.cpp \
    src/dz_setting.cpp \
    src/values/dz_gridmeter.cpp \
    src/values/dz_temperature.cpp \
    $${VELIB}/src/base/ve_string.c \
    $${VELIB}/src/plt/posix_ctx.c \
    $${VELIB}/src/types/ve_dbus_item.c \
    $${VELIB}/src/types/ve_dbus_item_consumer.c \
    $${VELIB}/src/types/ve_stamp.c \
    $${VELIB}/src/types/ve_str.c \
    $${VELIB}/src/types/ve_values.c \
    $${VELIB}/src/types/ve_variant.c \
    $${VELIB}/src/types/ve_variant_print.c \
    $${VELIB}/src/utils/ve_getopt.c \
    $${VELIB}/src/utils/ve_getopt_long.c \
    $${VELIB}/src/utils/ve_item_utils.c \
    $${VELIB}/src/utils/ve_logger.c \
    $${VELIB}/task/task/console_no_arguments.c \
    $${VELIB}/task/task/platform_init.c \
    src/task.cpp \

!CONFIG(qt-app) {
    # build as c application
    SOURCES += \
        $${VELIB}/src/types/ve_item.c \
        $${VELIB}/task/task/main_libevent.c \

} else {
    # build as QT application
    QT += core gui
    CONFIG += ve-qitems-from-c
    greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

    SOURCES += \
        src/qt_main.cpp \
        $${VELIB}/src/plt/task_qt.cpp \

    HEADERS += \
        $${VELIB}/inc/velib/platform/task_qt.hpp \

    include($${VELIB}/src/qt/ve_qitems.pri)
}

HEADERS  +=  \
    inc/dz_item.hpp \
    inc/dz_driver.hpp \
    inc/dz_node.hpp \
    inc/dz_nodename.hpp \
    inc/dz_value.hpp \
    src/dz_commandclass.hpp \
    inc/dz_constvalue.hpp \
    inc/dz_setting.hpp \
    inc/values/dz_gridmeter.hpp \
    inc/values/dz_temperature.hpp \
    $${VELIB}/inc/velib/base/types.h \
    $${VELIB}/inc/velib/types/variant.h \
    $${VELIB}/inc/velib/types/variant_print.h \
    $${VELIB}/inc/velib/types/ve_dbus_item.h \
    $${VELIB}/inc/velib/types/ve_item.h \
    $${VELIB}/inc/velib/types/ve_item_def.h \
    $${VELIB}/inc/velib/types/ve_str.h \
    $${VELIB}/inc/velib/utils/ve_assert.h \
    $${VELIB}/inc/velib/utils/ve_getopt.h \
    $${VELIB}/inc/velib/utils/ve_item_utils.h \
    $${VELIB}/inc/velib/utils/ve_logger.h \
    $${VELIB}/inc/velib/utils/ve_rand.h \
    $${VELIB}/inc/velib/utils/ve_todo.h \
    $${VELIB}/inc/velib/velib_config.h \
    inc/velib/velib_config_app.h \

target.path = /home/root
INSTALLS += target
