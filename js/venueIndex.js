(() => {
	mapboxgl.accessToken =
		"pk.eyJ1IjoiY29yeW51bGwiLCJhIjoiY2o3cXd1aXZhMmhoNjJx" +
		"cW55a21wZjZybiJ9.pzgYjyACUqqxlKnmd_iTJw";
	let map = new mapboxgl.Map({
		container: "mapLocationContainer",
		style: "mapbox://styles/mapbox/dark-v10"
	});
	window.map = map;
})();