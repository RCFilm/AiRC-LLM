AiRC-LLM: Advanced Interactive Research Console for Large Language Models
Overview

AiRC-LLM (Advanced Interactive Research Console for Large Language Models) is a comprehensive application designed to interact with various Large Language Model (LLM) APIs, including Hugging Face, DeepSeek, and Ollama. The application provides a user-friendly GUI for managing multiple workspaces, selecting models, and generating responses based on user input.

Features

Workspace Management: Create, rename, and delete workspaces.
Model Selection: Choose from a variety of models available through different APIs.
Chat Interface: Interact with models through a chat interface.
Settings: Configure API settings and model parameters.
Contextual Embeddings: Manage and query embeddings for text data.
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
4. Using CMake (Optional)

If you prefer using CMake, you can generate the build files and compile the project as follows:

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

Add Workspace: Click the "+" button to add a new workspace.
Rename Workspace: Right-click on a workspace and select "Rename Workspace".
Delete Workspace: Right-click on a workspace and select "Delete Workspace".
Model Selection

Select API: Choose between "Ollama", "HuggingFace", and "DeepSeek" APIs.
Select Model: Choose a model from the list of available models.
Chat Interface

Send Message: Type your message in the input box and press "Send" or hit "Enter".
Clear Chat: Click "Clear" to clear the chat history.
Settings

Open Settings: Click the "Settings" button to configure API settings and model parameters.
Contextual Embeddings

Add Embedding: Add embeddings for text data.
Query Embedding: Query the nearest text based on an embedding.
Contributing

We welcome contributions! Please see our CONTRIBUTING.md for details on how to contribute to this project.

License

This project is licensed under the RCFilm Proprietary License. By accessing, using, or modifying this software, you agree to be bound by the terms and conditions of this license.

Key Points of the License:

Non-Commercial Use Only: You are granted a non-exclusive, non-transferable license to use this software for non-commercial purposes only. Commercial use is strictly prohibited without explicit written permission from the original author (RCFilm).
Modification and Ownership: Any modifications made to the software must be shared with the original author (RCFilm), and the original author retains ownership of those modifications. You may not distribute or use any modified versions of the software without explicit written permission from the original author.
Attribution: You must include attribution to the original author (RCFilm) in any use or distribution of this software.
No Warranty: This software is provided "as is," without warranty of any kind, express or implied.
Termination: Your rights under this license will terminate automatically without notice from the original author if you fail to comply with any term(s) of this license.
Governing Law: This license shall be governed by and construed in accordance with the laws of USA, without regard to its conflict of law principles.
Contact Information

For any questions or to request permission for commercial use, please contact RCFilm at https://github.com/RCFilm.
Thank you for using AiRC-LLM! We hope it serves your research and development needs effectively.
