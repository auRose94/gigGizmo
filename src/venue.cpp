#include "venue.hpp"

#include "error.hpp"
#include "template.hpp"
#include "user.hpp"

namespace gg {

	collection venue::col;

	obj& venue::getPrototype() {
		static auto proto = obj(initList{
			{"name", ""},
			{"contact", ""},
			{"shortDesc", ""},
			{"longDesc", ""},
			{"accent", ""},
			{"icon", ""},
			{"location", obj()},
			{"uploads", gold::var(gold::list())},
			{"owners", gold::var(gold::list())},
			{"hide", false},
			{"proto", model::getPrototype()},
		});
		return proto;
	}

	venue::venue() : model(col) { setParent(getPrototype()); }

	venue::venue(obj data) : model(col, data) {
		setParent(getPrototype());
	}

	gold::var venue::addOwners(list args) {
		auto owners = getList("owners");
		if (owners)
			for (auto it = args.begin(); it != args.end(); ++it) {
				if (it->isObject()) {
					auto o = it->getObject();
					auto id = o.getString("_id");
					owners.pushString(id);
				} else if (it->isString()) {
					auto id = it->getString();
					owners.pushString(id);
				}
			}
		return gold::var();
	}

	gold::var venue::removeOwners(list args) {
		auto owners = getList("owners");
		if (owners)
			for (auto it = args.begin(); it != args.end(); ++it) {
				if (it->isObject()) {
					auto o = it->getObject();
					auto id = o.getString("_id");
					auto oit = owners.find(id);
					if (oit != owners.end()) owners.erase(oit);
				} else if (it->isString()) {
					auto id = it->getString();
					auto oit = owners.find(id);
					if (oit != owners.end()) owners.erase(oit);
				}
			}
		return gold::var();
	}

	gold::var venue::isOwner(list args) {
		auto owners = getList("owners");
		if (args.size() > 1) {
			auto ret = gold::list();
			for (auto it = args.begin(); it != args.end(); ++it) {
				if (it->isObject()) {
					auto o = it->getObject();
					auto id = o.getString("_id");
					auto oit = owners.find(id);
					if (oit != owners.end())
						ret.pushBool(true);
					else
						ret.pushBool(false);
				} else if (it->isString()) {
					auto id = it->getString();
					auto oit = owners.find(id);
					if (oit != owners.end())
						ret.pushBool(true);
					else
						ret.pushBool(false);
				} else {
					ret.pushBool(false);
				}
			}
			return ret;
		} else {
			// 1
			auto arg = args[0];
			if (arg.isObject()) {
				auto o = arg.getObject();
				auto id = o.getString("_id");
				auto oit = owners.find(id);
				if (oit != owners.end()) return true;
			} else if (arg.isString()) {
				auto id = arg.getString();
				auto oit = owners.find(id);
				if (oit != owners.end()) return true;
			}
		}
		return false;
	}

	gold::var venue::getOwners(list args) {
		auto owners = getList("owners");
		auto find = gold::obj(initList{
			{"_id", owners},
		});
		return user::findMany({find});
	}

