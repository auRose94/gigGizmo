#include <database.hpp>
#include <iostream>
#include <server.hpp>
#include <thread>

#include "band.hpp"
#include "index.hpp"
#include "login.hpp"
#include "register.hpp"
#include "session.hpp"
#include "template.hpp"
#include "user.hpp"
#include "venue.hpp"

using namespace gold;
using namespace gg;

int main() {
	auto serv = server();
	auto db = database({
		{"name", "GigGizmo"},
		{"appName", "gig-gizmo"},
	});
	serv.setMountPoint({"./js", "./css", "./assets"});
	auto con = db.connect();
	if (con.isError()) {
		cerr << con << endl;
		return 1;
	}

	setIndexRoute(db, serv);
	
	band::setRoutes(db, serv);
	session::setRoutes(db, serv);
	user::setRoutes(db, serv);
	venue::setRoutes(db, serv);

	serv.start();
	serv.destroy();
	db.destroy();

	return 0;
}