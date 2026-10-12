#pragma once
#include <WinSock2.h>
#include <map>
#include <vector>
#include <string>
#include <memory>

class ChatRoom;
class ChatMember;

class ChatServer {
	SOCKET listenSocket;
	fd_set MasterSet;
	std::map <int, std::unique_ptr <ChatRoom>> Rooms;
	std::vector<std::unique_ptr<ChatMember>> Members;
	bool isRunning;


	void AcceptClient();
	void ProcessClient(ChatMember* member);
	ChatRoom* FindRoom(int RoomId);
	void JoinRoom(ChatMember* member, int RoomId, const std::string& nickname);
	void LeaveRoom(ChatMember* member);
	void RemoveClient(ChatMember* member);

public:
	ChatServer()
	{
		this->listenSocket = INVALID_SOCKET;
		this->isRunning = false;
	}
	~ChatServer();

	bool StartServer(int& port);
	void Run();
	void StopServer();
	bool CreateRoom(const int &roomId, const std::string& roomName);
};