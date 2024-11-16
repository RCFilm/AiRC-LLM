QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    MarkdownHighlighter.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    MarkdownHighlighter.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Add Ollama API endpoint
OLLAMA_API_ENDPOINT = http://localhost:11434/v1
