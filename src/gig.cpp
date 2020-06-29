#include "gig.hpp"

#include <file.hpp>
#include <filesystem>
#include <image.hpp>

#include "band.hpp"
#include "error.hpp"
#include "session.hpp"
#include "template.hpp"
#include "types.hpp"
#include "user.hpp"
#include "venue.hpp"

namespace gg {
	using div = gold::HTML::div;
	namespace fs = std::filesystem;

	struct tm parseDateTime(string& date, string& time) {
		auto utm = time_t{std::time(NULL)};
		struct tm tm = *localtime(&utm);

		auto y = stoi(date.substr(0, 4));
		auto ms = date[6] == '0' ? stoi(date.substr(6, 1))
														 : stoi(date.substr(5, 2));
		auto ds = date[9] == '0' ? stoi(date.substr(9, 1))
														 : stoi(date.substr(8, 2));

		auto h = stoi(time.substr(0, 2));
		auto m = stoi(time.substr(3, 2));

		tm.tm_year = y - 1900;
		tm.tm_mon = ms;
		tm.tm_mday = ds;
		tm.tm_hour = h;
		tm.tm_min = m;
		tm.tm_sec = 0;

		return tm;
	}

	std::map<string, string> gig::nameMap =
		std::map<string, string>({
			{"desc", "Description"},
			{"bands", "Bands"},
			{"venue", "Venue"},
			{"startTime", "Start Time"},
			{"endTime", "End Time"},
			{"hide", "Hide"},
			{"owners", "Owners"},
		});

	object& gig::getPrototype() {
		static auto proto = obj({
			{"desc", ""},
			{"bands", var()},
			{"venue", var()},
			{"startTime", 0},
			{"endTime", 0},
			{"hide", false},
			{"owners", gold::var(gold::list())},
			{"proto", model::getPrototype()},
		});
		return proto;
	}

	gig::gig() : model() {}

	gig::gig(object data) : model() {
		copy(data);
		setParent(gig::getPrototype());
		auto date = getString("date");
		auto start = getString("startTime");
		auto end = getString("endTime");
		if (date.length() > 0) {
			if (start.length() > 0) {
				auto startTime = parseDateTime(date, start);
				auto uST = mktime(&startTime);
				// cout << asctime(&startTime) << endl;
				setInt64("startTime", uST * 1000);
			}
			if (end.length() > 0) {
				auto endTime = parseDateTime(date, end);
				auto uET = mktime(&endTime);
				// cout << asctime(&endTime) << endl;
				setInt64("endTime", uET * 1000);
			}
			erase("date");
		}

		if (getType("bands") == typeString) {
			auto bs =
				file::parseJSON(getStringView("bands")).getList();
			setList("bands", bs);
		}
		erase("venueName");
		erase("bandNames");
	}

