#include "venue.hpp"

#include "error.hpp"
#include "file.hpp"
#include "template.hpp"
#include "upload.hpp"
#include "user.hpp"

namespace gg {
	using obj = gold::obj;

	std::map<string, string> venue::nameMap =
		std::map<string, string>({
			{"name", "Name"},
			{"contact", "Email Contact"},
			{"shortDesc", "Short Description"},
			{"longDesc", "Long Description"},
			{"accent", "Accent Color"},
			{"icon", "Icon"},
			{"location", "Location"},
			{"tags", "Tags"},
			{"uploads", "Uploads"},
			{"owners", "Owners"},
			{"hide", "Hide"},
		});

	obj& venue::getPrototype() {
		static auto proto = obj(initList{
			{"name", ""},
			{"contact", ""},
			{"shortDesc", ""},
			{"longDesc", ""},
			{"accent", ""},
			{"icon", ""},
			{"location", obj()},
			{"tags", list()},
			{"uploads", list()},
			{"owners", list()},
			{"hide", false},
			{"proto", model::getPrototype()},
		});
		return proto;
	}

	venue::venue() : model() {}

	venue::venue(obj data)
		: model(
				getPrototype().getObject<collection>("collection"),
				data) {
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
		return user::findMany({obj{
			{"_id", owners},
		}});
	}

	var venue::save(list args) {
		auto iconData = getBinary("icon");
		erase("icon");
		auto saved = model::save(args);
		if (saved.isError()) return saved;
		if (iconData.size() > 0) {
			auto owners = list(getList("owners"));
			auto id = getString("_id");
			if (id.size() > 0) owners.pushString(id);
			auto icon =
				upload({{"src", iconData}, {"owners", owners}});
			auto iconSaved = icon.save();
			if (iconSaved.isError()) return iconSaved;
			setString("icon", icon.getString("_id"));
			saved = model::save(args);
			if (saved.isError()) return saved;
		}
		return *this;
	}

