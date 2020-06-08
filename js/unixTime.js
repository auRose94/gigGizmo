(async () => {
	let changeAllElements = () => {
		let items = $(".unixTime");
		items.each((_, item) => {
			let unix =
				Number.parseInt(item.getAttribute("data-unix-mill")) ||
				Number.parseInt(item.innerText);
			let date = luxon.DateTime.fromMillis(unix);
			item.innerText = date.toRelative();
			item.setAttribute("data-unix-mill", unix);
		});
	};
	changeAllElements();
	setInterval(() => {
		changeAllElements();
	}, 1000);
})();