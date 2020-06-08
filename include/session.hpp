#pragma once

#include <database.hpp>

#include "user.hpp"

namespace gg {
	using list = gold::list;
	using var = gold::var;
	using namespace gold;

	struct session : public model {
	 protected:
		static collection col;

		static var getSession(list args);
		static var getSeshUser(list args);

	 public:
		static object& getPrototype();
		session();
		session(object data);
		session(user user);

		var isExpired(list args = {});
		var getUser(list args = {});
		var reset(list args = {});
		var save(list args = {});
		var writeSession(list args);

		static void setRoutes(database, server);
		static var findOne(list args);
	};

	void setSessionRoute(database db, server serv);

}  // namespace gg