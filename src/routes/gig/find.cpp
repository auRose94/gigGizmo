#include <iostream>

#include "bootstrap.hpp"
#include "gig.hpp"
#include "session.hpp"
#include "template.hpp"
#include "user.hpp"

using namespace gold;

namespace gg {
	using namespace gold;
	using namespace gg::bs;
	using div = HTML::div;
	gold::list gig::gigFind(
		session sesh, user u, obj data, list items) {
		auto content = gold::list{
			div({
				obj{{"class", "card pageCard text-light bg-dark"}},
				div({
					obj{{"class", "card-body"}},
					h5({
						obj{{"class", "card-title"}},
						"Find Gigs",
					}),
				}),

			}),
		};
		return content;
	}
}  // namespace gg