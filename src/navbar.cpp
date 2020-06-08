#include "navbar.hpp"

#include "session.hpp"
#include "user.hpp"

namespace gg {
	using div = HTML::div;
	using namespace HTML;

	li navDropdown(string id, string title, gold::list children) {
		auto container = div({
			obj{
				{"class", "dropdown-menu"},
				{"aria-labelledby", id},
			},
		});
		container += children;
		auto drop = li({
			obj{{"class", "nav-item dropdown"}},
			a({
				obj{
					{"class", "nav-link dropdown-toggle"},
					{"href", "#"},
					{"id", id},
					{"role", "button"},
					{"data-toggle", "dropdown"},
					{"aria-haspopup", "true"},
					{"aria-expanded", "false"},
				},
				title,
			}),
			container,
		});
		return drop;
	}

	a navDropdownItem(string href, string title, string curr) {
		auto active = curr == href;
		auto aClass = string(active ? " active" : "");
		auto className = "dropdown-item" + aClass;
		auto aItem = a({
			obj{{"class", className}, {"href", href}},
			title,
		});
		if (active) {
			aItem += {span({
				obj{{"class", "sr-only"}},
				" (current)",
			})};
		}
		return aItem;
	}

	li navOption(string href, string title, string curr) {
		auto active = curr == href;
		auto aClass = string(active ? " active" : "");
		auto className = "nav-item" + aClass;
		auto aItem = a({
			obj{{"class", "nav-link"}, {"href", href}},
			title,
		});
		if (active) {
			aItem += {span({
				obj{{"class", "sr-only"}},
				" (current)",
			})};
		}
		auto op = li({
			obj{{"class", className}},
			aItem,
		});

		return op;
	}

	gold::HTML::ul navbar(
		user u, session sesh, string current) {
		string firstName = "User";
		string q = "";
		auto errors = gold::list();
		if (u) firstName = u.getString("firstName");
		if (sesh) q = "?s=" + sesh.getID();

		auto ret = ul({
			obj{{"class", "navbar-nav mr-auto"}},
		});
		auto userDropdownTitle = u ? firstName : "Guest";
		if (u) {
			auto vItems = gold::list({
				navDropdownItem("/venue" + q, "New Venue", current),
				navDropdownItem(
					"/list/venue" + q, "List Venues", current),
				navDropdownItem(
					"/find/venue" + q, "Find Venue", current),
			});
			auto bItems = gold::list({
				navDropdownItem("/band" + q, "New Band", current),
				navDropdownItem(
					"/list/band" + q, "List Bands", current),
				navDropdownItem("/find/band" + q, "Find Band", current),
			});
			auto uItem = gold::list({
				navDropdownItem("/home" + q, "Home", current),
				navDropdownItem("/options" + q, "Options", current),
				navDropdownItem("/register" + q, "Register", current),
				navDropdownItem("/login" + q, "Login", current),
			});
			ret +=
				{navDropdown("userDropdown", userDropdownTitle, uItem),
				 navDropdown("venueDropdown", "Venue", vItems),
				 navDropdown("bandDropdown", "Band", bItems)};
		} else {
			auto uItems = gold::list({
				navDropdownItem("/register" + q, "Register", current),

				navDropdownItem("/login" + q, "Login", current),
			});

			ret +=
				{navDropdown("userDropdown", userDropdownTitle, uItems),
				 navOption("/find/venue/" + q, "Find Venue", current),
				 navOption("/find/band/" + q, "Find Music", current)};
		}

		auto errJson = errors.getJSON();
		auto errComment = string("<!--") + errJson.dump() + "-->";
		ret += {errComment};
		return ret;
	}

	gold::HTML::ul navbar(request req) {
		auto path = req.getString("path");
		auto sesh =
			req.callMethod("getSession").getObject<session>();
		auto sessionID = sesh.getString("_id");
		auto uVar = gold::var();
		string firstName = "User";
		string rootUrl = "";
		user u;
		auto errors = gold::list();
		if (!sesh.isExpired()) {
			u = sesh.getUser().getObject<user>();
		}
		return navbar(u, sesh, path);
	}
}  // namespace gg