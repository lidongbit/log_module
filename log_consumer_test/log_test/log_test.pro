TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        ../../log_consumer/double_fifo.c \
        ../../log_consumer/mem_consumer.c

HEADERS += \
    ../../log_consumer/double_fifo.h
