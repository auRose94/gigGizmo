#include "template.hpp"

#include "navbar.hpp"
#include "session.hpp"
#include "user.hpp"

namespace gg {
	using link = HTML::link;
	using div = HTML::div;

	html getTemplateEx(
		user& u, session& s, string path, gold::list content,
		gold::list header) {
		using namespace HTML;
		auto sessionID = s ? s.getID() : "";
		auto rootUrl = string();
		if (s) rootUrl = string("/") + sessionID;
		auto headEl = head({
			obj{{"lang", "en"}},
			title({"GigGizmo"}),
			meta{{obj{{"charset", "utf-8"}}}},
			meta({obj{
				{"name", "viewport"},
				{"content",
				 "width=device-width, initial-scale=1, "
				 "shrink-to-fit=no"},
			}}),
			link({obj{
				{"rel", "shortcut icon"},
				{"href", "/assets/favicon.ico"},
			}}),
			link({obj{
				{"rel", "stylesheet"},
				{"href", "/css/bootstrap/bootstrap.min.css"},
			}}),
			link({obj{
				{"rel", "stylesheet"},
				{"href", "/css/bootstrap/bootstrap-utilities.min.css"},
			}}),
			link({obj{
				{"rel", "stylesheet"},
				{"href", "/css/bootstrap/bootstrap-reboot.min.css"},
			}}),
			link({obj{
				{"rel", "stylesheet"},
				{"href", "/css/bootstrap/bootstrap-grid.min.css"},
			}}),
			link({obj{
				{"rel", "stylesheet"},
				{"href", "/css/baseStyle.css"},
			}}),
			script({obj{
				{"src", "/js/bootstrap/bootstrap.bundle.min.js"},
			}}),
		});
		headEl += header;
		auto bodyEl = body({
			nav({
				obj{
					{"class",
					 "navbar navbar-expand-lg navbar-dark bg-dark p-1 "
					 "fixed-top"},
				},
				a({
					obj{
						{"class", "navbar-brand"},
						{"href", rootUrl + "/"},
					},
					img({
						obj({
							{"src", "/assets/logo_white.svg"},
							{"height", "32"},
							{"class", "logo"},
						}),
					}),
					"GigGizmo",
				}),
				button({
					obj{
						{"class", "navbar-toggler"},
						{"type", "button"},
						{"data-toggle", "collapse"},
						{"data-target", "#navbarSupportedContent"},
						{"aria-controls", "navbarSupportedContent"},
						{"aria-expanded", "false"},
						{"aria-label", "Toggle navigation"},
					},
					span({
						obj{
							{"class", "navbar-toggler-icon"},
						},
					}),
				}),
				div({
					obj{
						{"class", "collapse navbar-collapse"},
						{"id", "navbarSupportedContent"},
					},
					navbar(u, s, path),
					form({
						obj{
							{"class", "form-inline my-2 my-lg-0 d-flex"},
						},
						input({
							obj{
								{"class", "form-control mr-sm-2"},
								{"type", "search"},
								{"placeholder", "Search"},
								{"aria-label", "Search"},
							},
						}),
						button({
							obj{
								{"class",
								 "btn btn-outline-success my-2 my-sm-0"},
								{"type", "submit"},
							},
							"Search",
						}),
					}),
				}),
			}),
		});
		bodyEl += content;
		return html({
			headEl,
			bodyEl,
		});
	}

	html getTemplate(
		request req, gold::list content, gold::list header) {
		using namespace HTML;
		auto path = req.getString("path");
		auto sessionID = req.getParameter({0}).getString();
		auto seshVar =
			session::findOne({obj{{"_id", sessionID}}});
		auto sesh = seshVar.getObject<session>();
		auto uVar = sesh ? sesh.getUser() : gold::var();
		auto u = user();
		if (uVar.isObject()) uVar.returnObject<user>(u);
		return getTemplateEx(u, sesh, path, content, header);
	}  // namespace gg
}  // namespace gg