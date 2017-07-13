#pragma once

constexpr size_t OID_SIZE = 12;

namespace model {
	class Player {
	public:
		bsoncxx::oid _id;

		std::string name;
		int32_t level;
		bsoncxx::oid user_oid;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(CEREAL_NVP(_id),
				CEREAL_NVP(name), CEREAL_NVP(level)
				, CEREAL_NVP(user_oid));
		}
	};

	class User : public boson::UnderlyingBSONDataBase
	{
	public:
		bsoncxx::oid _id;
		std::string name;
		std::string email;
		bsoncxx::types::b_binary uuid;
		std::vector<bsoncxx::oid> players;
		std::chrono::system_clock::time_point register_date;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(CEREAL_NVP(_id), 
				CEREAL_NVP(name), CEREAL_NVP(email), CEREAL_NVP(uuid)
				, CEREAL_NVP(players), CEREAL_NVP(register_date)
				, CEREAL_NVP(data)
			);
		}
	};

	class Character : public boson::UnderlyingBSONDataBase
	{
	public:
		bsoncxx::oid _id;
		int64_t wid;
		int32_t sClass;
		std::string nickname;
		int32_t level;
		int32_t exp;
		int32_t awaken;
		std::chrono::system_clock::time_point login_time;
		std::chrono::system_clock::time_point  reg_date;

		template <class Archive>
		void serialize(Archive& ar) {
			ar(CEREAL_NVP(_id),
				CEREAL_NVP(wid), CEREAL_NVP(sClass), CEREAL_NVP(nickname)
				, CEREAL_NVP(level), CEREAL_NVP(exp), CEREAL_NVP(awaken)
				, CEREAL_NVP(login_time), CEREAL_NVP(reg_date)
			);
		}
	};
}
