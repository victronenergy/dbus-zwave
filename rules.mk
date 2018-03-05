T = dbus-zwave$(EXT)
TARGETS += $T
INSTALL_BIN += $T

DEFINES += DBUS
override CFLAGS += $(shell pkg-config --cflags dbus-1 libopenzwave)
$T_LIBS += -lpthread -ldl -lm `pkg-config --libs dbus-1 libopenzwave` -levent -levent_pthreads -L$(OBJDIR)

INCLUDES += $(VELIB_PATH)/inc
SUBDIRS += $(VELIB_PATH)
$T_DEPS += $(call subtree_tgts,$(d)/$(VELIB_PATH))

INCLUDES += src
SUBDIRS += src
$T_DEPS += $(call subtree_tgts,$(d)/src)

LINKER=g++
