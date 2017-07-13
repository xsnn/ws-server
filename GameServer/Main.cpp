#include "stdafx.h"
#include "BGServer.h"

static bool ValidatePort(const char* flagname, int32 value) {
	if (value > 0 && value < 32768)   // value is ok
		return true;
	printf("Invalid value for --%s: %d\n", flagname, (int)value);
	return false;
}
DEFINE_int32(port, 80, "What port to listen on");
DEFINE_validator(port, &ValidatePort);
DEFINE_int32(ssl_port, 443, "What ssl port to listen on");
DEFINE_validator(ssl_port, &ValidatePort);
DEFINE_int32(npc_port, 1080, "What port to npc listen on");
DEFINE_validator(npc_port, &ValidatePort);
DEFINE_string(listen, "localhost,0.0.0.0", "listen address");
DEFINE_string(locale, "", "set locale");

int main(int argc, char** argv)
{
	FLAGS_logtostderr = 1;
	FLAGS_stderrthreshold = 0;
	FLAGS_log_dir = "./logs";
	FLAGS_colorlogtostderr = true;
	google::ParseCommandLineFlags(&argc, &argv, false);
	google::InitGoogleLogging(argv[0]);

	constexpr auto buildDate = "(Build )";
 	LOG(INFO) << buildDate;

	setlocale(LC_ALL, "");// FLAGS_locale);
#if true // odb test
	try {
		auto pgdb = make_unique<odb::pgsql::database>("postgres", "", "game_test_db", "10.1.0.106", 5432);
		{
			odb::transaction pt(pgdb->begin());
			platformTable::item item(0x18, u8"½Î±¸·Á", "{ option=1 }");
			auto item_id = pgdb->persist(item);
			for (auto result : pgdb->query<platformTable::item>())
			{
				cout << "test" << endl;
			}
			pt.commit();
		}
		auto db = make_unique<odb::sqlite::database>(
			"./DataTable/pc.db3"
			, SQLITE_OPEN_READONLY);
		odb::transaction t(db->begin());
		for (auto result : db->query<gameData::PC_Awaken>())
		{
			auto ws = U8TOWS(result.Description);
			cout << result.UID << "\t" << result.ResName << "\t";
			wcout << ws << endl;
		}
		for (auto result : db->query<gameData::PC_Stat_Growth>())
		{
			cout << result.PC_UID << "\t" << result.Attack_Hit << endl;
		}
		for (auto result : db->query<gameData::PC_EXP>())
		{
			cout << result.Level << "\t" << result.Exp << endl;
		}
		for (auto result : db->query<gameData::PC_Class>())
		{
			cout << result.PC_UID << endl;
		}
		t.commit();
	}
	catch (const odb::exception& e)
	{
		cerr << e.what() << endl;
		return 1;
	}
#endif

	GOOGLE_PROTOBUF_VERIFY_VERSION;
	try
	{
		auto _ioService = make_unique<asio::io_service>();

		auto _bloodGodServer = make_unique<BGServer>(_ioService.get());
		_bloodGodServer->StartServer(FLAGS_port, FLAGS_npc_port);

		LOG(INFO) << "Server get ready on game(" << FLAGS_port << "), npc(" << FLAGS_npc_port << ")...";

		_ioService->run();

		LOG(INFO) << "Server Shutdown";
	}
	catch (websocketpp::exception const & e) {
		LOG(FATAL) << e.what();
		std::cout << e.what() << std::endl;
	}
	catch (const std::exception& e) {
		DLOG(FATAL) << e.what();
		//std::cout << e.what() << std::endl;
	}
	google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
