#pragma once

#include <database.hpp>
#include <server.hpp>

#include "user.hpp"

namespace gg {
	using namespace gold;

	struct venue : public model {
	 public:
		static object& getPrototype();
		static collection col;
		venue();
		venue(object data);

		gold::var addOwners(list args);
		gold::var removeOwners(list args);
		gold::var isOwner(list args);
		gold::var getOwners(list args);

		static gold::list venueCreate(user u, obj data, obj errs);
		static gold::list venueOptions(user u, venue ven);
		static gold::list venueFind(obj data, obj errs);
		static gold::list venueList(
			obj filter, gold::list results);
		static gold::list venueIndex(venue& b);

		static void setRoutes(database, server);
		static gold::var findOne(list args);
		static gold::var findMany(list args);
	};
}  // namespace gg