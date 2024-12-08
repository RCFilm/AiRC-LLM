# AiRC-LLC.pro
QT += core gui network widgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/deepseek_api.cpp \
    src/huggingface_agent.cpp \
    src/huggingface_api.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/mainwindow_helpers.cpp \
    src/ollama_agent.cpp \
    src/ollama_api.cpp \
    src/workspace.cpp \
    src/workspace_manager.cpp

HEADERS += \
    headers/deepseek_api.h \
    headers/huggingface_agent.h \
    headers/huggingface_api.h \
    headers/llm_agent_interface.h \
    headers/llm_api_interface.h \
    headers/mainwindow.h \
    headers/mainwindow_helpers.h \
    \  # include/Ollama.h # Update the path to Ollama.h
    headers/ollama_agent.h \
    headers/ollama_api.h \
    headers/workspace.h \
    headers/workspace_manager.h \
    include/Ollama.hpp \
    include/hnswlib/bruteforce.h \
    include/hnswlib/hnswalg.h \
    include/hnswlib/hnswlib.h \
    include/hnswlib/space_ip.h \
    include/hnswlib/space_l2.h \
    include/hnswlib/stop_condition.h \
    include/hnswlib/visited_list_pool.h

FORMS += \
    ui/mainwindow.ui

# Add the include path for headers
INCLUDEPATH += $$PWD/headers
INCLUDEPATH += $$PWD/include

# Set the deployment target to macOS 12.0
QMAKE_MACOSX_DEPLOYMENT_TARGET = 12.0

# Add the include path for cmark
INCLUDEPATH += /usr/local/include

# Link against the cmark library
LIBS += -L/usr/local/lib -lcmark

# Set the application name
TARGET = AiRC-LLC

DISTFILES += \
    LICENSE \
    README.txt
