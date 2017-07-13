#pragma once

class ConnectionBase : public websocketpp::connection_base
{
public:
	boost::uuids::uuid sessionId = boost::uuids::basic_random_generator<boost::mt19937>()();
};

