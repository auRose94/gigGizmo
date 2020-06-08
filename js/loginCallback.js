(() => {
	let form = $("#login")[0];
	let formSubmit = $("#formSubmit")[0];
	let onFormSubmit = (e) => {
		e.preventDefault();
		e.stopImmediatePropagation();
		let email = $("#email").val();
		let password = $("#password").val();
		let Authorization = "Basic " + btoa(email + ":" + password);
		$.ajax({
			type: "POST",
			dataType: "json",
			headers: { Authorization },
			success: (resp) => {
				if (resp.session) {
					let sesh = resp.session;
					localStorage.setItem("session", JSON.stringify(sesh));
					location.assign(`/home${
						resp.useCookies ? "" : `?s=${sesh._id}`
					}`);
				}
			},
			error: (e) => {
				console.error(e);
			}
		});
	};
	form.onsubmit = onFormSubmit;
	formSubmit.click(onFormSubmit);
})();