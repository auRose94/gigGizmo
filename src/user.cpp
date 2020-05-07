#include "user.hpp"

#include <cryptopp/cryptlib.h>
#include <cryptopp/hex.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/sha.h>

#include <cstddef>
#include <iostream>
#include <locale>
#include <random>
#include <regex>

#include "bootstrap.hpp"
#include "error.hpp"
#include "session.hpp"
#include "template.hpp"
#include "user.hpp"
#include "utility.hpp"

namespace gg {

	using namespace CryptoPP;
	collection user::col;
	static mutex cryptoMutex;

	obj& user::getPrototype() {
		static auto proto = obj({
			{"email", ""},
			{"hash", ""},
			{"salt", ""},
			{"firstName", ""},
			{"midName", ""},
			{"lastName", ""},
			{"company", ""},
			{"birthday", ""},
			{"gender", ""},
			{"phone", ""},
			{"address1", ""},
			{"address2", ""},
			{"country", ""},
			{"city", ""},
			{"zone", ""},
			{"zip", ""},
			{"icon", ""},
			{"proto", gold::model::getPrototype()},
		});
		return proto;
	}

	void user::setRoutes(database db, server serv) {
		auto colRes = db.getCollection({"user"});
		if (!colRes.isObject()) cerr << colRes << endl;
		auto def = collection();
		col = colRes.getObject<collection>();

		func getUserHome = [](const list& args) -> gold::var {
			serveArgs(args, req, res);
			auto seshId = req.getParameter({0}).getString();
			auto found = session::findOne({obj{{"_id", seshId}}});
			auto d = html();
			if (found.isObject(session::getPrototype())) {
				auto sesh = found.getObject<session>();
				auto uVar = sesh.getUser();
				auto u = uVar.getObject<user>();
				auto url = string("/") + seshId + "/home";
				d = getTemplateEx(u, sesh, url, user::userHome(sesh));
			} else {
				d = getTemplate(req, errorPage({found}));
			}

			res.writeHeader({"Content-Type", "text/html"});
			return res.end({string(d)});
		};

		func getLogin = [](const list& args) -> gold::var {
			serveArgs(args, req, res);
			auto content = user::userLogin();
			auto resp = getTemplate(req, content);

			auto d = (string)(resp);
			res.writeHeader({"Content-Type", "text/html"});
			return res.end({d});
		};

		func postLogin = [](const list& args) -> gold::var {
			serveArgs(args, req, res);
			std::string* buffer = new string();

			func onDataCallback = [=](const list& dargs) -> gold::var {
				serveArgs(dargs, req, res);
				auto chuck = dargs[0].getString();
				auto isEnd = dargs[1].getBool();
				(*buffer) = (*buffer) + chuck;
				if (isEnd) {
					string userEmail;
					string userPassword;

					auto params = obj();
					obj::parseURLEncoded(*buffer, params);
					userEmail = params.getString("email");
					userPassword = params.getString("password");
					auto acceptHeader = req.getHeader({"accept"});
					auto allHeaders = req.getAllHeaders();
					auto acceptValue = !acceptHeader.isEmpty()
															 ? acceptHeader.getString()
															 : string("application/json");
					if (acceptValue.find("text/html") != string::npos) {
						auto content = gold::list();
						auto error = string("");
						if (user::invalidEmail(userEmail, error))
							content = user::userLogin("", error);
						else if (user::invalidPassword(userPassword, error))
							content = user::userLogin("", "", error);
						else {
							auto loggedIn =
								user::login(userEmail, userPassword);
							if (loggedIn.isError())
								error = string(*loggedIn.getError());
							else if (loggedIn.isList()) {
								auto ret = loggedIn.getList();
								auto sesh = ret[1].getObject<session>();
								auto path =
									string("/") + sesh.getString("_id") + "/home";
								res.writeStatus({"303 See Other"});
								res.writeHeader({"Location", path});
								delete buffer;
								res.writeHeader({"Content-Type", "text/html"});
								return res.end({""});
							} else {
								error = "Invalid credentials.";
							}
							content = user::userLogin(error);
						}

						auto resp = getTemplate(req, content);

						res.writeHeader({"Content-Type", "text/html"});
						res.end({string(resp)});
					} else if (
						acceptValue.find("application/json") !=
						string::npos) {
						cout << "JSON";
					} else {
						cout << acceptValue << endl;
					}
					delete buffer;
				}
				return gold::var();
			};

			func onAbort = [](const list&) -> gold::var {
				return gold::var();
			};

			res.onAborted({onAbort});
			res.onData({onDataCallback});

			return gold::var();
		};

		func getRegister = [](const list& args) -> gold::var {
			serveArgs(args, req, res);
			auto errors = obj();
			auto resp =
				getTemplate(req, user::userRegister(obj(), errors));

			auto d = (string)(resp);
			res.writeHeader({"Content-Type", "text/html"});
			return res.end({d});
		};

		func postRegister = [](const list& pArgs) -> gold::var {
			serveArgs(pArgs, req, res);
			std::string* buffer = new string();

			func onDataCallback = [&](const list& args) -> gold::var {
				auto req = pArgs[0].getObject<request>();
				auto res = pArgs[1].getObject<response>();
				auto chuck = args[0].getString();
				auto isEnd = args[1].getBool();
				(*buffer) = (*buffer) + chuck;
				if (isEnd) {
					auto d = html();
					auto t = "text/html";
					if (req.isWWWFormURLEncoded()) {
						auto params = obj();
						obj::parseURLEncoded(*buffer, params);
						auto u = user::create(params);
						if (u.isObject(user::getPrototype())) {
							// Created
							auto userO = u.getObject<user>();
							auto sesh = session(userO);
							auto succ = sesh.save();
							if (succ.isError()) {
								// TODO: Create error screen
							} else {
								auto url =
									string("/") + sesh.getString("_id") + "/home";
								res.writeHeader({"Location", url});
								return res.writeStatus({"303 See Other"});
							}
						} else if (u.isObject()) {
							// Form issue
							auto errors = u.getObject();
							d = getTemplate(
								req, user::userRegister(params, errors));
						} else if (u.isError()) {
							d = getTemplate(req, errorPage({u}));
						}
					}

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

		func getOptions = [](const list& args) -> gold::var {
			serveArgs(args, req, res);
			auto errors = obj();
			auto seshId = req.getParameter({0}).getString();
			auto found = session::findOne({obj{{"_id", seshId}}});
			auto d = string();
			if (found.isObject(session::getPrototype())) {
				auto sesh = found.getObject<session>();
				auto uVar = sesh.getUser();
				auto u = uVar.getObject<user>();
				if (u) {
					d = (string)(
						getTemplate(req, user::userOptions(u, errors)));
				} else {
					d = (string)(getTemplate(
						req,
						errorPage({genericError(
							"Could not find user in session.")})));
				}
			}

			res.writeHeader({"Content-Type", "text/html"});
			return res.end({d});
		};

		func postOptions = [](const list& pArgs) -> gold::var {
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
						auto found =
							session::findOne({obj{{"_id", seshId}}});
						if (found.isObject(session::getPrototype())) {
							auto sesh = found.getObject<session>();
							auto uVar = sesh.getUser();
							auto u = uVar.getObject<user>();
							if (u) {
								auto pass = params.getString("password");
								auto confPass =
									params.getString("confPassword");
								auto email = params.getString("email");
								auto confEmail = params.getString("confEmail");
								if (email != "" && confEmail == email)
									u.changeEmail({email});
								if (pass != "" && confPass == pass)
									u.changePassword({pass});
								params.erase("confEmail");
								params.erase("password");
								params.erase("confPassword");
								u.copy(params);
								auto resp = u.save();
								if (resp.isError())
									d = getTemplate(req, errorPage({resp}));
								else
									d = getTemplate(
										req, user::userOptions(u, obj()));
							} else
								d = getTemplate(
									req,
									errorPage({genericError(
										"Could not get user from session.")}));
						} else
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

		// Session
		serv.get({"/:sessionID/options", getOptions});
		serv.post({"/:sessionID/options", postOptions});
		serv.get({"/:sessionID/register", getRegister});
		serv.post({"/:sessionID/register", postRegister});
		serv.get({"/:sessionID/login", getLogin});
		serv.post({"/:sessionID/login", postLogin});
		serv.get({"/:sessionID/home", getUserHome});

		// Without
		serv.get({"/register", getRegister});
		serv.post({"/register", postRegister});
		serv.get({"/login", getLogin});
		serv.post({"/login", postLogin});
	}

	user::user() : model(col) { setParent(getPrototype()); }

	user::user(obj data) : model(col, data) {
		setParent(getPrototype());
	}

	gold::var user::checkPassword(list args) {
		auto p = args[0].getString();
		auto s = getString("salt");
		auto ch = getString("hash");
		auto rph = generateHash(p, s);
		if (rph.isError()) return rph;
		auto ph = rph.getString();
		return gold::var(ch == ph);
	}

	gold::var user::changePassword(list args) {
		auto pass = args[0].getString();
		auto nSalt = randomString(16);
		auto nHash = generateHash(pass, nSalt);
		if (nHash.isError()) return nHash;
		setString("hash", nHash.getString());
		setString("salt", nSalt);
		return gold::var();
	}

	gold::var user::changeEmail(list args) {
		auto email = args[0].getString();
		auto conf = randomString(16);
		setString("emailChangeConf", conf);
		setString("emailChangeValue", email);
		// TODO: Send email confirmation two both addresses; new (to
		// confirm) and old (to verify address)
		return gold::var();
	}

	gold::var user::getIcon(list) {
		auto id = getString("icon");
		return gold::var();
	}

	gold::var user::generateHash(string value, string salt) {
		try {
			unique_lock<mutex> gaurd(cryptoMutex);
			using byte = unsigned char;
			byte derived[SHA256::DIGESTSIZE];

			int unused = 0;

			static PKCS5_PBKDF2_HMAC<SHA256> pbkdf;
			pbkdf.DeriveKey(
				(byte*)derived,
				sizeof(derived),
				unused,
				(byte*)value.data(),
				value.size(),
				(byte*)salt.data(),
				salt.size(),
				1024,
				0.0f);

			std::string result;
			auto sink = new StringSink(result);
			auto encoder = new HexEncoder(sink);

			encoder->Put(derived, sizeof(derived));
			encoder->MessageEnd();

			delete encoder;
			return result;
		} catch (exception& e) {
			return genericError(e);
		}
	}

	gold::var user::create(obj data) {
		auto cemail = data.getString("confEmail");
		auto cp = data.getString("confPassword");
		auto email = data.getString("email");
		auto p = data.getString("password");
		auto fn = data.getString("firstName");
		auto mn = data.getString("midName");
		auto ln = data.getString("lastName");
		auto com = data.getString("company");
		auto ph = data.getString("phone");
		auto ge = data.getString("gender");
		auto bi = data.getString("birthday");
		auto a1 = data.getString("address1");
		auto a2 = data.getString("address2");
		auto con = data.getString("country");
		auto city = data.getString("city");
		auto zone = data.getString("zone");
		auto zip = data.getString("zip");
		auto icon = data.getString("icon");
		auto ut = data.getString("userType");

		string e;
		auto err = obj({});
		if (email != cemail)
			err.setString(
				"confEmail", "Confirm email doesn't match.");
		if (p != cp)
			err.setString(
				"confPassword", "Confirm password doesn't match.");
		if (invalidEmail(email, e)) err.setString("email", e);
		if (invalidPassword(p, e)) err.setString("password", e);
		if (invalidFirstName(fn, e)) err.setString("firstName", e);
		if (invalidMidName(mn, e)) err.setString("midName", e);
		if (invalidLastName(ln, e)) err.setString("lastName", e);
		if (invalidCompany(com, e)) err.setString("company", e);
		if (invalidPhone(ph, e)) err.setString("phone", e);
		if (invalidGender(ge, e)) err.setString("gender", e);
		if (invalidBirthday(bi, e)) err.setString("birthday", e);
		if (invalidAddress1(a1, e)) err.setString("address1", e);
		if (invalidAddress2(a2, e)) err.setString("address2", e);
		if (invalidCountry(con, e)) err.setString("country", e);
		if (invalidCity(city, e)) err.setString("city", e);
		if (invalidZone(zone, e)) err.setString("zone", e);
		if (invalidZip(zip, e)) err.setString("zip", e);
		if (invalidIcon(icon, e)) err.setString("icon", e);
		if (invalidUserType(ut, e)) err.setString("userType", e);

		if (err.size() > 0) return err;

		auto existU = user::col.findOne({obj{{"email", email}}});
		if (existU.isError())
			return existU;
		else if (existU.isObject())
			return genericError("User with email exists.");

		auto u = user(data);
		auto salt = randomString(16);
		auto hash = generateHash(p, salt);
		u.setString("salt", salt);
		u.setString("hash", hash);
		// We have to remove sensitive data
		u.erase("password");
		u.erase("confPassword");
		u.erase("confEmail");
		return u.save();
	}

	gold::var user::login(string email, string password) {
		auto exist = user::findOne({obj{{"email", email}}});
		if (exist.isError())
			return exist;
		else if (exist.isObject()) {
			auto u = exist.getObject<user>();
			auto valid = u.checkPassword({password});
			if (valid.isError())
				return valid;
			else if (valid.getBool()) {
				auto sesh = session(u);
				auto saved = sesh.save();
				if (saved.isError()) return saved;
				return gold::var(gold::list{u, sesh});
			}
		}
		return gold::var();
	}

	string user::transformBirthday(string value) {
		if (value.size() != 8) {
			return value;
		}
		auto year = value.substr(0, 4);
		auto month = value.substr(4, 2);
		auto day = value.substr(6, 2);
		return year + "-" + month + "-" + day;
	}

	bool user::invalidEmail(string email, string& error) {
		error = "";
		if (email.find("@") == string::npos) {
			error = "Missing @ symbol from address.";
			return true;
		}
		if (email.find(".") == string::npos) {
			error = "Missing . symbol from address.";
			return true;
		}
		return false;
	}

	bool user::invalidPassword(string password, string& error) {
		error = "";
		if (password.size() < 6) {
			error = "Password needs to be larger than 6.";
			return true;
		}
		bool symbol = false;
		bool number = false;
		for (auto it = password.begin(); it != password.end();
				 ++it) {
			if (isdigit(*it))
				number = true;
			else if (!isalnum(*it))
				symbol = true;
			if (number && symbol) break;
		}
		if (!(number && symbol)) {
			if (!number && !symbol)
				error = "Password missing number & symbol.";
			else if (!number)
				error = "Password missing number.";
			else if (!symbol)
				error = "Password missing symbol.";
			return true;
		}
		return false;
	}

	bool user::invalidFirstName(string fName, string& error) {
		error = "";
		auto s = fName.size();
		if (s == 0) {
			error = "First name is empty.";
			return true;
		} else if (s > 255) {
			error = "First name is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidMidName(string mName, string& error) {
		error = "";
		auto s = mName.size();
		if (s > 255) {
			error = "Middle name is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidLastName(string lName, string& error) {
		error = "";
		auto s = lName.size();
		if (s == 0) {
			error = "Last name is empty.";
			return true;
		} else if (s > 255) {
			error = "Last name is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidCompany(string com, string& error) {
		error = "";
		auto s = com.size();
		if (s > 255) {
			error = "Company is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidPhone(string b, string& error) {
		error = "";
		std::regex e("\\+[0-9]{8,}");
		if (!regex_match(b, e)) {
			error = "Invalid phone number.";
			return true;
		}
		return false;
	}

	bool user::invalidGender(string g, string& error) {
		error = "";
		if (g.size() == 0) return false;
		auto fc = tolower(g[0]);
		if (!(fc == 'n' || fc == 'f' || fc == 'm')) {
			error =
				"Expected gender; (n)on-binary, (f)emale, (m)ale, or "
				"blank";
			return true;
		}
		return false;
	}

	bool user::invalidBirthday(string b, string& error) {
		error = "";
		auto s = b.size();
		if (s == 0) {
			error = "Birthday is empty.";
			return true;
		} else if (s > 16) {
			error = "Birthday is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidAddress1(string a1, string& error) {
		error = "";
		auto s = a1.size();
		if (s == 0) {
			error = "Address 1 is empty.";
			return true;
		} else if (s > 255) {
			error = "Address 1 is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidAddress2(string a2, string& error) {
		error = "";
		auto s = a2.size();
		if (s > 255) {
			error = "Address 2 is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidCountry(string c, string& error) {
		error = "";
		auto s = c.size();
		if (s == 0) {
			error = "Country is empty.";
			return true;
		} else if (s > 128) {
			error = "Country is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidCity(string c, string& error) {
		error = "";
		auto s = c.size();
		if (s == 0) {
			error = "City is empty.";
			return true;
		} else if (s > 128) {
			error = "City is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidZone(string z, string& error) {
		error = "";
		auto s = z.size();
		if (s == 0) {
			error = "State/Prov/Zone is empty.";
			return true;
		} else if (s > 128) {
			error = "State/Prov/Zone is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidZip(string z, string& error) {
		error = "";
		auto s = z.size();
		if (s == 0) {
			error = "ZIP is empty.";
			return true;
		} else if (s > 32) {
			error = "ZIP is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidIcon(string id, string& error) {
		error = "";
		auto s = id.size();
		if (s > 32) {
			error = "Icon id is too big.";
			return true;
		}
		return false;
	}

	bool user::invalidUserType(string t, string& error) {
		error = "";
		if (t == "patron") return false;
		if (t == "manager") return false;
		if (t == "all") return false;
		error = "Expected userType; patron, manager, or all.";
		return true;
	}

	gold::var user::findOne(list args) {
		auto value = user::col.findOne(args);
		return collection::setParentModel(
			value, user::getPrototype());
	}

	gold::var user::findMany(list args) {
		auto value = user::col.findMany(args);
		return collection::setParentModel(
			value, user::getPrototype());
	}

}  // namespace gg