#include <iostream>

#include "../../../include/bootstrap.hpp"
#include "band.hpp"
#include "session.hpp"
#include "template.hpp"
#include "upload.hpp"
#include "user.hpp"

using namespace gold;

namespace gg {
	using namespace gold;
	using namespace gg::bs;
	using link = HTML::link;
	using div = HTML::div;

	form bandForm(user u, obj data, obj errs) {
		return form({
			atts{{"method", "post"}},
			row({
				col({formInputRow(
					"text", "name", "Name", true, errs.getString("name"),
					atts{{"value", data.getString("name")}})}),
				col({formInputRow(
					"email", "contact", "Contact Email", true,
					errs.getString("contact"),
					atts{
						{"value", data.getString(
												"contact", u.getString("email"))}})}),
			}),
			row({
				bs::col({formInputRow(
					"text", "country", "Country", true,
					errs.getString("country"),
					atts{{"value", data.getString("country")}})}),
				bs::col({formInputRow(
					"text", "zone", "State", true, errs.getString("zone"),
					atts{{"value", data.getString("zone")}})}),
				bs::col({formInputRow(
					"text", "city", "City", true, errs.getString("city"),
					atts{{"value", data.getString("city")}})}),
			}),
			row({
				bs::col({formInputRow(
					"text", "address1", "Address 1", true,
					errs.getString("address1"),
					atts{{"value", data.getString("address1")}})}),
				bs::col({formInputRow(
					"text", "address2", "Address 2", false,
					errs.getString("address2"),
					atts{{"value", data.getString("address2")}})}),
				bs::col({formInputRow(
					"text", "zip", "ZIP", true, errs.getString("zip"),
					atts{{"value", data.getString("zip")}})}),
			}),
			row({
				col({formInputColumn(
					"text", "shortDesc", "Short Description", true,
					errs.getString("shortDesc"),
					atts{
						{"value", data.getString("shortDesc")},
						{"maxLength", 256},
						{"spellcheck", true},
						{"style", "height: 5em;"},
					})}),
			}),
			row({
				col({
					atts{{"style", "margin-bottom: 92px;"}},
					input({
						atts{
							{"type", "hidden"},
							{"id", "editorOutput"},
							{"name", "longDesc"},
							{"required", true},
							{"value", data.getString("longDesc")},
						},
					}),
					label({
						atts{{"class", "form-label"}, {"for", "longDesc"}},
						"Long Description",
					}),
					div({
						atts{{"class", "text-dark bg-light rounded"}},
						div({
							atts{{"id", "editor"}, {"class", "rounded"}},
							data.getString("longDesc"),
						}),
					}),
				}),
			}),
			row({
				col({
					button({
						atts{
							{"type", "button"},
							{"id", "iconInput"},
							{"class", "btn btn-primary"},
						},
						"Upload Icon",
					}),
					input({
						atts{
							{"type", "hidden"},
							{"id", "iconFile"},
							{"name", "icon"},
						},
					}),
					span({
						atts{{"id", "iconFileName"}, {"class", "px-1"}},
					}),
					img({
						atts{
							{"id", "iconPreview"},
							{"class", "rounded-circle invisible"},
							{"width", "32"},
							{"height", "32"},
						},
					}),
				}),
			}),
			button({
				obj{
					{"type", "submit"},
					{"class", "btn btn-primary float-right"},
				},
				"Submit",
			}),
		});
	}

	gold::list band::bandCreate(
		session s, user u, obj data, obj errs) {
		auto errorBody = tbody();
		if (errs)
			for (auto it = errs.begin(); it != errs.end(); ++it) {
				//<th scope="row">1</th>
				errorBody += {
					tr({
						th({atts{{"scope", "row"}}, nameMap[it->first]}),
						th(list::initList{it->second}),
					}),
				};
			}
		auto errorTable =
			table({atts{{"class", "table"}},
						 thead({
							 tr({
								 th({atts{{"scope", "bs::col"}}, "Field"}),
								 th({atts{{"scope", "bs::col"}}, "Error"}),
							 }),
						 }),
						 errorBody});
		auto greetings =
			string("Hello, ") + u.getString("firstName") + "!";
		auto content = gold::list{
			link({atts{
				{"rel", "stylesheet"},
				{"href", "/css/main/quill.snow.css"},
			}}),
			link({atts{
				{"rel", "stylesheet"},
				{"href", "/css/Croppie/croppie.css"},
			}}),
			script({atts{
				{"src", "/js/main/quill.min.js"},
			}}),
			script({atts{
				{"src", "/js/Croppie/croppie.min.js"},
			}}),
			div({
				atts{{"class", "card pageCard text-light bg-dark"}},
				div({
					atts{{"class", "card-body"}},
					h5({
						atts{{"class", "card-title"}},
						greetings,
					}),
					p({"To create a band you need to fill out this "
						 "form. We try to pack in as much as possible "
						 "so visitors to the site can better find it. "
						 "Accurate information is recommended. If you have "
						 "any issues, please feel free to contact "
						 "support."}),
					errs.size() > 0 ? div({errorTable}) : div{},
					bandForm(u, data, errs),
					upload::cropperDialog(),
				}),
			}),
			script({
				atts{
					{"defer", true},
					{"src", "/js/main/quillCallback.js"},
				},
			}),
			script({
				atts{
					{"defer", true},
					{"src", "/js/main/croppieCallback.js"},
				},
			}),
		};
		return content;
	}
}  // namespace gg