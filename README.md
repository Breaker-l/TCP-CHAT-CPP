# TCP-CHAT-CPP# 💬 Консольный TCP-чат на C++ (WinSock2) / Console TCP Chat in C++

[English Version](#english-version) | [Русская версия](#русская-версия)

---

## English Version

A full-stack console client-server application for real-time messaging with chat room (channel) support and message history retention. Developed in C++ using the **WinSock2** library for Windows.

### ✨ Features
- **Architecture:** Client-Server (TCP/IP).
- **Multiple Connections:** The server utilizes the `select()` function for non-blocking processing of the client pool within a single thread.
- **Multithreaded Client:** A dedicated `std::thread` is used for continuous message reading without blocking user console input.
- **Room System:** Users can dynamically enter and leave isolated chat rooms.
- **Message History:** Chat logs are stored on the server in text files and automatically sent to new participants upon entering a room.
- **Memory Safety:** Built using modern C++ principles with `std::unique_ptr` and thread-safe flags (`std::atomic`).

### 📁 Code Structure
- `ChatMember.h` — Shared component (socket storage, inbound packet buffering, data transfer).
- `ChatRoom.h` — Server-side room logic (member list, message logging, `Broadcast` delivery).
- `ChatServer.h` / `ServerMain.cpp` — Server core (WinSock initialization, `MasterSet` pool, command parsing).
- `ClientMain.cpp` — Client code (connection configuration, command dispatch, and background message loop).

### 🎮 Protocol Commands
Once connected, the client interacts with the server using text commands:
1. `1 <room_id> <nickname>` — Join a room (e.g., `1 12345 Vladislav`).
2. `2 <message>` — Send a text message to the current room.
3. `3` — Leave the current room.
4. `4` — Disconnect from the server.

### 🚀 Compilation & Execution
**Server Build:** `g++ ServerMain.cpp -o ChatServer -lws2_32`  
**Client Build:** `g++ ClientMain.cpp -o ChatClient -lws2_32`  

---

## Русская версия

Полноценное клиент-серверное приложение для общения в реальном времени с поддержкой комнат (каналов) и сохранением истории сообщений. Разработано на C++ с использованием библиотеки **WinSock2** для ОС Windows.

### ✨ Особенности проекта
- **Архитектура:** Клиент-Сервер (TCP/IP).
- **Множественные подключения:** Сервер использует функцию `select()` для неблокирующей обработки пула клиентов в одном потоке.
- **Многопоточный клиент:** Выделенный поток `std::thread` для непрерывного чтения сообщений без блокировки консольного ввода пользователя.
- **Система комнат:** Возможность динамического входа и выхода из изолированных чат-комнат.
- **История сообщений:** Логирование переписки на сервере в файлы и автоматическая отправка истории новому участнику при входе.
- **Безопасность памяти:** Активное использование умных указателей `std::unique_ptr` и потокобезопасных флагов `std::atomic`.

### 📁 Структура кода
- `ChatMember.h` — общий компонент (хранение сокета, буферизация входящих пакетов, отправка/прием данных).
- `ChatRoom.h` — логика комнаты на сервере (список участников, широковещательная рассылка `Broadcast`, работа с файлами истории).
- `ChatServer.h` / `ServerMain.cpp` — ядро сервера (инициализация WinSock, пул `MasterSet`, парсинг команд).
- `ClientMain.cpp` — код клиента (меню подключения, отправка команд и фоновый поток приема сообщений).

### 🎮 Протокол взаимодействия (Команды)
После подключения к серверу клиент отправляет текстовые команды:
1. `1 <room_id> <nickname>` — Войти в комнату под указанным никнеймом (например: `1 12345 Vladislav`).
2. `2 <message>` — Отправить текстовое сообщение в текущую комнату.
3. `3` — Покинуть комнату.
4. `4` — Полностью отключиться от сервера.

### 🚀 Как скомпилировать и запустить
**Сборка Сервера:** `g++ ServerMain.cpp -o ChatServer -lws2_32`  
**Сборка Клиента:** `g++ ClientMain.cpp -o ChatClient -lws2_32`  

---

## 🧑‍💻 Author / Автор
- **Vladislav Lomeyko** (Kursk, Russia).
