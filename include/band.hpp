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
		static std::map<string, string> nameMap;
		static object& getPrototype();
		band();
		band(object data);

		var addOwners(list args);
		var removeOwners(list args);
		var isOwner(list args);
		var getOwners(list args);

		var save(list args = {});

		static list bandCreate(session sesh, user u, obj data, obj errs);
		static list bandOptions(session sesh, user u, band item);
		static list bandFind(session sesh, user u, obj data, list items);
		static list bandList(
			session sesh, user u, obj filter, list results);
		static list bandIndex(session sesh, user u, band b);

		static void setRoutes(database, server);
		static var findOne(list args);
		static var findMany(list args);
	};
}  // namespace gg