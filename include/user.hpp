#pragma once

#include <database.hpp>
#include <server.hpp>

namespace gg {
	using string = std::string;
	using list = gold::list;
	using namespace gold;

	struct user : public model {
	 public:
		static std::map<string, string> nameMap;
		static object& getPrototype();
		static collection col;
		user();
		user(object data);

		gold::var checkPassword(list args);
		gold::var changePassword(list args);
		gold::var changeEmail(list args);
		gold::var getIcon(list args);
		gold::var addFriend(list args);
		gold::var removeFriend(list args);
		gold::var blockUser(list args);
		gold::var deleteUser(list args);

		static gold::var generateHash(string value, string salt);
		static void setRoutes(database, server);
		static gold::var create(object data);
		static gold::var login(string email, string password);

		// Utilities
		static string transformBirthday(string value);

		// Site rendering
		static gold::list userHome(struct session& sesh);
		static gold::list userLogin(
			string response = "",
			string emailError = "",
			string passwordError = "");
		static gold::list userRegister(obj data, object errs);
		static gold::list userOptions(obj data, object errs);

		// Validation
		static bool invalidEmail(string email, string& error);
		static bool invalidPassword(string password, string& error);
		static bool invalidFirstName(string fName, string& error);
		static bool invalidMidName(string mName, string& error);
		static bool invalidLastName(string lName, string& error);
		static bool invalidCompany(string com, string& error);
		static bool invalidPhone(string b, string& error);
		static bool invalidGender(string b, string& error);
		static bool invalidBirthday(string b, string& error);
		static bool invalidAddress1(string a1, string& error);
		static bool invalidAddress2(string a2, string& error);
		static bool invalidCountry(string c, string& error);
		static bool invalidCity(string c, string& error);
		static bool invalidZone(string z, string& error);
		static bool invalidZip(string z, string& error);
		static bool invalidIcon(string id, string& error);
		static bool invalidUserType(string t, string& error);

		static gold::var findOne(list args);
		static gold::var findMany(list args);
	};

}  // namespace gg