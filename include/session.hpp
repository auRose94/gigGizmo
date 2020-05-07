#pragma once

#include <database.hpp>

#include "user.hpp"

using list = gold::list;

namespace gg {
	using namespace gold;

	struct session : public model {
	 protected:
		static collection col;

	 public:
		static object& getPrototype();
		session();
		session(object data);
		session(user user);

		gold::var isExpired(list args = {});
		gold::var getUser(list args = {});
		gold::var reset(list args = {});
		gold::var save(list args = {});

		static void setRoutes(database, server);
		static gold::var findOne(list args);
	};

	void setSessionRoute(database db, server serv);

}  // namespace gg