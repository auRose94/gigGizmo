(async () => {
	let sesh = JSON.parse(localStorage.getItem("session"));
	let q = "";
	if (!sesh.useCookies)
		q = `?s=${sesh._id}`;
	window.dateFormatter = (value, row, index, field) => {
		let span = document.createElement("span");
		span.setAttribute("class", "unixTime");

		let unix = Number.parseInt(value);
		let date = luxon.DateTime.fromMillis(unix);
		span.innerText = date.toRelative();
		span.setAttribute("data-unix-mill", unix);

		return span.outerHTML;
	};
	window.descFormatter = (value, row, index, field) => {
		let link = document.createElement("a");
		let itemId = row["_id"];
		link.innerHTML = value || itemId;
		link.setAttribute("href", `/upload/${itemId}${q}`);
		if (row.src != "") {
			let img = document.createElement("img");
			link.appendChild(img);
			img.setAttribute("src", `/upload/${itemId}/image${q}`);
			img.setAttribute("class", "rounded");
			img.setAttribute("width", 32);
			img.setAttribute("height", 32);
		}
		return link.outerHTML;
	};
})();