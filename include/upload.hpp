#pragma once

#include <database.hpp>
#include <server.hpp>

#include "user.hpp"

namespace gg {
	using namespace gold;

	using list = gold::list;
	using var = gold::var;

	struct upload : public model {
		static void createUploadFolder();
		static bool createUpload(string p, string_view data);
	 public:
		static object& getPrototype();
		static collection col;
		upload();
		upload(object data);

		var addOwners(list args);
		var removeOwners(list args);
		var isOwner(list args);
		var getOwners(list args);

		static list uploadOptions(user u, obj data, obj errs);
		static list uploadFind(obj data, obj errs);
		static list uploadList(obj filter, list results);
		static list uploadIndex(upload b);

		static void setRoutes(database, server);
		static var findOne(list args);
		static var findMany(list args);
	};
}  // namespace gg