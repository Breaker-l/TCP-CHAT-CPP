#pragma once
#include <fstream>
#include <list>
class ChatMember;

class ChatRoom {
	std::fstream Messages;
	int chatId;
	std::string file_name;
	std::list<ChatMember*> Members;
	std::string chatName;
public:
	ChatRoom(const int& id,const std::string& name)
	{
		this->chatId = id;
		file_name = name;
		file_name += ".txt";
		std::ifstream test(file_name);
		if (!test) {
			std::ofstream create(file_name);
			create.close();
		}

		Messages.open(file_name, std::ios::in | std::ios::out | std::ios::app);
	}

	bool AddMember(ChatMember* member);
	void RemoveMember(ChatMember* member);

	void Broadcast(const std::string& message, ChatMember* exclude = nullptr);
	void SendHistoryTo(ChatMember* member);

	int GetId() const;
	size_t GetMemberCount() const;
	bool IsEmpty() const;

};

