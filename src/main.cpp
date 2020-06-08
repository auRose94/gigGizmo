#include <database.hpp>
#include <iostream>
#include <server.hpp>
#include <thread>

#include "band.hpp"
#include "error.hpp"
#include "index.hpp"
#include "login.hpp"
#include "register.hpp"
#include "session.hpp"
#include "template.hpp"
#include "upload.hpp"
#include "user.hpp"
#include "venue.hpp"

using namespace gold;
using namespace gg;

int main() {
	auto serv = server(obj{});
	auto db =
		database({{"name", "GigGizmo"}, {"appName", "gig-gizmo"}});
	serv.setMountPoint({"./js", "./css", "./assets"});
	auto con = db.connect();
	if (con.isError()) {
		cerr << con << endl;
		return 1;
	}

	session::getPrototype().setString(
		"domain", "127.0.0.1:8080");

	setIndexRoute(db, serv);

	band::setRoutes(db, serv);
	session::setRoutes(db, serv);
	upload::setRoutes(db, serv);
	user::setRoutes(db, serv);
	venue::setRoutes(db, serv);

	serv.setErrorHandler({func([&](gold::list args) -> gold::var {
		auto req = args[0].getObject<request>();
		auto res = args[1].getObject<response>();
		gold::list content =
			errorPage({genericError("404 Page not found")});
		res.writeStatus({"404 NOT FOUND"});
		res.end({getTemplate(req, content)});
		return gold::var();
	})});

	serv.start();
	serv.destroy();
	db.destroy();

	return 0;
}