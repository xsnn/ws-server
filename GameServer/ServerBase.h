#pragma once
template<typename T>
struct TypeParseTraits
{
	static const char* name;
};
#define REGISTER_PARSE_TYPE(X) \
	template <> const char* TypeParseTraits<X>::name = #X;
#define REGISTER_PACKET_HANDLER(C,X) \
	
	)
#define INLINE_PACKET_HANDLER(X) \
	
	}
#define DECLARE_PACKET_HANDLER(X) \
	
#define IMPLEMENT_PACKET_HANDLER(C,X) \
	

template <typename T>
class ServerBase abstract : public websocketpp::server<T>
{
public:
	ServerBase(asio::io_service* ios)
	{
		this->init_asio(ios);

		this->set_error_channels(websocketpp::log::elevel::warn);
		this->set_access_channels(websocketpp::log::alevel::all);
		this->clear_access_channels(websocketpp::log::alevel::frame_payload);

		this->set_open_handler(boost::bind(&ServerBase::on_open, this, _1));
		this->set_close_handler(boost::bind(&ServerBase::on_close, this, _1));
		this->set_message_handler(boost::bind(&ServerBase::on_message, this, _1, _2));
	}
	virtual ~ServerBase() {}

	typedef set<websocketpp::connection_hdl, owner_less<websocketpp::connection_hdl> > con_list;

	virtual void on_open(websocketpp::connection_hdl hdl) {
		connection_ptr con = this->get_con_from_hdl(hdl);

		lock_guard<mutex> guard(_connection_lock);
		_connections.insert(hdl);
	}
	virtual void on_close(websocketpp::connection_hdl hdl) {
		lock_guard<mutex> guard(_connection_lock);
		_connections.erase(hdl);
	}
	virtual void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
		switch (msg->get_opcode())
		{
		case websocketpp::frame::opcode::value::text:
			OnTextMessage(hdl, msg);
			break;
		case websocketpp::frame::opcode::value::binary:
			OnBinaryMessage(hdl, msg);
			break;
		default:
			DLOG(WARNING) << "on_message " << hdl.lock().get() << ", " << msg->get_opcode();
			break;
		}
	}
	virtual void OnTextMessage(websocketpp::connection_hdl hdl, message_ptr msg) {
		try {
			lock_guard<mutex> guard(_connection_lock);
			for each (auto it in _connections) {
				this->send(it, msg->get_payload(), msg->get_opcode());
			}
		}
		catch (const websocketpp::lib::error_code& e) {
			std::cout << "Echo failed : " << e << "(" << e.message() << ")" << std::endl;
		}
	}
	virtual void OnBinaryMessage(websocketpp::connection_hdl hdl, message_ptr msg) {
		websocketpp::lib::error_code  ec;
		connection_ptr con = this->get_con_from_hdl(hdl, ec);
		if (con)
		{
			auto payload = msg->get_payload();
			if (false == OnProtoBuf(con, payload))
			{
				DLOG(WARNING) << "No Packet Handler";
			}
		}
		else
		{
			LOG(ERROR) << ec.message();
		}
	}
	bool OnProtoBuf(connection_ptr con, string payload)
	{
		auto istream = make_shared<io::ArrayInputStream>(payload.data(), (int)payload.size());
		auto cstream = make_shared<io::CodedInputStream>(istream.get());

		

		return false;
	}
	void SendProtoBufToList(shared_ptr<google::protobuf::Message> msg, con_list conList)
	{
	}
	void SendProtoBufToAll(shared_ptr<google::protobuf::Message> msg)
	{
		lock_guard<mutex> guard(_connection_lock);
		for each (auto it in _connections) {
			auto con = this->get_con_from_hdl(it);
			SendProtoBuf(con, msg);
		}
	}
	void SendProtoBuf(connection_ptr con, shared_ptr<google::protobuf::Message> msg)
	{
		if (con == NULL)
			return;

		VLOG(1) << "SendProtoBuf() \n" << msg->DebugString();
		auto name = msg->GetDescriptor()->name();
		auto len = (uint8_t)name.size();
		
		

		DLOG(INFO) << name << " sending " << cstream.ByteCount() << " bytes";

		websocketpp::lib::error_code ec = con->send(out_string.data(), cstream.ByteCount(), websocketpp::frame::opcode::value::binary);
		CHECK(!ec) << "send failed";
	}
protected:
	con_list _connections;
	atomic<int> _con_count;
	mutex _connection_lock;

protected:
	std::map<string, tuple< shared_ptr<google::protobuf::Message>, boost::function<void(connection_ptr, shared_ptr<google::protobuf::Message>)> > > _callbackInfoMap;
};
