QT =
TARGET = dbus-zwave
TEMPLATE = app

# CONFIG += qt-app

DEFINES += CFG_DBUS
CONFIG += dbus link_pkgconfig
PKGCONFIG = dbus-1 libopenzwave
LIBS += -levent

VELIB = ext/velib

include($${VELIB}/projects/common.pri)

INCLUDEPATH += src $${VELIB}/inc

SOURCES += \
    src/dz_item.cpp \
    src/dz_driver.cpp \
    src/dz_node.cpp \
    src/dz_value.cpp \
    src/dz_namedvalue.cpp \
    src/task.cpp \
    $${VELIB}/src/base/ve_string.c \
    $${VELIB}/src/plt/posix_ctx.c \
    $${VELIB}/src/types/ve_dbus_item.c \
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
    src/dz_item.h \
    src/dz_driver.h \
    src/dz_node.h \
    src/dz_value.h \
    src/dz_namedvalue.h \
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
    src/velib/velib_config_app.h \

target.path = /home/root
INSTALLS += target
