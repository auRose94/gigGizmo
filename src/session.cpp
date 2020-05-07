#include "session.hpp"

#include <database.hpp>
#include <iostream>

#include "user.hpp"
#include "utility.hpp"

namespace gg {

	collection session::col;
	const auto expireOffset = 86400;

	obj& session::getPrototype() {
		static auto proto = obj({
			{"user", ""},
			{"app", ""},
			{"expire", 0},
			{"isExpired", method(&session::isExpired)},
			{"getUser", method(&session::getUser)},
			{"reset", method(&session::reset)},
			{"save", method(&session::save)},
			{"proto", model::getPrototype()},
		});
		return proto;
	}

	session::session() : model(col) { setParent(getPrototype()); }

	session::session(user u) : model(col) {
		setParent(getPrototype());
		setString("user", u.getID());
		setInt64("expire", getMonoTime() + expireOffset);
	}

	session::session(obj data) : model(col, data) {
		setParent(getPrototype());
	}

	var session::isExpired(list) {
		auto current = getMonoTime();
		auto expire = getInt64("expire");
		return current <= expire;
	}

	var session::getUser(list) {
		auto id = getString("user");
		return user::findOne({obj({
			{"_id", id},
		})});
	}

	var session::reset(list) {
		auto now = getMonoTime();
		setInt64("expire", now + expireOffset);
		return var();
	}

	var session::save(list args) {
		auto ret = model::save(args);
		if (ret.isObject()) {
			auto o = ret.getObject();
			copy(o);
		}
		auto indRes = col.addIndexes({
			"session",
			obj{{"expire", 1}},
			obj{{"expireAfterSeconds", 0}},
		});
		if (indRes.isError()) cerr << indRes << endl;
		return ret;
	}

	void session::setRoutes(database db, server) {
		auto colRes = db.getCollection({"session"});
		if (colRes.isError()) cerr << colRes << endl;
		col = colRes.getObject<collection>();
		getPrototype().setObject("col", col);
	}

	gold::var session::findOne(list args) {
		auto value = session::col.findOne(args);
		return collection::setParentModel(
			value, session::getPrototype());
	}

}  // namespace gg