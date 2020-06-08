#pragma once

#include <database.hpp>
#include <server.hpp>

#include "user.hpp"

namespace gg {
	using namespace gold;
	using string = std::string;
	using list = gold::list;
	using var = gold::var;

	struct band : public model {
	 public:
		static object& getPrototype();
		static collection col;
		band();
		band(object data);

		var addOwners(list args);
		var removeOwners(list args);
		var isOwner(list args);
		var getOwners(list args);

		static list bandCreate(user u, obj data, obj errs);
		static list bandOptions(user u, obj data, obj errs);
		static list bandFind(obj data, obj errs);
		static list bandList(obj filter, list results);
		static list bandIndex(band b);

		static void setRoutes(database, server);
		static var findOne(list args);
		static var findMany(list args);
	};
}  // namespace gg