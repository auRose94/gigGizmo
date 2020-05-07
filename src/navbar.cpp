#include "navbar.hpp"

#include "session.hpp"
#include "user.hpp"

namespace gg {
	using div = HTML::div;
	using namespace HTML;

	li navDropdown(
		string id, string title, gold::list children) {
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
		user& u, session& sesh, string current) {
		string firstName = "User";
		string rootUrl = "";
		auto errors = gold::list();
		if (u) firstName = u.getString("firstName");
		if (sesh) rootUrl = string("/") + sesh.getID();

		auto ret = ul({
			obj{{"class", "navbar-nav mr-auto"}},
		});
		auto userDropdownTitle = u ? firstName : "Guest";
		if (u) {
			auto vItems = gold::list({
				navDropdownItem(
					rootUrl + "/venue/", "New Venue", current),
				navDropdownItem(
					rootUrl + "/list/venue/", "List Venues", current),
				navDropdownItem(
					rootUrl + "/find/venue/", "Find Venue", current),
			});
			auto bItems = gold::list({
				navDropdownItem(
					rootUrl + "/band/", "New Band", current),
				navDropdownItem(
					rootUrl + "/list/band/", "List Bands", current),
				navDropdownItem(
					rootUrl + "/find/band/", "Find Band", current),
			});
			auto uItem = gold::list({
				navDropdownItem(rootUrl + "/home", "Home", current),
				navDropdownItem(
					rootUrl + "/options", "Options", current),
				navDropdownItem(
					rootUrl + "/register", "Register", current),
				navDropdownItem(rootUrl + "/login", "Login", current),
			});
			ret +=
				{navDropdown("userDropdown", userDropdownTitle, uItem),
				 navDropdown("venueDropdown", "Venue", vItems),
				 navDropdown("bandDropdown", "Band", bItems)};
		} else {
			auto uItems = gold::list({
				navDropdownItem(
					rootUrl + "/register", "Register", current),

				navDropdownItem(rootUrl + "/login", "Login", current),
			});

			ret +=
				{navDropdown("userDropdown", userDropdownTitle, uItems),
				 navOption(
					 rootUrl + "/find/venue/", "Find Venue", current),
				 navOption(
					 rootUrl + "/find/band/", "Find Music", current)};
		}

		auto errJson = errors.getJSON();
		auto errComment = string("<!--") + errJson.dump() + "-->";
		ret += {errComment};
		return ret;
	}

	gold::HTML::ul navbar(request req) {
		auto path = req.getString("path");
		auto sessionID = req.getParameter({0}).getString();
		auto uVar = gold::var();
		auto seshVar = gold::var();
		string firstName = "User";
		string rootUrl = "";
		user u;
		session sesh;
		auto errors = gold::list();
		if (sessionID.size() > 0) {
			seshVar = session::findOne({obj{{"_id", sessionID}}});
			if (seshVar.isError()) errors.pushVar(seshVar);
			if (seshVar.isObject()) {
				sesh = seshVar.getObject<session>();
				if (!sesh.isExpired()) {
					uVar = sesh.getUser();
					if (uVar.isError()) errors.pushVar(uVar);
					if (uVar.isObject()) {
						u = uVar.getObject<user>();
						firstName = u.getString("firstName");
						rootUrl = string("/") + sessionID;
					}
				}
			}
		}
		return navbar(u, sesh, path);
	}
}  // namespace gg