	var gig::addOwners(list args) {
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

	var gig::removeOwners(list args) {
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

	var gig::isOwner(list args) {
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

	var gig::getOwners(list args) {
		auto owners = getList("owners");
		auto returns = list({});
		auto criteria = obj{{"_id", owners}};
		returns += user::findMany({criteria}).getList();
		returns += venue::findMany({criteria}).getList();
		returns += band::findMany({criteria}).getList();
		return returns;
	}

	var gig::save(list args) {
		// TODO: Check if valid before saving
		// TODO: Convert band/venue object to ids
		auto saved = model::save(args);
		if (saved.isError()) return saved;
		return *this;
	}

	void gig::setRoutes(database db, server serv) {
		auto colRes = db.getCollection({"gig"});
		if (!colRes.isObject())
			cerr << "Could not get gig collection" << endl;
		getPrototype().setObject(
			"col", colRes.getObject<collection>());
		auto cacheControl = serv.getString("cacheControl");

		func getGig = [](list args) -> gold::var {
			serveArgs(args, req, res);

			auto id = req.getParameter({0}).getString();
			auto item =
				gig::findOne({obj({{"_id", id}})}).getObject<gig>();

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			auto u = req.callMethod("getUser").getObject<user>();
			if (req.acceptingJSON()) {
				return res.end({item});
			} else if (req.acceptingHTML()) {
				return res.end(
					{getTemplate(req, gig::gigIndex(sesh, u, item))});
			}
			return var();

			return req.setYield({true});
		};

		func getListGigs = [](list args) -> gold::var {
			serveArgs(args, req, res);

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			if (sesh) {
				auto u = req.callMethod("getUser").getObject<user>();
				if (u) {
					auto find = obj{
						{"owners", obj{{"$in", list({u.getID()})}}},
					};
					auto opt = obj({});
					auto resp = gig::findMany({find, opt});
					if (req.acceptingJSON()) {
						if (resp.isError()) {
							return res.end(
								{obj{{"error", (string)*resp.getError()}}});
						} else if (resp.isList()) {
							auto l = resp.getList();
							auto j = obj{
								{"total", l.size()},
								{"rows", l},
							};
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
								req, gig::gigList(sesh, u, {}, items))});
						}
					}
				}
			}

			return req.setYield({true});
		};

		func putGig = [](const list& pArgs) -> gold::var {
			using namespace HTML;
			serveArgs(pArgs, req, res);
			auto itemId = req.getParameter({1}).getString();

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

					auto itemRet = gig::findOne({obj{{"_id", itemId}}});
					if (!itemRet.isError()) {
						auto item = itemRet.getObject<gig>();
						item.copy(params);
						auto saved = item.save();
						if (!saved.isError())
							return res.end(
								{getTemplate(req, gigOptions(sesh, u, item))});
						else
							return res.end(
								{getTemplate(req, errorPage({saved}))});
					} else
						return res.end({getTemplate(
							req, errorPage({"Gig not found."}))});
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

		func getFindGigs = [](list args) -> gold::var {
			serveArgs(args, req, res);

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			auto u = user();
			if (sesh) u = req.callMethod("getUser").getObject<user>();
			auto searchParams = obj({});
			auto find = obj({});
			auto opt = obj({});
			auto found = gig::findMany({find, opt});
			if (found.isError()) {
				auto err = found.getError();
				return res.end({getTemplate(req, errorPage({*err}))});
			} else if (found.isList()) {
				auto items = found.getList();
				return res.end({getTemplate(
					req, gig::gigFind(sesh, u, searchParams, items))});
			}

			return req.setYield({true});
		};

		func getCreateGig = [](const list& args) -> gold::var {
			serveArgs(args, req, res);

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			if (sesh) {
				auto u = req.callMethod("getUser").getObject<user>();
				if (u)
					return res.end({getTemplate(
						req, gig::gigCreate(sesh, u, obj{}, obj{}))});
			}

			return req.setYield({true});
		};

		func postCreateGig = [](const list& pArgs) -> gold::var {
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
					auto item = gig(params);
					item.setList("owners", list({u.getString("_id")}));
					auto resp = item.save();
					// TODO: Do check, show errors
					if (!resp.isError()) {
						auto itemId = item.getString("_id");
						auto url = "/gig/" + itemId;
						sesh.writeSession(
							{obj{{"res", res}, {"redirect", url}}});
						res.end({getTemplate(
							req, gig::gigCreate(sesh, u, obj{}, obj{}))});
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

		serv.get({"/find/gig", getFindGigs});
		serv.get({"/list/gig", getListGigs});
		serv.get({"/list/gig.json", getListGigs});
		serv.get({"/gig/:gigID", getGig});
		serv.get({"/gig", getCreateGig});

		serv.post({"/gig", postCreateGig});
		serv.put({"/gig/:gigID", putGig});
	}

	var gig::findOne(list args) {
		auto col = gig::getPrototype().getObject<collection>("col");
		auto value = col.findOne(args);
		return collection::setParentModel(
			value, gig::getPrototype());
	}

	var gig::findMany(list args) {
		auto col = gig::getPrototype().getObject<collection>("col");
		auto value = col.findMany(args);
		return collection::setParentModel(
			value, gig::getPrototype());
	}

}  // namespace gg