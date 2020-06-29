#pragma once

#include <database.hpp>
#include <server.hpp>

#include "user.hpp"

namespace gg {
	using namespace gold;
	using string = std::string;
	using list = gold::list;
	using var = gold::var;

	struct gig : public model {
	 public:
		static std::map<string, string> nameMap;
		static object& getPrototype();
		gig();
		gig(object data);

		var addOwners(list args);
		var removeOwners(list args);
		var isOwner(list args);
		var getOwners(list args);

		var save(list args = {});

		static list gigCreate(session sesh, user u, obj data, obj errs);
		static list gigOptions(session sesh, user u, gig item);
		static list gigFind(session sesh, user u, obj data, list items);
		static list gigList(
			session sesh, user u, obj filter, list results);
		static list gigIndex(session sesh, user u, gig b);

		static void setRoutes(database, server);
		static var findOne(list args);
		static var findMany(list args);
	};
}  // namespace gg