	void venue::setRoutes(database db, server serv) {
		auto colRes = db.getCollection({"venue"});
		if (!colRes.isObject())
			cerr << "Could not get venue collection" << endl;
		col = colRes.getObject<collection>();

		func getVenue = [](const list& args) -> gold::var {
			using namespace HTML;
			serveArgs(args, req, res);
			auto sessionID = req.getParameter({0}).getString();
			auto venueID = req.getParameter({1}).getString();
			if (venueID.size() == 0) venueID = sessionID;
			auto venRes = venue::findOne({obj({{"_id", venueID}})});
			auto ven = venRes.getObject<venue>();
			auto content = gold::list();
			if (ven)  // Found
				content = venue::venueIndex(ven);
			else  // Not
				content = errorPage({genericError("Venue not found.")});

			auto resp = getTemplate(req, content);

			auto d = (string)(resp);
			res.writeHeader({"Content-Type", "text/html"});
			return res.end({d});
		};

		func getListVenues = [](const list& args) -> gold::var {
			serveArgs(args, req, res);

			auto content = gold::list();

			auto seshId = req.getParameter({0}).getString();
			auto found = session::findOne({obj{{"_id", seshId}}});
			if (found.isObject(session::getPrototype())) {
				auto sesh = found.getObject<session>();
				auto uVar = sesh.getUser();
				auto u = uVar.getObject<user>();
				if (u) {
					auto find = obj({
						{"owners", u.getID()},
					});
					auto opt = obj({});
					auto resp = venue::findMany({find, opt});
					if (resp.isError()) {
						auto err = resp.getError();
						return res.end({err->what()});
					}
					auto venues = resp.getList();
					content = venue::venueList(u, venues);
				} else
					content = errorPage(
						{genericError("User not found in session.")});
			} else
				content =
					errorPage({genericError("Session not found.")});

			auto resp = getTemplate(req, content);

			auto d = (string)(resp);
			res.writeHeader({"Content-Type", "text/html"});
			return res.end({d});
		};

		func getCreateVenue = [](const list& args) -> gold::var {
			serveArgs(args, req, res);

			auto content = gold::list();

			auto seshId = req.getParameter({0}).getString();
			auto found = session::findOne({obj{{"_id", seshId}}});
			if (found.isObject(session::getPrototype())) {
				auto sesh = found.getObject<session>();
				auto uVar = sesh.getUser();
				auto u = uVar.getObject<user>();
				if (u)
					content = venue::venueCreate(u, obj{}, obj{});
				else
					content = errorPage(
						{genericError("User not found in session.")});
			} else
				content =
					errorPage({genericError("Session not found.")});

			auto resp = getTemplate(req, content);

			auto d = (string)(resp);
			res.writeHeader({"Content-Type", "text/html"});
			return res.end({d});
		};

		func putVenue = [](const list& pArgs) -> gold::var {
			using namespace HTML;
			serveArgs(pArgs, req, res);
			std::string* buffer = new string();

			func onDataCallback = [&](const list& args) -> gold::var {
				serveArgs(pArgs, req, res);
				auto chuck = args[0].getString();
				auto isEnd = args[1].getBool();
				(*buffer) = (*buffer) + chuck;
				if (isEnd) {
					auto d = html();
					auto t = "text/html";
					if (req.isWWWFormURLEncoded()) {
						auto params = obj();
						obj::parseURLEncoded(*buffer, params);

						auto seshId = req.getParameter({0}).getString();
						auto venueId = req.getParameter({1}).getString();
						auto found =
							session::findOne({obj{{"_id", seshId}}});
						if (found.isObject(session::getPrototype())) {
							auto sesh = found.getObject<session>();
							auto uVar = sesh.getUser();
							auto u = uVar.getObject<user>();
							if (u) {
								auto venRet =
									venue::findOne({obj{{"_id", venueId}}});
								if (venRet.isError()) {
									d = getTemplate(req, errorPage({venRet}));
								} else {
									auto ven = venRet.getObject<venue>();
									ven.copy(params);
									auto saved = ven.save();
									if (!saved.isError())
										d = getTemplate(req, venueOptions(u, ven));
									else
										d = getTemplate(req, errorPage({saved}));
								}
							} else
								d = getTemplate(
									req,
									errorPage({genericError(
										"Could not get user from session.")}));
						} else if (found.isError())
							d = getTemplate(req, errorPage({found}));
					}

					res.writeHeader({"Content-Type", t});
					res.end({string(d)});
				}
				delete buffer;
				return gold::var();
			};

			func onAbort = [&](const list& args) -> gold::var {
				serveArgs(pArgs, req, res);
				auto error = args[0].getError();
				gold::list content = errorPage({*error});
				auto d = (string)getTemplate(req, content);
				res.writeHeader({"Content-Type", "text/html"});
				res.end({d});
				return gold::var();
			};

			res.onAborted({onAbort});
			res.onData({onDataCallback});

			return gold::var();
		};

		func postCreateVenue = [](const list& pArgs) -> gold::var {
			serveArgs(pArgs, req, res);
			std::string* buffer = new string();

			func onDataCallback = [=](const list& args) -> gold::var {
				serveArgs(args, req, res);
				auto chuck = args[0].getString();
				auto isEnd = args[1].getBool();
				(*buffer) = (*buffer) + chuck;
				if (isEnd) {
					auto d = html();
					auto t = "text/html";
					if (req.isWWWFormURLEncoded()) {
						auto params = obj();
						obj::parseURLEncoded(*buffer, params);
						auto seshId = req.getParameter({0}).getString();
						auto venueId = req.getParameter({1}).getString();
						auto found =
							session::findOne({obj{{"_id", seshId}}});
						if (found.isObject(session::getPrototype())) {
							auto sesh = found.getObject<session>();
							auto uVar = sesh.getUser();
							auto u = uVar.getObject<user>();
							if (u) {
								auto venRes =
									venue::findOne({obj({{"_id", venueId}})});
								if (venRes.isError())
									d = getTemplate(req, errorPage({venRes}));
								else if (venRes.isObject()) {
									auto ven = venRes.getObject<venue>();
									ven.copy(params);
									auto resp = ven.save();
									// TODO: Do check, show errors
									if (resp.isError())
										d = getTemplate(req, errorPage({resp}));
									else
										d = getTemplate(
											req, venueCreate(u, ven, obj{}));
								}
							} else
								d = getTemplate(
									req,
									errorPage({genericError(
										"Could not get user from session.")}));
						} else
							d = getTemplate(req, errorPage({found}));
					} else
						d = getTemplate(
							req,
							errorPage({genericError(
								"Expected URL Form encoded content.")}));

					res.writeHeader({"Content-Type", t});
					res.end({string(d)});
				}
				delete buffer;
				return gold::var();
			};

			func onAbort = [=](const list& args) -> gold::var {
				serveArgs(args, req, res);
				auto error = args[0].getError();
				gold::list content = errorPage({*error});
				auto d = (string)getTemplate(req, content);
				res.writeHeader({"Content-Type", "text/html"});
				res.end({d});
				return gold::var();
			};

			res.onAborted({onAbort});
			res.onData({onDataCallback});

			return gold::var();
		};

		serv.get({R"(/venue/:venueID)", getVenue});
		serv.get({R"(/:sessionID/venue/:venueID)", getVenue});
		serv.get({R"(/:sessionID/list/venue)", getListVenues});
		serv.get({R"(/:sessionID/venue)", getCreateVenue});

		serv.post({R"(/:sessionID/venue)", postCreateVenue});
		serv.put({R"(/:sessionID/venue/:venueID)", putVenue});
	}

	gold::var venue::findOne(list args) {
		auto value = venue::col.findOne(args);
		return collection::setParentModel(
			value, venue::getPrototype());
	}

	gold::var venue::findMany(list args) {
		auto value = venue::col.findMany(args);
		return collection::setParentModel(
			value, venue::getPrototype());
	}
}  // namespace gg