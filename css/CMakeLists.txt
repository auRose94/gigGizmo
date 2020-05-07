
get_filename_component(
	BASE_STYLE_ABS
	"./baseStyle.css"
	ABSOLUTE
)

if(NOT (IS_SYMLINK ${CMAKE_CURRENT_BINARY_DIR}/baseStyle.css))
	file(
		CREATE_LINK
		${BASE_STYLE_ABS}
		${CMAKE_CURRENT_BINARY_DIR}/baseStyle.css
		SYMBOLIC
	)
endif()

