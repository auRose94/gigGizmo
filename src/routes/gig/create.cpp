#include <iostream>

#include "../../../include/bootstrap.hpp"
#include "gig.hpp"
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

	form gigForm(user u, obj data, obj errs) {
		return form({
			atts{{"method", "post"}},
			row({
				col({
					input({
						atts{
							{"type", "hidden"},
							{"id", "bandIds"},
							{"name", "bands"},
							{"required", true},
							{"value", data.getString("bands")},
						},
					}),
					formInputRow(
						"text", "bandNames", "Bands", true,
						errs.getString("bandNames"),
						atts{
							{"value", data.getString("bandNames")},
						}),
				}),
				col({
					input({
						atts{
							{"type", "hidden"},
							{"id", "venueId"},
							{"name", "venue"},
							{"required", true},
							{"value", data.getString("venue")},
						},
					}),
					formInputRow(
						"text", "venueName", "Venue", true,
						errs.getString("venueName"),
						atts{
							{"value", data.getString("venueName")},
						}),
				}),
			}),
			row({
				col({
					formInputRow(
						"date", "date", "Date", true,
						errs.getString("date"),
						atts{
							{"value", data.getString("date")},
						}),
				}),
				col({
					formInputRow(
						"time", "startTime", "Start", true,
						errs.getString("startTime"),
						atts{
							{"value", data.getString("startTime")},
						}),
				}),
				col({
					formInputRow(
						"time", "endTime", "End", true,
						errs.getString("endTime"),
						atts{
							{"value", data.getString("endTime")},
						}),
				}),
			}),
			row({
				col({
					atts{{"style", "margin-bottom: 92px;"}},
					input({
						atts{
							{"type", "hidden"},
							{"id", "editorOutput"},
							{"name", "desc"},
							{"required", true},
							{"value", data.getString("desc")},
						},
					}),
					label({
						atts{{"class", "form-label"}, {"for", "desc"}},
						"Description",
					}),
					div({
						atts{{"class", "text-dark bg-light rounded"}},
						div({
							atts{{"id", "editor"}, {"class", "rounded"}},
							data.getString("desc"),
						}),
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

	gold::list gig::gigCreate(
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
				{"href", "/css/jquery-ui/jquery-ui.min.css"},
			}}),
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
			script({atts{
				{"src", "/js/jquery-ui/jquery-ui.js"},
			}}),
			div({
				atts{{"class", "card pageCard text-light bg-dark"}},
				div({
					atts{{"class", "card-body"}},
					h5({
						atts{{"class", "card-title"}},
						greetings,
					}),
					p({"To create a gig you need to fill out this "
						 "form. You need to find your band and venue by "
						 "name. You can include information about what's "
						 "going to be availible that night, that being "
						 "drinks and songs. If you have any issues, please "
						 "feel free to contact support."}),
					errs.size() > 0 ? div({errorTable}) : div{},
					gigForm(u, data, errs),
					upload::cropperDialog(),
				}),
			}),
			script({
				atts{
					{"defer", true},
					{"src", "/js/main/quillCallback.js"},
				},
			}),
			script({atts{
				{"src", "/js/main/gigCreate.js"},
			}}),
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