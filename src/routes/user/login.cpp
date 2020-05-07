#include <iostream>

#include "bootstrap.hpp"
#include "session.hpp"
#include "template.hpp"
#include "user.hpp"

using namespace gold;

namespace gg {
	using namespace gold;
	using namespace gg::bs;
	using div = HTML::div;
	gold::list user::userLogin(
		string response, string emailError, string passwordError) {
		string eText, eClass, pText, pClass;
		if (emailError.empty()) {
			eText = "Your address is in good hands";
			eClass = "form-text text-muted";
		} else {
			eText = emailError;
			eClass = "form-text text-danger";
		}
		if (passwordError.empty()) {
			pText = "We securely store everything.";
			pClass = "form-text text-muted";
		} else {
			pText = passwordError;
			pClass = "form-text text-danger";
		}
		auto em = formInput("email", "email", "Email", true);
		em += {small({
			obj{{"id", "emailHelp"}, {"class", eClass}},
			eText,
		})};
		auto ps =
			formInput("password", "password", "Password", true);
		ps += {small({
			obj{{"id", "passwordHelp"}, {"class", pClass}},
			pText,
		})};
		auto content = gold::list{
			div({
				obj{{"class", "card loginCard"}},
				div({
					obj{{"class", "card-body"}},
					h5({
						obj{{"class", "card-title"}},
						"Login",
					}),
				}),
				h3({obj{
							{"class", response.empty() ? "" : "text-danger"}},
						response}),
				form({
					obj{{"method", "post"}},
					em,
					ps,
					formCheck("rememberMe", "Remember Me!"),
					button({
						obj{
							{"type", "submit"},
							{"class", "btn btn-primary float-right"},
						},
						"Submit",
					}),
				}),
			}),
		};
		return content;
	}
}  // namespace gg