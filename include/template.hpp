#pragma once

#include <html.hpp>
#include <server.hpp>

#include "session.hpp"
#include "user.hpp"

namespace gg {
	using namespace gold;
	using namespace HTML;

	html getTemplateEx(
		user u, session s, string path, gold::list content,
		gold::list header = gold::list());
	html getTemplate(
		request req, gold::list content,
		gold::list header = gold::list());
}  // namespace gg