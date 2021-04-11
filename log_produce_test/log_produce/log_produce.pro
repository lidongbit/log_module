TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    ../../log_produce/Force_Log.c \
    ../../log_produce/buffer_manager.c \
    ../../log_produce/mem_produce.c

HEADERS += \
    ../../log_produce/Force_Log.h \
    ../../log_produce/buffer_manager.h
