#include "upload.hpp"

#include <file.hpp>
#include <filesystem>
#include <image.hpp>

#include "band.hpp"
#include "session.hpp"
#include "user.hpp"
#include "venue.hpp"

namespace gg {
	namespace fs = std::filesystem;
	object& upload::getPrototype() {
		static auto proto = obj({
			{"verified", false},
			{"nsfw", false},
			{"hide", false},
			{"src", ""},
			{"width", 0},
			{"height", 0},
			{"size", 0},
			{"hash", ""},
			{"desc", ""},
			{"attr", ""},
			{"mime", ""},
			{"license", ""},
			{"owners", gold::var(gold::list())},
			{"proto", model::getPrototype()},
		});
		return proto;
	}

	collection upload::col;

	upload::upload() : model() {}

	void upload::createUploadFolder() {
		auto upPath = "./upload";
		auto exists = fs::is_directory(upPath);
		if (!exists) {
			auto created = fs::create_directory(upPath);
			if (!created)
				cerr << "Failed to create upload directory. (" << upPath
						 << ")" << endl;
		}
	}

	bool upload::createUpload(string p, string_view data) {
		auto f = file::saveFile(p, data);
		if (f.isError()) {
			cerr << *f.getError() << endl;
			return false;
		}
		if (f.isObject()) return true;
		return false;
	}

	upload::upload(object data) : model() {
		copy(data);
		setParent(upload::getPrototype());
		createUploadFolder();
		auto src = getStringView("src");
		if (src.find("data:") != string::npos) {
			auto mime = string();
			auto bin = file::decodeDataURL(src, mime);
			auto strData = string_view((char*)bin.data(), bin.size());
			auto img = image({{"data", strData}});
			auto h = std::hash<string_view>();
			auto hash = to_string((uint64_t)h(strData));
			auto path = "./upload/" + hash + ".bin";
			auto created = createUpload(path, strData);
			if (created) {
				setString("src", path);
			}
			setUInt32("width", img.getUInt32("width"));
			setUInt32("height", img.getUInt32("height"));
			setUInt64("size", bin.size());
			setString("hash", hash);
			setString("mime", mime);
		}
	}

	var upload::addOwners(list args) {
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

	var upload::removeOwners(list args) {
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

	var upload::isOwner(list args) {
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

	var upload::getOwners(list args) {
		auto owners = getList("owners");
		auto returns = list({});
		auto criteria = obj{{"_id", owners}};
		returns += user::findMany({criteria}).getList();
		returns += venue::findMany({criteria}).getList();
		returns += band::findMany({criteria}).getList();
		return returns;
	}

	void upload::setRoutes(database db, server serv) {
		auto colRes = db.getCollection({"upload"});
		if (!colRes.isObject())
			cerr << "Could not get upload collection" << endl;
		getPrototype().setObject(
			"col", colRes.getObject<collection>());
		auto cacheControl = serv.getString("cacheControl");

		func getUpload = [](list args) -> gold::var {
			serveArgs(args, req, res);

			auto id = req.getParameter({0}).getString();
			auto item = upload::findOne({obj({{"_id", id}})})
										.getObject<upload>();
			auto hide = item.getBool("hide");
			auto nsfw = item.getBool("nsfw");

			auto sesh =
				req.callMethod("getSession").getObject<session>();
			if (sesh) {
				auto u = req.callMethod("getUser").getObject<user>();
				if (u) {
					if (hide && item.isOwner({u})) {
						return res.end({item});
					} else if (!hide) {
						return res.end({item});
					}
				}
			}
			if (!hide && !nsfw) return res.end({item});

			return req.setYield({true});
		};

		func getUploadImage =
			[cacheControl](list args) -> gold::var {
			serveArgs(args, req, res);

			auto id = req.getParameter({0}).getString();
			auto item = upload::findOne({obj({{"_id", id}})})
										.getObject<upload>();
			if (item) {
				auto dataURL = item.getString("src");
				auto mime = string();
				binary bin;
				if (dataURL.find("data:") != string::npos) {
					bin = file::decodeDataURL(dataURL, mime);
				}
				if (dataURL.find("./upload/") != string::npos) {
					mime = item.getString("mime");
					auto f = file::readFile(dataURL);
					if (f.isObject()) {
						auto fo = f.getObject<file>();
						bin = fo.getBinary("data");
					}
				}
				auto hide = item.getBool("hide");
				auto nsfw = item.getBool("nsfw");
				auto hash = item.getString("hash");
				auto chash = req.getHeader({"if-none-match"});

				if (hash.size() > 0 && hash.compare(chash) == 0) {
					res.writeStatus({"304 Not Changed"});
					res.writeHeader({"Cache-Control", cacheControl});
					res.end();
				} else {
					auto sesh =
						req.callMethod("getSession").getObject<session>();
					if (sesh) {
						auto u =
							req.callMethod("getUser").getObject<user>();
						if (u) {
							if (hide && item.isOwner({u})) {
								res.writeHeader({"Content-Type", mime});
								res.writeHeader(
									{"Cache-Control", cacheControl});
								res.writeHeader({"ETag", hash});
								return res.end({bin});
							} else if (!hide) {
								res.writeHeader({"Content-Type", mime});
								res.writeHeader(
									{"Cache-Control", cacheControl});
								res.writeHeader({"ETag", hash});
								return res.end({bin});
							}
						}
					}
					if (!hide && !nsfw) {
						res.writeHeader({"Content-Type", mime});
						res.writeHeader({"Cache-Control", cacheControl});
						res.writeHeader({"ETag", hash});
						return res.end({bin});
					}
				}
			}

			return req.setYield({true});
		};

		serv.get({"/upload/:uploadID/image", getUploadImage});
		serv.get({"/upload/:uploadID", getUpload});
	}

	var upload::findOne(list args) {
		auto col =
			upload::getPrototype().getObject<collection>("col");
		auto value = col.findOne(args);
		return collection::setParentModel(
			value, upload::getPrototype());
	}

	var upload::findMany(list args) {
		auto col =
			upload::getPrototype().getObject<collection>("col");
		auto value = col.findMany(args);
		return collection::setParentModel(
			value, upload::getPrototype());
	}
}  // namespace gg