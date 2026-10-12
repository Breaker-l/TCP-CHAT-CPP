
#define _WINSOCKAPI_


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


#define _WINSOCK_DEPRECATED_NO_WARNINGS


#include <windows.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>


#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include "ChatMember.h"

std::atomic<bool> isRunning(true);

void ReceiverThread(ChatMember& client) {
    std::vector<std::string> messages;
    while (isRunning && client.isActive()) {
        if (client.ReceiveMessages(messages)) {
            for (const auto& msg : messages) {
                std::cout << "\r" << msg << "\n> " << std::flush;
            }
            messages.clear();
        }
    }
}

int main() {
    system("color 70");

    std::cout << "=== TCP Chat Client ===\n";
    std::cout << "Made by Vladislav Lomeyko, 16 y.o.\n";
    std::cout << "Kursk, Russia\n\n";

    ChatMember client;

    std::string serverIP;
    std::string portStr;
    int port;

    std::cout << "Enter server IP (default: 127.0.0.1): ";
    std::getline(std::cin, serverIP);
    if (serverIP.empty()) serverIP = "127.0.0.1";

    std::cout << "Enter port (default: 54000): ";
    std::getline(std::cin, portStr);
    if (portStr.empty()) portStr = "54000";
    port = std::stoi(portStr);

    std::cout << "\nConnecting to " << serverIP << ":" << port << "...\n";

    if (!client.Connect(serverIP, port)) {
        std::cerr << "Failed to connect to server!\n";
        system("pause");
        return 1;
    }

    std::cout << "Connected successfully!\n\n";

    std::thread receiver(ReceiverThread, std::ref(client));
    receiver.detach();

    std::cout << "=== Commands ===\n";
    std::cout << "1 <room_id> <nickname> - Join room\n";
    std::cout << "2 <message>            - Send message\n";
    std::cout << "3                      - Leave room\n";
    std::cout << "4                      - Quit\n\n";

    std::string input;
    while (isRunning && client.isActive()) {
        std::cout << "> " << std::flush;
        std::getline(std::cin, input);

        if (input.empty()) continue;

        std::string command = input + "\n";
        if (!client.Send(command)) {
            std::cerr << "Failed to send message!\n";
            break;
        }

        if (input[0] == '4') {
            std::cout << "Goodbye!\n";
            isRunning = false;
            break;
        }
    }

    client.Disconnect();
    WSACleanup();
    system("pause");
    return 0;
}