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
	gold::list venue::venueOptions(user u, venue ven) {
		auto content = gold::list{
			div({
				obj{{"class", "card pageCard text-light bg-dark"}},
				div({
					obj{{"class", "card-body"}},
					h5({
						obj{{"class", "card-title"}},
						"Find Venues",
					}),
				}),

			}),
		};
		return content;
	}
}  // namespace gg