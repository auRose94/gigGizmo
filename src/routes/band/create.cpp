#include <iostream>

#include "bootstrap.hpp"
#include "session.hpp"
#include "template.hpp"
#include "user.hpp"
#include "band.hpp"

using namespace gold;

namespace gg {
	using namespace gold;
	using namespace gg::bs;
	using div = HTML::div;
	gold::list band::bandCreate(user u, obj data, obj errs) {
		auto greetings =
			string("Hello, ") + u.getString("firstName") + "!";
		auto content = gold::list{
			div({
				obj{{"class", "card pageCard text-light bg-dark"}},
				div({
					obj{{"class", "card-body"}},
					h5({
						obj{{"class", "card-title"}},
						greetings,
					}),
				}),

			}),
		};
		return content;
	}
}  // namespace gg