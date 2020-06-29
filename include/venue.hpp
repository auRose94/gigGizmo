#pragma once

#include <database.hpp>
#include <server.hpp>

#include "user.hpp"

namespace gg {
	using namespace gold;
	using list = gold::list;
	using var = gold::var;

	struct venue : public model {
	 public:
		static std::map<string, string> nameMap;
		static object& getPrototype();
		venue();
		venue(object data);

		var addOwners(list args);
		var removeOwners(list args);
		var isOwner(list args);
		var getOwners(list args);

		var save(list args = {});

		static list venueCreate(session sesh, user u, obj data, obj errs);
		static list venueOptions(session sesh, user u, venue ven);
		static list venueFind(session sesh, user u, obj data, list venues);
		static list venueList(
			session sesh, user u, obj filter, list results);
		static list venueIndex(session sesh, user u, venue b);

		static void setRoutes(database, server);
		static var findOne(list args);
		static var findMany(list args);
	};
}  // namespace gg