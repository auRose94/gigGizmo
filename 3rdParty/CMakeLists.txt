
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