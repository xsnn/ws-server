#include "stdafx.h"
#include "GameServer.h"
#include "utils.h"


namespace fLI {
	extern google::int32 FLAGS_port;
}


REGISTER_PARSE_TYPE(Echo);
REGISTER_PARSE_TYPE(Broadcast);
REGISTER_PARSE_TYPE(Heartbeat);
// game packets
REGISTER_PARSE_TYPE(RegisterDevice);
REGISTER_PARSE_TYPE(SignIn);
REGISTER_PARSE_TYPE(CreatePlayer);

GameConnection::GameConnection()
{
}
GameConnection::~GameConnection()
{
}
void GameConnection::SendProtoBuf(shared_ptr<google::protobuf::Message> msg)
{
}

GameServer::GameServer(asio::io_service* ios)
	: ServerBase(ios)
{
	REGISTER_PACKET_HANDLER(GameServer, Echo);
	REGISTER_PACKET_HANDLER(GameServer, Broadcast);
	REGISTER_PACKET_HANDLER(GameServer, RegisterDevice);
	REGISTER_PACKET_HANDLER(GameServer, SignIn);
	REGISTER_PACKET_HANDLER(GameServer, CreatePlayer);

	_db.Connect("");

#ifdef _DEBUG
	if (80 != fLI::FLAGS_port)
		choshTest();
#endif
}

IMPLEMENT_PACKET_HANDLER(GameServer, Echo)
{
	auto ws = U8TOWS(data->text());
	auto u8 = WSTOU8(ws);
	auto mbs = WSTOMBS(ws.c_str());

	DLOG(INFO) << mbs;

	SendProtoBuf(con, data);
}

IMPLEMENT_PACKET_HANDLER(GameServer, Broadcast)
{
	auto ws = U8TOWS(data->text());
	DLOG(INFO) << WSTOMBS(ws.c_str());

	SendProtoBufToAll(data);
}

IMPLEMENT_PACKET_HANDLER(GameServer, RegisterDevice)
{
	auto uuid = data->uuid();
	DLOG(INFO) << "uuid " << boost::algorithm::hex(uuid);

	model::User user;
	user.name = u8"ÀÌ¸§Ù£í®";
	user.email = "test@test.com";
	user.uuid = bsoncxx::types::b_binary{ bsoncxx::binary_sub_type::k_uuid,
		static_cast<uint32_t>(uuid.size()),	reinterpret_cast<const uint8_t*>(uuid.c_str()) };
	user.register_date = std::chrono::system_clock::now();

	GAMEDB_INIT_COLLECTION(_db.GetNewClient(), user_col);
	mangrove::collection_wrapper<model::User> userCol(user_col);
	auto res = userCol.insert_one(user);
	if (res)
	{
		CHECK(res->inserted_id().type() == bsoncxx::type::k_oid);
		DLOG(INFO) << res->inserted_id().get_oid().value.to_string();
		DLOG(INFO) << user._id.to_string();

		bsoncxx::oid  oid = res->inserted_id().get_oid().value;
		auto oidBytes = oid.bytes();
		auto oidString = oid.to_string();

		auto updateToken = make_shared<UpdateToken>();
		auto token = updateToken->mutable_token();
		token->set_type(TokenType::update);
		token->set_body(oidBytes, OID_SIZE);

		SendProtoBuf(con, updateToken);
	}
}

IMPLEMENT_PACKET_HANDLER(GameServer, SignIn)
{
	auto updateToken = data->mutable_update_token();

	bsoncxx::oid user_oid(updateToken->data(), updateToken->size());
	DLOG(INFO) << "user_oid " << user_oid.to_string();

	auto filter = bsoncxx::builder::stream::document{} <<
		"_id" << user_oid << finalize;
	
	bsoncxx::builder::stream::document join;
	join << "players" << "$_id";


	GAMEDB_INIT_COLLECTION(_db.GetNewClient(), user_col);
	mangrove::collection_wrapper<model::User> userCol(user_col);
	GAMEDB_GET_COLLECTION(player_col);
	mangrove::collection_wrapper<model::Player> playerCol(player_col);

	auto r_user = userCol.find_one(filter.view());
	if (r_user)
	{
		DLOG(INFO) << r_user->email << r_user->name;

		auto playerFilter = bsoncxx::builder::stream::document{} <<
			"user_oid" << user_oid << finalize;

		auto sendPacket = make_shared<PlayerList>();
		auto players_r = playerCol.find(playerFilter.view());

		for each (const auto& player_r in players_r) {
			DLOG(INFO) << "playerId: " << player_r._id.to_string();
			auto player = sendPacket->add_player();
			player->set_oid(player_r._id.bytes(), OID_SIZE);
			player->set_name(player_r.name);
			player->set_level(player_r.level);
		}
		SendProtoBuf(con, sendPacket);
	}
}

IMPLEMENT_PACKET_HANDLER(GameServer, CreatePlayer)
{
	auto playerName = data->name();
	auto updateToken = data->mutable_update_token();
	bsoncxx::oid user_oid(updateToken->data(), updateToken->size());
	DLOG(INFO) << "user_oid " << user_oid.to_string();

	GAMEDB_INIT_COLLECTION(_db.GetNewClient(), user_col);
	GAMEDB_GET_COLLECTION(player_col);

	auto filter = bsoncxx::builder::stream::document{} <<
		"_id" << user_oid << finalize;
	mangrove::collection_wrapper<model::User> userCol(user_col);
	auto r_user = userCol.find_one(filter.view());
	if (r_user)
	{
		model::Player player;
		player.name = playerName;
		player.level = 0;
		player.user_oid = user_oid;

		mangrove::collection_wrapper<model::Player> playerCol(player_col);
		auto res = playerCol.insert_one(player);
		if (res)
		{
			bsoncxx::oid  new_player_oid = res->inserted_id().get_oid().value;

			r_user->players.push_back(new_player_oid);

			auto result = userCol.find_one_and_replace(filter.view(), r_user.get());
			DCHECK(result);

			auto sendPacket = make_shared<Player>();
			sendPacket->set_oid(new_player_oid.bytes(), OID_SIZE);
			sendPacket->set_name(player.name);
			sendPacket->set_level(player.level);
			SendProtoBuf(con, sendPacket);
		}
	}
}
