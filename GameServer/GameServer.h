#pragma once
#include "ServerBase.h"
#include "ConnectionBase.h"
#include "MongoConnection.h"

class GameConnection : public ConnectionBase
{
private:

public:
	GameConnection();
	~GameConnection();

	void		SendProtoBuf(shared_ptr<google::protobuf::Message> msg);
};


struct GameServerConfig
	: public websocketpp::config::asio
{
	typedef GameConnection connection_base;
};

class GameServer :
	public ServerBase<GameServerConfig>
{
public:
	GameServer(asio::io_service* ios);
protected:
	void on_open(websocketpp::connection_hdl hdl) override {
		ServerBase::on_open(hdl);
	}
	void on_close(websocketpp::connection_hdl hdl) override {
		ServerBase::on_close(hdl);
	}
	template<typename K>
	void OnPacket(connection_ptr con, shared_ptr<google::protobuf::Message> msg)
	{
		VLOG(1) << "OnPacket() \n" << msg->DebugString();
		OnPacket(con, static_pointer_cast<K>(msg));
	}

	DECLARE_PACKET_HANDLER(Echo)
	DECLARE_PACKET_HANDLER(Broadcast)
	// sample
	DECLARE_PACKET_HANDLER(RegisterDevice)
	DECLARE_PACKET_HANDLER(SignIn)
	DECLARE_PACKET_HANDLER(CreatePlayer)

protected:
	MongoConnection _db;
};
