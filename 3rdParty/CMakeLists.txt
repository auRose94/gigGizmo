
get_filename_component(
	BUILD_ROOT
	"${CMAKE_CURRENT_BINARY_DIR}/../"
	ABSOLUTE
)

get_filename_component(
	BOOTSTRAP_ABS
	"./bootstrap/"
	ABSOLUTE
)

if(NOT (EXISTS ${BUILD_ROOT}/js))
	file(
		MAKE_DIRECTORY
		${BUILD_ROOT}/js
	)
endif()

if(NOT (EXISTS ${BUILD_ROOT}/css))
	file(
		MAKE_DIRECTORY
		${BUILD_ROOT}/css
	)
endif()

if(NOT (IS_SYMLINK ${BUILD_ROOT}/js/bootstrap))
	file(
		CREATE_LINK 
		${BOOTSTRAP_ABS}/dist/js/ 
		${BUILD_ROOT}/js/bootstrap
		SYMBOLIC
	)
endif()

if(NOT (IS_SYMLINK ${BUILD_ROOT}/css/bootstrap))
	file(
		CREATE_LINK 
		${BOOTSTRAP_ABS}/dist/css/ 
		${BUILD_ROOT}/css/bootstrap
		SYMBOLIC
	)
endif()

get_filename_component(
	CROPPIER_ABS
	"./Croppie/"
	ABSOLUTE
)

if(NOT (IS_SYMLINK ${BUILD_ROOT}/js/Croppie))
	file(
		CREATE_LINK 
		${CROPPIER_ABS}
		${BUILD_ROOT}/js/Croppie
		SYMBOLIC
	)
endif()

if(NOT (IS_SYMLINK ${BUILD_ROOT}/css/Croppie))
	file(
		CREATE_LINK 
		${CROPPIER_ABS}
		${BUILD_ROOT}/css/Croppie
		SYMBOLIC
	)
endif()


get_filename_component(
	MAPBOXGL_ABS
	"./mapbox-gl-js/"
	ABSOLUTE
)

if(NOT (IS_SYMLINK ${BUILD_ROOT}/js/mapbox-gl-js))
	file(
		CREATE_LINK 
		${MAPBOXGL_ABS}
		${BUILD_ROOT}/js/mapbox-gl-js
		SYMBOLIC
	)
endif()

if(NOT (IS_SYMLINK ${BUILD_ROOT}/css/mapbox-gl-js))
	file(
		CREATE_LINK 
		${MAPBOXGL_ABS}
		${BUILD_ROOT}/css/mapbox-gl-js
		SYMBOLIC
	)
endif()



get_filename_component(
	BOOTSTRAPTABLE_ABS
	"./bootstrap-table/"
	ABSOLUTE
)

if(NOT (IS_SYMLINK ${BUILD_ROOT}/js/bootstrap-table))
	file(
		CREATE_LINK 
		${BOOTSTRAPTABLE_ABS}
		${BUILD_ROOT}/js/bootstrap-table
		SYMBOLIC
	)
endif()

if(NOT (IS_SYMLINK ${BUILD_ROOT}/css/bootstrap-table))
	file(
		CREATE_LINK 
		${BOOTSTRAPTABLE_ABS}
		${BUILD_ROOT}/css/bootstrap-table
		SYMBOLIC
	)
endif()





get_filename_component(
	FONTAWESOME_ABS
	"./fontawesome/"
	ABSOLUTE
)

if(NOT (IS_SYMLINK ${BUILD_ROOT}/js/fontawesome))
	file(
		CREATE_LINK 
		${FONTAWESOME_ABS}
		${BUILD_ROOT}/js/fontawesome
		SYMBOLIC
	)
endif()

if(NOT (IS_SYMLINK ${BUILD_ROOT}/css/fontawesome))
	file(
		CREATE_LINK 
		${FONTAWESOME_ABS}
		${BUILD_ROOT}/css/fontawesome
		SYMBOLIC
	)
endif()