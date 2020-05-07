#pragma once

#include <database.hpp>
#include <server.hpp>

#include "user.hpp"

namespace gg {
	using namespace gold;

	struct band : public model {
	 public:
		static object& getPrototype();
		static collection col;
		band();
		band(object data);

		gold::var addOwners(list args);
		gold::var removeOwners(list args);
		gold::var isOwner(list args);
		gold::var getOwners(list args);

		static gold::list bandCreate(user u, obj data, obj errs);
		static gold::list bandOptions(user u, obj data, obj errs);
		static gold::list bandFind(obj data, obj errs);
		static gold::list bandList(
			obj filter, gold::list results);
		static gold::list bandIndex(band& b);

		static void setRoutes(database, server);
		static gold::var findOne(list args);
		static gold::var findMany(list args);
	};
}  // namespace gg