	void venue::setRoutes(database db, server serv) {
		auto colRes = db.getCollection({"venue"});
		if (!colRes.isObject())
			cerr << "Could not get venue collection" << endl;
		getPrototype().setObject(
			"collection", colRes.getObject<collection>());

		func getVenue = [](list args) -> gold::var {
			using namespace HTML;
			serveArgs(args, req, res);
			auto venueID = req.getParameter({0}).getString();
			auto venRes = venue::findOne({obj({{"_id", venueID}})});
			auto ven = venRes.getObject<venue>();
			if (ven) {  // Found
				return res.end(
					{getTemplate(req, venue::venueIndex(ven))});
			}

			return req.setYield({true});
		};

		func getListVenues = [](list args) -> gold::var {
			serveArgs(args, req, res);

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			if (sesh) {
				auto u = req.callMethod("getUser").getObject<user>();
				if (u) {
					auto find = obj({
						{"owners", obj({{"$in", list({u.getID()})}})},
					});
					auto opt = obj({});
					auto resp = venue::findMany({find, opt});
					if (req.acceptingJSON()) {
						res.writeHeader(
							{"Content-Type", "application/json"});
						if (resp.isError()) {
							auto e = (string)*resp.getError();
							auto j = "{\"error\":" + e + "}";
							return res.end({j});
						} else if (resp.isList()) {
							auto l = resp.getList();
							auto j = obj({
														 {"total", l.size()},
														 {"rows", l},
													 })
												 .getJSON();
							return res.end({j});
						}
					} else if (req.acceptingHTML()) {
						if (resp.isError()) {
							auto err = resp.getError();
							return res.end(
								{getTemplate(req, errorPage({*err}))});
						} else {
							auto venues = resp.getList();
							return res.end({getTemplate(
								req, venue::venueList(sesh, u, {}, venues))});
						}
					}
				}
			}

			return req.setYield({true});
		};

		func getFindVenues = [](list args) -> gold::var {
			serveArgs(args, req, res);

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			auto u = user();
			if (sesh) u = req.callMethod("getUser").getObject<user>();
			auto searchParams = obj({});
			auto find = obj({});
			auto opt = obj({});
			auto found = venue::findMany({find, opt});
			if (found.isError()) {
				auto err = found.getError();
				return res.end({getTemplate(req, errorPage({*err}))});
			} else if (found.isList()) {
				auto venues = found.getList();
				return res.end({getTemplate(
					req, venue::venueFind(u, searchParams, venues))});
			}

			return req.setYield({true});
		};

		func getCreateVenue = [](list args) -> gold::var {
			serveArgs(args, req, res);

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			if (sesh) {
				auto u = req.callMethod("getUser").getObject<user>();
				if (u)
					return res.end({getTemplate(
						req, venue::venueCreate(u, obj{}, obj{}))});
			}

			return req.setYield({true});
		};

		func putVenue = [](const list& pArgs) -> gold::var {
			using namespace HTML;
			serveArgs(pArgs, req, res);
			auto venueId = req.getParameter({1}).getString();

			func onDataCallback = [=](list args) -> gold::var {
				auto data = args[0].getString();
				auto req = args[2].getObject<request>();
				auto res = args[3].getObject<response>();
				auto u = req.callMethod("getUser").getObject<user>();
				if (req.isWWWFormURLEncoded()) {
					auto params = obj();
					obj::parseURLEncoded(data, params);

					auto venRet = venue::findOne({obj{{"_id", venueId}}});
					if (!venRet.isError()) {
						auto ven = venRet.getObject<venue>();
						ven.copy(params);
						auto saved = ven.save();
						if (!saved.isError())
							return res.end(
								{getTemplate(req, venueOptions(u, ven))});
						else
							return res.end(
								{getTemplate(req, errorPage({saved}))});
					} else
						return res.end({getTemplate(
							req, errorPage({"Venue not found."}))});
				} else
					return res.end({getTemplate(
						req,
						errorPage({"Could not parse encoded form."}))});
			};

			func onAbort = [](list args) -> gold::var {
				auto req = args[0].getObject<request>();
				auto res = args[1].getObject<response>();
				res.end({getTemplate(
					req, errorPage({genericError("Aborted.")}))});
				return gold::var();
			};

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			if (sesh) {
				auto u = req.callMethod("getUser").getObject<user>();
				if (u) {
					res.onAborted({onAbort, req});
					res.onData({onDataCallback, req});
					return gold::var();
				}
			}

			return req.setYield({true});
		};

		func postCreateVenue = [](list pArgs) -> gold::var {
			serveArgs(pArgs, req, res);

			func onDataCallback = [](list args) -> gold::var {
				auto data = args[0].getString();
				auto req = args[1].getObject<request>();
				auto res = args[2].getObject<response>();
				auto sesh =
					req.callMethod("getSession").getObject<session>();
				auto u = req.callMethod("getUser").getObject<user>();
				if (req.isWWWFormURLEncoded()) {
					auto params = obj();
					obj::parseURLEncoded(data, params);
					auto ven = venue(params);
					ven.setList("owners", list({u.getString("_id")}));
					auto resp = ven.save();
					// TODO: Do check, show errors
					if (!resp.isError()) {
						auto vId = ven.getString("_id");
						auto sId = sesh.getString("_id");
						auto url = "/venue/" + vId;
						res.writeHeader({"Location", url});
						res.writeStatus({"303 See Other"});
						res.end({getTemplate(
							req, venue::venueCreate(u, obj{}, obj{}))});
					} else
						res.end({getTemplate(req, errorPage({resp}))});
				} else
					return res.end({getTemplate(
						req,
						errorPage({"Could not parse encoded form."}))});

				return gold::var();
			};

			func onAbort = [](list args) -> gold::var {
				auto req = args[0].getObject<request>();
				auto res = args[1].getObject<response>();
				gold::list content =
					errorPage({genericError("Aborted")});
				res.end({getTemplate(req, content)});
				return gold::var();
			};

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			if (sesh) {
				auto u = req.callMethod("getUser").getObject<user>();
				if (u) {
					res.onAborted({onAbort, req});
					res.onData({onDataCallback, req});
					return gold::var();
				}
			}

			return req.setYield({true});
		};

		serv.get({"/venue/:venueID", getVenue});
		serv.get({"/find/venue", getFindVenues});
		serv.get({"/list/venue", getListVenues});
		serv.get({"/list/venue.json", getListVenues});
		serv.get({"/venue", getCreateVenue});

		serv.post({"/venue", postCreateVenue});
		serv.put({"/venue/:venueID", putVenue});
	}

	gold::var venue::findOne(list args) {
		auto col =
			venue::getPrototype().getObject<collection>("collection");
		auto value = col.findOne(args);
		return collection::setParentModel(
			value, venue::getPrototype());
	}

	gold::var venue::findMany(list args) {
		auto col =
			venue::getPrototype().getObject<collection>("collection");
		auto value = col.findMany(args);
		return collection::setParentModel(
			value, venue::getPrototype());
	}
}  // namespace gg