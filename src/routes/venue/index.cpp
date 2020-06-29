#include <iostream>

#include "bootstrap.hpp"
#include "session.hpp"
#include "template.hpp"
#include "user.hpp"
#include "venue.hpp"

using namespace gold;

namespace gg {
	using namespace gold;
	using namespace gg::bs;
	using div = HTML::div;
	using link = HTML::link;
	gold::list venue::venueIndex(session sesh, user u, venue b) {
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
					div({
						atts{
							{"id", "mapLocationContainer"},
							{"style", "width:100%;min-height:300px;"},
						},
					}),
				}),
			}),

			link({atts{
				{"rel", "stylesheet"},
				{"href", "/css/mapbox-gl-js/mapbox-gl.min.css"},
			}}),
			script({atts{
				{"src", "/js/mapbox-gl-js/mapbox-gl.min.js"},
			}}),
			script({
				atts{
					{"defer", true},
					{"src", "/js/main/venueIndex.js"},
				},
			}),
		};
		return content;
	}
}  // namespace gg