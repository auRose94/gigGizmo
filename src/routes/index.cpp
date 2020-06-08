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
			return res.end(
				{getTemplate(req, {p({"Hello world 2.0"})})});
		};

		serv.get({"/", getIndex});
	}
}  // namespace gg