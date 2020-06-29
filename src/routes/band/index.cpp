#include <iostream>

#include "band.hpp"
#include "bootstrap.hpp"
#include "session.hpp"
#include "template.hpp"
#include "user.hpp"

using namespace gold;

namespace gg {
	using namespace gold;
	using namespace gg::bs;
	using div = HTML::div;
	using link = HTML::link;
	gold::list band::bandIndex(session sesh, user u, band b) {
		auto q = string();
		if (sesh && !sesh.getBool("useCookies"))
			q = "?s=" + sesh.getString("_id");
		auto iconSrc =
			"/upload/" + b.getString("icon") + "/image" + q;
		auto iconPage = "/upload/" + b.getString("icon") + q;
		auto content = gold::list{
			div({
				obj{{"class", "card pageCard text-light bg-dark"}},
				div({
					obj{{"class", "card-body"}},
					h5({obj{{"class", "card-title"}},
							b.getString("name")}),
					div({
						a({
							atts{{"href", iconPage}},
							img({atts{
								{"src", iconSrc},
								{"width", "256"},
								{"height", "256"},
								{"class", "float-left shadow-lg rounded mr-3"},
							}}),
						}),
						div({
							atts{{"class", "descContainer"}},
							b.getString("longDesc"),
						}),
					}),

				}),
			}),
		};
		return content;
	}
}  // namespace gg