#define _WINSOCKAPI_
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include "ChatServer.h"
#include "ChatRoom.h"
#include "ChatMember.h"


bool ChatRoom::AddMember(ChatMember* member) {
    if (member == nullptr) return false;
    SendHistoryTo(member);
    std::string message = "Member - " + member->GetNickname() + " has joined the chat";
    Members.push_back(member);
    member->SetCurrentRoom(this);
    Broadcast(message);
    return true;
}

void ChatRoom::RemoveMember(ChatMember* member) {
    if (member == nullptr) return;
    Broadcast(member->GetNickname() + " has left the chat");
    Members.remove(member);
    member->SetCurrentRoom(nullptr);
}

size_t ChatRoom::GetMemberCount() const { return Members.size(); }
int ChatRoom::GetId() const { return chatId; }
bool ChatRoom::IsEmpty() const { return Members.empty(); }

void ChatRoom::Broadcast(const std::string& message, ChatMember* exclude) {
    Messages << message << "/";
    Messages.flush();
    for (auto member : Members) {
        if (member != exclude) member->Send(message);
    }
}

void ChatRoom::SendHistoryTo(ChatMember* member) {
    if (member == nullptr) return;
    Messages.seekg(0);
    std::string currentMessage;
    char ch;
    while (Messages.get(ch)) {
        if (ch == '/') {
            if (!currentMessage.empty()) {
                member->Send(currentMessage);
                currentMessage.clear();
            }
        }
        else {
            currentMessage += ch;
        }
    }
    Messages.clear();
    Messages.seekg(0,std::ios::end);
}



ChatServer::~ChatServer() {
    StopServer();
    if (listenSocket != INVALID_SOCKET) closesocket(listenSocket);
    Members.clear();
    Rooms.clear();
    WSACleanup();
}

bool ChatServer::StartServer(int& port) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

    listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) { WSACleanup(); return false; }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket); WSACleanup(); return false;
    }

    FD_ZERO(&MasterSet);
    FD_SET(listenSocket, &MasterSet);

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        closesocket(listenSocket); WSACleanup(); return false;
    }

    return true;
}

void ChatServer::AcceptClient() {
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    SOCKET clientSocket = accept(listenSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) return;

    auto newMember = std::make_unique<ChatMember>(clientSocket);
    SOCKET sock = newMember->GetSocket();
    FD_SET(sock, &MasterSet);

    std::string welcome =
        "=== Welcome to TCP Chat made by Vladislav ===\r\n"
        "You must join a room first!\r\n"
        "1 <room_id> <nickname> - Join room\r\n"
        "2 <message> - Send message\r\n"
        "3 - Leave the room\r\n"
        "4 - Leave the server\r\n";
    newMember->Send(welcome);

    Members.push_back(std::move(newMember));
}

void ChatServer::Run() {
    isRunning = true;
    while (isRunning) {
        fd_set readSet = MasterSet;
        timeval timeout = { 0, 100000 };

        int activity = select(0, &readSet, NULL, NULL, &timeout);
        if (activity == SOCKET_ERROR || activity == 0) continue;

        if (FD_ISSET(listenSocket, &readSet)) {
            AcceptClient();
            activity--;
        }

        for (size_t i = 0; i < Members.size() && activity > 0; ++i) {
            ChatMember* member = Members[i].get();
            if (FD_ISSET(member->GetSocket(), &readSet)) {
                ProcessClient(member);
                activity--;
            }
        }

        std::vector<size_t> toRemove;
        for (size_t i = 0; i < Members.size(); ++i) {
            if (!Members[i]->isActive()) toRemove.push_back(i);
        }
        for (int i = toRemove.size() - 1; i >= 0; --i) {
            RemoveClient(Members[toRemove[i]].get());
        }
    }
}

void ChatServer::ProcessClient(ChatMember* member) {
    std::vector<std::string> commands;
    if (!member->ReceiveMessages(commands)) {
        RemoveClient(member);
        return;
    }
    for (const auto& cmd : commands) {
        if (cmd.empty()) continue;
        char action = cmd[0];

        if (action == '1') {
            size_t s1 = cmd.find(' ');
            size_t s2 = cmd.find(' ', s1 + 1);
            if (s1 == std::string::npos || s2 == std::string::npos) {
                member->Send("Usage: 1 <room_id> <nickname>\r\n");
                continue;
            }
            std::string roomStr = cmd.substr(s1 + 1, s2 - s1 - 1);
            std::string nick = cmd.substr(s2 + 1);
            JoinRoom(member, std::stoi(roomStr), nick);
        }
        else if (action == '2') {
            size_t s = cmd.find(' ');
            if (s == std::string::npos) {
                member->Send("Usage: 2 <message>\r\n");
                continue;
            }
            if (!member->GetCurrentRoom()) {
                member->Send("Join a room first!\r\n");
                continue;
            }
            std::string msg = member->GetNickname() + ": " + cmd.substr(s + 1);
            member->GetCurrentRoom()->Broadcast(msg, member);
        }
        else if (action == '3') {
            if (member->GetCurrentRoom()) {
                member->Send("You have left the room.\r\n");
                LeaveRoom(member);
            }
            else {
                member->Send("You are not in a room.\r\n");
            }
        }
        else if (action == '4') {
            member->Send("Goodbye!\r\n");
            RemoveClient(member);
            return;
        }
        else {
            member->Send("Unknown command. Use 1-4.\r\n");
        }
    }
}

ChatRoom* ChatServer::FindRoom(int roomId) {
    auto it = Rooms.find(roomId);
    return (it != Rooms.end()) ? it->second.get() : nullptr;
}

void ChatServer::JoinRoom(ChatMember* member, int roomId, const std::string& nickname) {
    if (!member) return;
    ChatRoom* room = FindRoom(roomId);
    if (!room) {
        member->Send("Room is not found.\r\n");
        return;
    }
    if (member->GetCurrentRoom()) {
        member->Send("Leave your current room first!\r\n");
        return;
    }
    member->SetNickname(nickname);
    room->AddMember(member);
}

void ChatServer::LeaveRoom(ChatMember* member) {
    if (!member) return;
    ChatRoom* room = member->GetCurrentRoom();
    if (room) room->RemoveMember(member);
}

void ChatServer::RemoveClient(ChatMember* member) {
    if (!member) return;
    if (member->GetCurrentRoom()) LeaveRoom(member);
    FD_CLR(member->GetSocket(), &MasterSet);
    member->Disconnect();

    auto it = std::find_if(Members.begin(), Members.end(),
        [member](const std::unique_ptr<ChatMember>& p) { return p.get() == member; });
    if (it != Members.end()) Members.erase(it);
}

void ChatServer::StopServer() {
    isRunning = false;
}

bool ChatServer::CreateRoom(const int& roomId, const std::string& roomName) {
    if (Rooms.find(roomId) != Rooms.end()) {
        return false; 
    }
    Rooms[roomId] = std::make_unique<ChatRoom>(roomId, roomName);
    return true;
}

int main() {
    ChatServer server;
    int port = 54000;
    std::cout << "Starting server on port " << port << "...\n";
    if (!server.StartServer(port)) {
        std::cerr << "Failed to start server!\n";
        return 1;
    }

    server.CreateRoom(83501, "Main");
    server.CreateRoom(12345, "Fun");

    std::cout << "Created rooms: 83501 (Main), 12345 (Fun)\n";
    // =================================

    std::cout << "Server started! Waiting for clients...\n";
    server.Run();
    return 0;
}