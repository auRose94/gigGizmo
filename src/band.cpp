#include "band.hpp"

#include <iostream>

#include "error.hpp"
#include "template.hpp"
#include "upload.hpp"
#include "user.hpp"

using namespace gold;

namespace gg {

	std::map<string, string> band::nameMap =
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

	obj& band::getPrototype() {
		static auto proto = obj(initList{
			{"name", ""},
			{"contact", ""},
			{"shortDesc", ""},
			{"longDesc", ""},
			{"accent", ""},
			{"icon", ""},
			{"location", obj()},
			{"tags", list()},
			{"uploads", gold::var(gold::list())},
			{"owners", gold::var(gold::list())},
			{"hide", false},
			{"proto", model::getPrototype()},
		});
		return proto;
	}

	band::band() : model() {}

	band::band(obj data)
		: model(
				getPrototype().getObject<collection>("collection"),
				data) {
		setParent(getPrototype());
	}

	gold::var band::addOwners(list args) {
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

	gold::var band::removeOwners(list args) {
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

	gold::var band::isOwner(list args) {
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

	gold::var band::getOwners(list args) {
		auto owners = getList("owners");
		return user::findMany({obj(initList{
			{"_id", owners},
		})});
	}

	var band::save(list args) {
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

	void band::setRoutes(database db, server serv) {
		auto colRes = db.getCollection({"band"});
		if (!colRes.isObject())
			cerr << "Could not get band collection" << endl;
		getPrototype().setObject(
			"col", colRes.getObject<collection>());

		func getBand = [](const list& args) -> gold::var {
			using namespace HTML;
			serveArgs(args, req, res);
			auto sesh =
				req.callMethod("getSession").getObject<session>();
			auto u = req.callMethod("getUser").getObject<user>();
			auto itemID = req.getParameter({0}).getString();
			auto itemRes = band::findOne({obj({{"_id", itemID}})});
			auto item = itemRes.getObject<band>();
			if (item) {  // Found
				return res.end(
					{getTemplate(req, band::bandIndex(sesh, u, item))});
			}

			return req.setYield({true});
		};

		func getListBands = [](const list& args) -> gold::var {
			serveArgs(args, req, res);

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			if (sesh) {
				auto u = req.callMethod("getUser").getObject<user>();
				if (u) {
					auto find =
						obj{{"owners", obj{{"$in", list({u.getID()})}}}};
					auto opt = obj({});
					auto resp = band::findMany({find, opt});
					if (req.acceptingJSON()) {
						res.writeHeader(
							{"Content-Type", "application/json"});
						if (resp.isError()) {
							auto e = (string)*resp.getError();
							auto j = "{\"error\":" + e + "}";
							return res.end({j});
						} else if (resp.isList()) {
							auto l = resp.getList();
							auto j =
								obj{{"total", l.size()}, {"rows", l}}.getJSON();
							return res.end({j});
						}
					} else if (req.acceptingHTML()) {
						if (resp.isError()) {
							auto err = resp.getError();
							return res.end(
								{getTemplate(req, errorPage({*err}))});
						} else {
							auto items = resp.getList();
							return res.end({getTemplate(
								req, band::bandList(sesh, u, {}, items))});
						}
					}
				}
			}

			return req.setYield({true});
		};

		func getFindBands = [](list args) -> gold::var {
			serveArgs(args, req, res);

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			auto u = user();
			if (sesh) u = req.callMethod("getUser").getObject<user>();
			auto query = object();
			parseURLEncoded(req.getQuery(), query);
			query.erase("s");
			//TODO: More checks on permission and search validation
			auto opt = obj({});
			auto found = band::findMany({query, opt});
			if (req.acceptingJSON()) {
				if (found.isError()) {
					return res.end(
						{obj{{"error", (string)*found.getError()}}});
				} else if (found.isList()) {
					return res.end({found});
				}
			} else if (req.acceptingHTML()) {
				if (found.isError()) {
					auto err = found.getError();
					return res.end({getTemplate(req, errorPage({*err}))});
				} else if (found.isList()) {
					auto items = found.getList();
					return res.end({getTemplate(
						req, band::bandFind(sesh, u, query, items))});
				}
			}

			return req.setYield({true});
		};

		func getCreateBand = [](const list& args) -> gold::var {
			serveArgs(args, req, res);

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			if (sesh) {
				auto u = req.callMethod("getUser").getObject<user>();
				if (u)
					return res.end({getTemplate(
						req, band::bandCreate(sesh, u, obj{}, obj{}))});
			}

			return req.setYield({true});
		};

		func putBand = [](const list& pArgs) -> gold::var {
			using namespace HTML;
			serveArgs(pArgs, req, res);
			auto itemID = req.getParameter({1}).getString();

			func onDataCallback = [=](list args) -> gold::var {
				auto data = args[0].getString();
				auto req = args[2].getObject<request>();
				auto res = args[3].getObject<response>();
				auto sesh =
					req.callMethod("getSession").getObject<session>();
				auto u = req.callMethod("getUser").getObject<user>();
				if (req.isWWWFormURLEncoded()) {
					auto params = obj();
					obj::parseURLEncoded(data, params);

					auto itemRet = band::findOne({obj{{"_id", itemID}}});
					if (!itemRet.isError()) {
						auto item = itemRet.getObject<band>();
						item.copy(params);
						auto saved = item.save();
						if (!saved.isError())
							return res.end(
								{getTemplate(req, bandOptions(sesh, u, item))});
						else
							return res.end(
								{getTemplate(req, errorPage({saved}))});
					} else
						return res.end({getTemplate(
							req, errorPage({"Band not found."}))});
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

		func postCreateBand = [](const list& pArgs) -> gold::var {
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
					auto item = band(params);
					item.setList("owners", list({u.getString("_id")}));
					auto resp = item.save();
					// TODO: Do check, show errors
					if (!resp.isError()) {
						auto itemId = item.getString("_id");
						auto url = "/band/" + itemId;
						sesh.writeSession(
							{obj{{"res", res}, {"redirect", url}}});
						res.end({getTemplate(
							req, band::bandCreate(sesh, u, obj{}, obj{}))});
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

		serv.get({"/band/:bandID", getBand});
		serv.get({"/find/band", getFindBands});
		serv.get({"/find/band.json", getFindBands});
		serv.get({"/list/band", getListBands});
		serv.get({"/list/band.json", getListBands});
		serv.get({"/band", getCreateBand});

		serv.post({"/band", postCreateBand});
		serv.put({"/band/:bandID", putBand});
	}

	gold::var band::findOne(list args) {
		auto col =
			band::getPrototype().getObject<collection>("col");
		auto value = col.findOne(args);
		return collection::setParentModel(
			value, band::getPrototype());
	}

	gold::var band::findMany(list args) {
		auto col =
			band::getPrototype().getObject<collection>("col");
		auto value = col.findMany(args);
		return collection::setParentModel(
			value, band::getPrototype());
	}

}  // namespace gg