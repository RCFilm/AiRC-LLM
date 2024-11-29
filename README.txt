AiRC-LLM (Advanced Interactive Research Console for Large Language Models) is a comprehensive application designed to interact with various Large Language Model (LLM) APIs, including Hugging Face, DeepSeek, and Ollama. The application provides a user-friendly GUI for managing multiple workspaces, selecting models, and generating responses based on user input.

Table of Contents

Features
Prerequisites
Installation
Usage
Workspace Management
Model Selection
Chat Interface
Settings
Contextual Embeddings
Contributing
License
Contact
Features

Workspace Management: Create, rename, and delete workspaces.
Model Selection: Choose from multiple APIs (Ollama, HuggingFace, DeepSeek) and select models.
Interactive Chat: Interact with models through a chat interface with Markdown support.
Message History: Maintain a history of messages for each workspace.
API Configuration: Configure API settings and model parameters.
Embedding Management: Manage and query embeddings for text data.
Prerequisites

Before you begin, ensure you have met the following requirements:

Qt Framework: Version 5.15 or later.
C++ Compiler: Compatible with C++11 or later.
CMake: Version 3.10 or later (optional, for building with CMake).
Dependencies: Ensure you have the necessary dependencies installed, including cmark for Markdown processing.
Installation

1. Clone the Repository

bash
Copy
git clone https://github.com/RCFilm/AiRC-LLM.git
cd AiRC-LLM
2. Install Dependencies

Ensure you have the required dependencies installed. For example, on a Debian-based system:

bash
Copy
sudo apt-get install build-essential qt5-default libcmark-dev
3. Build the Project

Using Qt Creator (Recommended)

Open the project file AiRC-LLM.pro in Qt Creator.
Configure the project settings if necessary.
Build and run the project using the Qt Creator IDE.
Using Command Line

Generate the Makefile using qmake:

bash
Copy
qmake -makefile
Build the project:

bash
Copy
make
Run the application:

bash
Copy
./AiRC-LLM
Using CMake (Optional)

Create a build directory:

bash
Copy
mkdir build
cd build
Generate the build files:

bash
Copy
cmake ..
Build the project:

bash
Copy
make
Run the application:

bash
Copy
./AiRC-LLM
Usage

Workspace Management

Adding a Workspace

Function: MainWindow::addWorkspace()
Steps:
Click the "+" button.
Configure the workspace settings (API type, model, etc.).
Click "OK" to create the workspace.
Renaming a Workspace

Function: MainWindow::renameWorkspace()
Steps:
Right-click on the workspace.
Select "Rename Workspace".
Enter the new name and click "OK".
Deleting a Workspace

Function: MainWindow::deleteWorkspace()
Steps:
Right-click on the workspace.
Select "Delete Workspace".
Confirm the deletion in the dialog.
Model Selection

Selecting an API

Function: MainWindow::openSettings()
Steps:
Click the "Settings" button.
Select the desired API from the dropdown.
Click "OK" to save the settings.
Selecting a Model

Function: MainWindow::openSettings()
Steps:
Open the settings dialog.
Select the desired model from the dropdown.
Click "OK" to save the settings.
Chat Interface

Sending a Message

Function: MainWindow::sendMessage()
Steps:
Type your message in the input box.
Press "Send" or hit "Enter".
Clearing the Chat

Function: MainWindow::clearChat()
Steps:
Click the "Clear" button.
Settings

Configuring API Settings

Function: MainWindow::openSettings()
Steps:
Click the "Settings" button.
Configure the API settings and model parameters.
Click "OK" to save the settings.
Contextual Embeddings

Adding an Embedding

Function: Workspace::addEmbedding()
Steps:
Open the settings dialog.
Enable embedding usage.
Add embeddings for text data.
Querying an Embedding

Function: Workspace::getNearestText()
Steps:
Open the settings dialog.
Enable embedding usage.
Query the nearest text based on an embedding.
Contributing

We welcome contributions! Please see our CONTRIBUTING.md for details on how to contribute to this project.

License

This project is licensed under the RCFilm Proprietary License. By accessing, using, or modifying this software, you agree to be bound by the terms and conditions of this license. For more details, see the LICENSE.txt file.

Contact

For any questions or to request permission for commercial use, please contact RCFilm at https://github.com/RCFilm.

Thank you for using AiRC-LLM! We hope it serves your research and development needs effectively.
