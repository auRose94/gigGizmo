#include <iostream>

#include "bootstrap.hpp"
#include "session.hpp"
#include "template.hpp"
#include "user.hpp"

namespace gg {
	using namespace gold;
	using namespace gg::bs;
	using div = HTML::div;

	std::map<string, string> user::nameMap =
		std::map<string, string>({
			{"userType", "User Type"},
			{"firstName", "First"},
			{"midName", "Middle"},
			{"lastName", "Last"},
			{"phone", "Phone"},
			{"birthday", "Birthday"},
			{"gender", "Gender"},
			{"country", "Country"},
			{"zone", "Zone"},
			{"city", "City"},
			{"address1", "Address 1"},
			{"address2", "Address 2"},
			{"zip", "ZIP"},
			{"email", "Email"},
			{"confEmail", "Confirm Email"},
			{"password", "Password"},
			{"confPassword", "Confirm Password"},
		});

	gold::list user::userRegister(obj data, obj errs) {
		auto errorBody = tbody();
		if (errs)
			for (auto it = errs.begin(); it != errs.end(); ++it) {
				//<th scope="row">1</th>
				errorBody += {
					tr({
						th({obj{{"scope", "row"}}, nameMap[it->first]}),
						th(list::initList{it->second}),
					}),
				};
			}
		auto errorTable =
			table({obj{{"class", "table"}},
						 thead({
							 tr({
								 th({obj{{"scope", "bs::col"}}, "Field"}),
								 th({obj{{"scope", "bs::col"}}, "Error"}),
							 }),
						 }),
						 errorBody});
		auto uType = data.getString("userType");
		auto gender = data.getString("gender");
		auto content = gold::list({
			div({
				obj{{"class", "card pageCard"}},
				div({
					obj{{"class", "card-body"}},
					h5({
						obj{{"class", "card-title"}},
						"Registration",
					}),
					errs.size() > 0 ? div({errorTable}) : div{},
					form({
						obj{{"method", "post"}},
						row({
							bs::col({label({"Who are you?"})}),
							bs::col({formRadio(
								"userType", "uTPatron", "Patron", "patron",
								{
									{"checked",
									 uType == "" || uType == "patron"},
								})}),
							bs::col({formRadio(
								"userType", "uTManager", "Manager", "manager",
								{
									{"checked", uType == "manager"},
								})}),
							bs::col({formRadio(
								"userType", "uTAll", "All", "all",
								{
									{"checked", uType == "all"},
								})}),
						}),
						row({
							bs::col({formInput(
								"text", "firstName", "First", true,
								errs.getString("firstName"),
								obj{{"value", data.getString("firstName")}})}),
							bs::col({formInput(
								"text", "midName", "Middle", false,
								errs.getString("midName"),
								obj{{"value", data.getString("midName")}})}),
							bs::col({formInput(
								"text", "lastName", "Last", true,
								errs.getString("lastName"),
								obj{{"value", data.getString("lastName")}})}),
						}),
						row({
							bs::col({formInput(
								"tel", "phone", "Phone", true,
								errs.getString("phone"),
								{
									{"pattern", "\\+[0-9]{8,}"},
									{"placeholder", "+###########"},
									{"autocorrect", "on"},
									{"value", data.getString("phone")},
								})}),
							bs::col({formInput(
								"date", "birthday", "Birthday", true,
								errs.getString("birthday"),
								obj{{"value", transformBirthday(data.getString(
																"birthday"))}})}),
							bs::col({formSelect(
								"gender", "Gender", false, {},
								{
									formSelectOption("", "", gender == ""),
									formSelectOption(
										"Non-Binary", "n", gender == "n"),
									formSelectOption(
										"Female", "f", gender == "f"),
									formSelectOption("Male", "m", gender == "m"),
								})}),
						}),
						row({
							bs::col({formInput(
								"text", "country", "Country", true,
								errs.getString("country"),
								obj{{"value", data.getString("country")}})}),
							bs::col({formInput(
								"text", "zone", "State", true,
								errs.getString("zone"),
								obj{{"value", data.getString("zone")}})}),
							bs::col({formInput(
								"text", "city", "City", true,
								errs.getString("city"),
								obj{{"value", data.getString("city")}})}),
						}),
						row({
							bs::col({formInput(
								"text", "address1", "Address 1", true,
								errs.getString("address1"),
								obj{{"value", data.getString("address1")}})}),
							bs::col({formInput(
								"text", "address2", "Address 2", false,
								errs.getString("address2"),
								obj{{"value", data.getString("address2")}})}),
							bs::col({formInput(
								"text", "zip", "ZIP", true,
								errs.getString("zip"),
								obj{{"value", data.getString("zip")}})}),
						}),
						row({
							bs::col({formInput(
								"email", "email", "Email", true,
								errs.getString("email"),
								obj{{"value", data.getString("email")}})}),
							bs::col({formInput(
								"email", "confEmail", "Confirm", true,
								errs.getString("confEmail"),
								obj{{"value", data.getString("confEmail")}})}),
						}),
						row({
							bs::col({formInput(
								"password", "password", "Password", true,
								errs.getString("password"))}),
							bs::col({formInput(
								"password", "confPassword", "Confirm", true,
								errs.getString("confPassword"))}),
						}),
						button({
							obj{
								{"type", "submit"},
								{"class", "btn btn-primary float-right"},
							},
							"Submit",
						}),
					}),
				}),
			}),
		});
		return content;
	}
}  // namespace gg