(async () => {
	const toBase64 = file => new Promise((resolve, reject) => {
		const reader = new FileReader();
		reader.readAsDataURL(file);
		reader.onload = () => resolve(reader.result);
		reader.onerror = error => reject(error);
	});
	let onFileSelect = async () => {
		let fileInputDummy = $("#fileInputDummy");
		fileInputDummy.trigger("click");
		fileInputDummy.on("change", async (event) => {
			let lf = event.target.files[0];
			if (lf.name.search(/\.gif/g) != -1) {
				//GIF, No cropping
				const reader = new FileReader();
				reader.addEventListener("load", function () {
					$("#iconFileName").html(lf.name);
					$("#iconPreview").attr("src", reader.result);
					$("#iconPreview").removeClass("invisible");
					$("#iconFile").val(reader.result);
				}, false);
				if (lf) reader.readAsDataURL(lf);
			} else {
				$('#cropperModal').modal('show');
				$('#cropperModal').on('shown.bs.modal', async () => {
					let bin = await toBase64(lf);
					let el = $("#cropper");
					if (el.hasClass("croppie-container"))
						await el.croppie("bind", { url: bin });
					else
						el.croppie({
							url: bin,
							enableExif: true,
							enforceBoundary: false,
							viewport: { width: 200, height: 200, type: 'square' },
						});
					$("#cropperModalAccept").on("click", async () => {
						let size = { width: 1024 };
						let res = await el.croppie("result", {
							type: "base64",
							size,
							format: "png",
							quality: 1,
							circle: false
						});
						$("#iconFileName").html(lf.name);
						$("#iconPreview").attr("src", res);
						$("#iconPreview").removeClass("invisible");
						$("#iconFile").val(res);
						$('#cropperModal').modal('hide');
					});
				});
			}

		});

	};
	$("#iconInput").on("click", onFileSelect);

	let currentIcon = $("#iconFile").val();
	if (!!currentIcon) {
		$("#iconFileName").html("From reload");
		$("#iconPreview")
			.attr("src", currentIcon)
			.removeClass("invisible");
	}
})();

