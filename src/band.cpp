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

	band::band() : model(col) { setParent(getPrototype()); }

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
			auto content = gold::list();
			if (ba)  // Found
				content = bandIndex(ba);
			else  // Not
				content = errorPage({genericError("band not found.")});

			auto resp = getTemplate(req, content);

			auto d = (string)(resp);
			res.writeHeader({"Content-Type", "text/html"});
			return res.end({d});
		};

		func getListBands = [](const list& args) -> gold::var {
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
					auto resp = findMany({find, opt});
					if (resp.isError()) {
						auto err = resp.getError();
						return res.end({err->what()});
					}
					auto bands = resp.getList();
					content = bandList(u, bands);
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

		func getCreateBand = [](const list& args) -> gold::var {
			serveArgs(args, req, res);

			auto content = gold::list();

			auto seshId = req.getParameter({0}).getString();
			auto found = session::findOne({obj{{"_id", seshId}}});
			if (found.isObject(session::getPrototype())) {
				auto sesh = found.getObject<session>();
				auto uVar = sesh.getUser();
				auto u = uVar.getObject<user>();
				if (u)
					content = bandCreate(u, obj{}, obj{});
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

		func putBand = [](const list& pArgs) -> gold::var {
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
						auto bandId = req.getParameter({1}).getString();
						auto found =
							session::findOne({obj{{"_id", seshId}}});
						if (found.isObject(session::getPrototype())) {
							auto sesh = found.getObject<session>();
							auto uVar = sesh.getUser();
							auto u = uVar.getObject<user>();
							if (u) {
								auto baRet =
									band::findOne({obj{{"_id", bandId}}});
								if (baRet.isError()) {
									d = getTemplate(req, errorPage({baRet}));
								} else {
									auto ba = baRet.getObject<band>();
									ba.copy(params);
									// TODO: Do more checks
									auto saved = ba.save();
									if (!saved.isError())
										d = getTemplate(
											req, bandOptions(u, ba, obj{}));
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

		func postCreateBand = [](const list& pArgs) -> gold::var {
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
						auto BandId = req.getParameter({1}).getString();
						auto found =
							session::findOne({obj{{"_id", seshId}}});
						if (found.isObject(session::getPrototype())) {
							auto sesh = found.getObject<session>();
							auto uVar = sesh.getUser();
							auto u = uVar.getObject<user>();
							if (u) {
								auto baRes =
									band::findOne({obj({{"_id", BandId}})});
								if (baRes.isError())
									d = getTemplate(req, errorPage({baRes}));
								else if (baRes.isObject()) {
									auto ba = baRes.getObject<band>();
									ba.copy(params);
									auto resp = ba.save();
									// TODO: Do check, show errors
									if (resp.isError())
										d = getTemplate(req, errorPage({resp}));
									else
										d = getTemplate(
											req, bandCreate(u, ba, obj{}));
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

		serv.get({R"(/band/:bandID)", getBand});
		serv.get({R"(/:sessionID/band/:bandID)", getBand});
		serv.get({R"(/:sessionID/list/band)", getListBands});
		serv.get({R"(/:sessionID/band)", getCreateBand});

		serv.post({R"(/:sessionID/band)", postCreateBand});
		serv.put({R"(/:sessionID/band/:bandID)", putBand});
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