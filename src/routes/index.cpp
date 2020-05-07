#include "index.hpp"

#include <html.hpp>
#include <iostream>

#include "template.hpp"

namespace gg {

	void setIndexRoute(database, server serv) {
		func getIndex = [](const list& args) -> gold::var {
			using namespace HTML;
			auto req = args[0].getObject<request>();
			auto res = args[1].getObject<response>();
			auto resp = getTemplate(req, {p({"Hello world 2.0"})});

			auto d = (string)(resp);
			res.writeHeader({"Content-Type", "text/html"});
			return res.end({d});
		};

		// Session
		serv.get({"/", getIndex});

		// Without
		serv.get({"/:sessionID/", getIndex});
	}
}  // namespace gg