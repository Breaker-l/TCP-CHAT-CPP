#pragma once

#include <WinSock2.h>
#include <string>
#include <vector>
class ChatRoom;


class ChatMember {
	SOCKET socket;
	std::string recvbuffer;
	bool isConnected;
	std::string nickname;
	ChatRoom* currentRoom;
	int currentRoomId;
public:
	
	
	ChatMember(SOCKET s);
	
	ChatMember();

	bool Send(const std::string& Message);
	const std::string& GetNickname() const;
	bool ReceiveMessages(std::vector< std::string>& OutMessages);
	bool isActive() const;
	void Disconnect();
	bool Connect(const std::string& ip, int& port);
	ChatRoom* GetCurrentRoom() const;
	SOCKET GetSocket() const;
	int GetCurrentRoomId() const;
	void SetCurrentRoomId(int& roomId);
	void SetCurrentRoom(ChatRoom* room);
	void SetNickname(const std::string& nickname);
};

ChatMember::ChatMember() {
	this->socket = INVALID_SOCKET;
	this->isConnected = false;
	this->currentRoom = nullptr;
	this->currentRoomId = -1;
}

ChatMember::ChatMember(SOCKET s)
{
	this->socket = s;
	this->isConnected = true;
	this->currentRoom = nullptr;
	
}

bool ChatMember::Send(const std::string& message) {
	if (!isConnected) return false;

	std::string msg = message + "\n";
	int sent = ::send(socket, msg.c_str(), msg.size(), 0);

	if (sent == SOCKET_ERROR) {
		isConnected = false;
		closesocket(socket);
		return false;
	}
	return true;
}

const std::string& ChatMember::GetNickname() const {
	return nickname;
}

void ChatMember::SetNickname(const std::string& nickname) {
	this->nickname = nickname;
}

inline bool ChatMember::isActive() const
{
	return this->isConnected;
}

void ChatMember::Disconnect() {
	if (isConnected) {
		closesocket(this->socket);
		this->isConnected = false;
	}
}

bool ChatMember::Connect(const std::string& ip, int& port) {
	WSADATA wsadata;

	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		return false;
	}

	this->socket = ::socket(AF_INET, SOCK_STREAM, 0);

	if (this->socket == INVALID_SOCKET) {
		WSACleanup();
		return false;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());
	
	if (serverAddr.sin_addr.s_addr == INADDR_NONE) {
		closesocket(this->socket);
		WSACleanup();
		return false; 
	}

	if (::connect(this->socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		closesocket(this->socket);
		WSACleanup();
		return false;
	}

	this->isConnected = true;
	return true;
}
bool ChatMember::ReceiveMessages(std::vector<std::string>& OutMessages) {
	if (!isConnected) { return false; }
	char temp[1024];
	int bytes = recv(this->socket, temp, sizeof(temp) - 1, 0);
	if (bytes == 0) {
		this->isConnected = false;
		closesocket(this->socket);
		return false;
	}
	if (bytes == SOCKET_ERROR) {
		this->isConnected = false;
		closesocket(this->socket);
		return false;
	}
	this->recvbuffer.append(temp, bytes);
	size_t pos;
	while ((pos = this->recvbuffer.find('\n')) != std::string::npos) {
		std::string msg = recvbuffer.substr(0, pos);
		if (!msg.empty() && msg.back() == '\r') {
			msg.pop_back();
		}
		OutMessages.push_back(msg);
		this->recvbuffer.erase(0, pos + 1);
	}
	return true;
}
ChatRoom* ChatMember::GetCurrentRoom() const {
	return this->currentRoom;
}

SOCKET ChatMember::GetSocket() const {
	return this->socket;
}

void ChatMember::SetCurrentRoom(ChatRoom* room)  {
	this->currentRoom = room;
}

int ChatMember::GetCurrentRoomId() const {
	return this->currentRoomId;
}

void ChatMember::SetCurrentRoomId(int& roomId){
	this->currentRoomId = roomId;
}




