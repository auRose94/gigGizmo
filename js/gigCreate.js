(() => {
	let sesh = JSON.parse(localStorage.getItem("session"));
	let q = "";
	if (!sesh.useCookies)
		q = `?s=${sesh._id}`;
	let bandIdsInput = $("#bandIds");
	let venueIdsInput = $("#venueId");
	let bandInput = $("#bandNames");
	let venueInput = $("#venueName");

	function split(val) {
		let items = [];
		let qouteDepth = 0;
		let buffer = "";
		for (let i = 0; i < val.length; ++i) {
			let c = val[i];
			if (c == '"') {
				if (qouteDepth == 0)
					qouteDepth++;
				if (qouteDepth > 0)
					qouteDepth--;
			} else if (qouteDepth == 0 && c == ',') {
				items.push(buffer);
				buffer = "";
				i++;
			} else {
				buffer += c;
			}
		}
		if (buffer.length > 0)
			items.push(buffer);
		return items;
	}
	function extractLast(term) {
		return split(term).pop();
	}
	function getNames(items) {
		let names = "";
		items.forEach((item, index) => {
			let name = item.name;
			let commaIndex = name.search(',');
			if (commaIndex != -1)
				name = `"${name}"`;
			names += item.name +
				(index != items.length - 1 ? ", " : "");
		});
		return names;
	}

	function renderItem(ul, item) {
		let iconElement = "";
		if (item.value.icon) {
			iconElement = `<img 
			src="/upload/${item.value.icon}/image" 
			width="32" 
			height="32"></img>`;
		}
		return $("<li>")
			.append(
				"<div>" +
				iconElement +
				item.label +
				"</div>")
			.appendTo(ul);
	}

	function configureInput(idInput, nameInput, route, multi) {
		nameInput.addClass("ui-widget");
		idInput.attr("data-items", "");
		idInput.val("");
		nameInput.val("");
		nameInput
			.on("keydown", function (event) {
				// don't navigate away from the field on tab when selecting an item
				if (event.keyCode === $.ui.keyCode.TAB &&
					$(this).autocomplete("instance").menu.active) {
					event.preventDefault();
				}
			})
			.autocomplete({
				minLength: 0,
				source: function (request, response) {
					let parm = {};
					if (multi) {
						let spl = split(request.term);
						let last = spl.length > 0 ? spl[spl.length - 1] : "";
						parm = {
							q: last
						};
						let keep = [];
						let ids = [];
						let items = Array.from(
							JSON.parse(
								idInput.attr("data-items") || "[]"));
						for (let i = 0; i < items.length; ++i) {
							let item = items[i];
							let name = item.name;
							let commaIndex = name.search(',');
							if (commaIndex != -1)
								name = `"${name}"`;
							if (spl[i] == name) {
								ids.push(item._id);
								keep.push(item);
							}
						}
						idInput.val(JSON.stringify(ids));
						idInput.attr("data-items", JSON.stringify(keep));
					} else {
						parm = {
							q: extractLast(request.term)
						};
						idInput.val("");
						idInput.attr("data-items", "");
					}

					if (!sesh.useCookies)
						parm.s = sesh._id;
					$.ajax({
						type: 'GET',
						dataType: "json",
						url: route,
						data: parm,
						success: (resp) => {
							let data = [];
							Array.from(resp.rows).forEach((item) =>
								data.push({
									label: item.name,
									value: item
								})
							);
							response($.ui.autocomplete.filter(
								data, extractLast(request.term)));
						},
						error: console.error
					});
				},
				focus: function () {
					// prevent value inserted on focus
					return false;
				},
				select: function (event, ui) {
					if (multi) {
						let newId = ui.item.value._id;
						let newItem = ui.item.value;
						let idTerms = Array.from(
							JSON.parse(idInput.val() || "[]"));
						let items = Array.from(
							JSON.parse(
								idInput.attr("data-items") || "[]"));
						let found = false;
						for (let i = 0; i < idTerms.length; ++i) {
							if (idTerms[i] == newId) {
								found = true;
								break;
							}
						}
						if (!found) {
							idTerms.push(newId);
							items.push(newItem);
						}
						idInput.val(JSON.stringify(idTerms));
						idInput.attr(
							"data-items",
							JSON.stringify(items));
						nameInput.val(getNames(items));
					} else {
						let newId = ui.item.value._id;
						let newItem = ui.item.value;
						idInput.val(newId);
						idInput.attr(
							"data-items",
							JSON.stringify(newItem));
						nameInput.val(newItem.name);
					}
					return false;
				}
			})
			.autocomplete("instance")._renderItem = renderItem;
	}
	configureInput(bandIdsInput, bandInput, `/list/band.json`, true);
	configureInput(venueIdsInput, venueInput, `/list/venue.json`, false);

})();