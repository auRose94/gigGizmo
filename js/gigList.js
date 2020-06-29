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
	window.nameFormatter = (value, row, index, field) => {
		let link = document.createElement("a");
		let itemId = row["_id"];
		link.innerHTML = value;
		link.setAttribute("href", `/gig/${itemId}${q}`);
		if (row.icon != "") {
			let img = document.createElement("img");
			link.appendChild(img);
			img.setAttribute("src", `/upload/${row.icon}/image${q}`);
			img.setAttribute("class", "rounded");
			img.setAttribute("width", 32);
			img.setAttribute("height", 32);
		}
		return link.outerHTML;
	};

	window.bandCache = new Map();
	window.venueCache = new Map();

	window.gigResponseHandler = (res) => {
		let bandIds = new Set();
		let venueIds = new Set();
		for (index in res.rows) {
			let item = res.rows[index];
			for (idIndex in item.bands)
				bandIds.add(item.bands[idIndex]);
			venueIds.add(item.venue);
		}
		let bands = new Promise((res, rej) => {
			let parm = {
				_id: { $in: Array.from(bandIds) }
			};
			if (!sesh.useCookies)
				parm.s = sesh._id;
			$.ajax({
				type: 'GET',
				dataType: "json",
				url: `/find/band.json`,
				data: parm,
				success: res,
				error: rej
			});
		});
		let venues = new Promise((res, rej) => {
			let parm = {
				_id: { $in: Array.from(venueIds) }
			};
			if (!sesh.useCookies)
				parm.s = sesh._id;
			$.ajax({
				type: 'GET',
				dataType: "json",
				url: `/find/venue.json`,
				data: parm,
				success: res,
				error: rej
			});
		});
		res.venues = venues;
		res.bands = bands;
		return res;
	};

	let tableElem = $(".table");
	tableElem.on("load-success.bs.table", (data, res, status) => {
		Promise.all([res.bands, res.venues]).then(([b, v]) => {
			for (index in v) {
				let vItem = v[index];
				venueCache.set(vItem._id, vItem);
			}
			for (index in b) {
				let bItem = b[index];
				bandCache.set(bItem._id, bItem);
			}
			console.log(bandCache, venueCache);
			console.log(data, res, status);
		});
	});
})();