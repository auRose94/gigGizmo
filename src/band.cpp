#include "band.hpp"

#include <iostream>

#include "error.hpp"
#include "template.hpp"
#include "user.hpp"

using namespace gold;

namespace gg {

	collection band::col;

	obj& band::getPrototype() {
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

	band::band() : model() {}

	band::band(obj data) : model(col, data) {
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

	void band::setRoutes(database db, server serv) {
		auto colRes = db.getCollection({"band"});
		if (!colRes.isObject())
			cerr << "Could not get band collection" << endl;
		col = colRes.getObject<collection>();

		func getBand = [](const list& args) -> gold::var {
			using namespace HTML;
			serveArgs(args, req, res);
			auto sessionID = req.getParameter({0}).getString();
			auto BandID = req.getParameter({1}).getString();
			if (BandID.size() == 0) BandID = sessionID;
			auto baRes = band::findOne({obj({{"_id", BandID}})});
			auto ba = baRes.getObject<band>();
			if (ba)  // Found
				return res.end({getTemplate(req, bandIndex(ba))});

			return req.setYield({true});
		};

		func getListBands = [](const list& args) -> gold::var {
			serveArgs(args, req, res);

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
					auto resp = findMany({find, opt});
					if (!resp.isError()) {
						auto bands = resp.getList();
						return res.end(
							{getTemplate(req, bandList(u, bands))});
					}
				}
			}

			return req.setYield({true});
		};

		func getCreateBand = [](const list& args) -> gold::var {
			serveArgs(args, req, res);

			auto seshId = req.getParameter({0}).getString();
			auto found = session::findOne({obj{{"_id", seshId}}});
			if (found.isObject(session::getPrototype())) {
				auto sesh = found.getObject<session>();
				auto uVar = sesh.getUser();
				auto u = uVar.getObject<user>();
				if (u)
					return res.end(
						{getTemplate(req, bandCreate(u, obj{}, obj{}))});
			}

			return req.setYield({true});
		};

		func putBand = [](const list& pArgs) -> gold::var {
			using namespace HTML;
			serveArgs(pArgs, req, res);
			session sesh;
			user u;

			func onDataCallback = [&](const list& args) -> gold::var {
				auto data = args[0].getString();
				auto req = args[1].getObject<request>();
				auto res = args[2].getObject<response>();

				if (req.isWWWFormURLEncoded()) {
					auto params = obj();
					obj::parseURLEncoded(data, params);

					auto bandId = req.getParameter({1}).getString();
					auto baRet = band::findOne({obj{{"_id", bandId}}});
					if (baRet.isError()) {
						return res.end(
							{getTemplate(req, errorPage({baRet}))});
					} else {
						auto ba = baRet.getObject<band>();
						ba.copy(params);
						// TODO: Do more checks
						auto saved = ba.save();
						if (!saved.isError())
							return res.end(
								{getTemplate(req, bandOptions(u, ba, obj{}))});
						else
							return res.end(
								{getTemplate(req, errorPage({saved}))});
					}
				}

				return res.end({getTemplate(
					req, errorPage({genericError(
								 "Failed to parse encoded form.")}))});
			};

			func onAbort = [&](const list& args) -> gold::var {
				auto req = args[0].getObject<request>();
				auto res = args[1].getObject<response>();
				return res.end({getTemplate(
					req, errorPage({genericError("Aborted.")}))});
			};

			auto seshId = req.getParameter({0}).getString();
			auto found = session::findOne({obj{{"_id", seshId}}});
			if (found.isObject(session::getPrototype())) {
				sesh = found.getObject<session>();
				auto uVar = sesh.getUser();
				u = uVar.getObject<user>();
				if (u) {
					res.onAborted({onAbort, req});
					return res.onData({onDataCallback, req});
				}
			}

			return req.setYield({true});
		};

		func postCreateBand = [](const list& pArgs) -> gold::var {
			serveArgs(pArgs, req, res);
			session sesh;
			user u;

			func onDataCallback = [&](const list& args) -> gold::var {
				auto data = args[0].getString();
				auto req = args[1].getObject<request>();
				auto res = args[2].getObject<response>();

				if (req.isWWWFormURLEncoded()) {
					auto params = obj();
					obj::parseURLEncoded(data, params);
					auto BandId = req.getParameter({1}).getString();
					auto ban = band(params);
					ban.setList("owners", list({u.getString("_id")}));
					auto resp = ban.save();
					// TODO: Do check, show errors
					if (resp.isError())
						return res.end(
							{getTemplate(req, errorPage({resp}))});
					else {
						auto bId = ban.getString("_id");
						auto sId = sesh.getString("_id");
						auto url = "/band/" + bId;
						res.writeHeader({"Location", url});
						res.writeStatus({"303 See Other"});
						return res.end({getTemplate(
							req, band::bandCreate(u, obj{}, obj{}))});
					}
				}

				return res.end({getTemplate(
					req,
					errorPage({genericError(
						"Expected URL Form encoded content.")}))});
			};

			func onAbort = [=](const list& args) -> gold::var {
				auto req = args[0].getObject<request>();
				auto res = args[1].getObject<response>();
				return res.end({getTemplate(
					req, errorPage({genericError("Aborted.")}))});
			};

			auto seshId = req.getParameter({0}).getString();
			auto found = session::findOne({obj{{"_id", seshId}}});
			if (found.isObject(session::getPrototype())) {
				sesh = found.getObject<session>();
				u = sesh.getUser().getObject<user>();
				if (u) {
					res.onAborted({onAbort, req});
					return res.onData({onDataCallback, req});
				}
			}

			return req.setYield({true});
		};

		serv.get({"/band/:bandID", getBand});
		serv.get({"/list/band", getListBands});
		serv.get({"/band", getCreateBand});

		serv.post({"/band", postCreateBand});
		serv.put({"/band/:bandID", putBand});
	}

	gold::var band::findOne(list args) {
		auto value = band::col.findOne(args);
		return collection::setParentModel(
			value, band::getPrototype());
	}

	gold::var band::findMany(list args) {
		auto value = band::col.findMany(args);
		return collection::setParentModel(
			value, band::getPrototype());
	}

}  // namespace gg