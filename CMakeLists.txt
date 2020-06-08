cmake_minimum_required(VERSION 3.10)

# set the project name
project(GigGizmo)

include_directories(include)
add_subdirectory(3rdParty)

get_filename_component(
	BUILD_ROOT
	"${CMAKE_CURRENT_BINARY_DIR}/"
	ABSOLUTE
)

get_filename_component(
	ASSETS_ABS
	"./assets/"
	ABSOLUTE
)

get_filename_component(
	JS_ABS
	"./js/"
	ABSOLUTE
)

get_filename_component(
	CSS_ABS
	"./css/"
	ABSOLUTE
)

if(NOT (IS_SYMLINK ${BUILD_ROOT}/assets))
	file(
		CREATE_LINK 
		${ASSETS_ABS} 
		${BUILD_ROOT}/assets
		SYMBOLIC
	)
endif()

if(NOT (IS_SYMLINK ${BUILD_ROOT}/js/main))
	file(
		CREATE_LINK 
		${JS_ABS} 
		${BUILD_ROOT}/js/main
		SYMBOLIC
	)
endif()

if(NOT (IS_SYMLINK ${BUILD_ROOT}/css/main))
	file(
		CREATE_LINK 
		${CSS_ABS} 
		${BUILD_ROOT}/css/main
		SYMBOLIC
	)
endif()

FIND_LIBRARY(cryptopp crypto++ /usr/lib)

# add the executable
add_executable(
	GigGizmo
	src/routes/band/create.cpp
	src/routes/band/find.cpp
	src/routes/band/index.cpp
	src/routes/band/list.cpp
	src/routes/band/options.cpp
	src/routes/upload/find.cpp
	src/routes/upload/index.cpp
	src/routes/upload/list.cpp
	src/routes/upload/options.cpp
	src/routes/user/home.cpp
	src/routes/user/login.cpp
	src/routes/user/options.cpp
	src/routes/user/register.cpp
	src/routes/venue/create.cpp
	src/routes/venue/find.cpp
	src/routes/venue/index.cpp
	src/routes/venue/list.cpp
	src/routes/venue/options.cpp
	src/routes/index.cpp
	src/band.cpp
	src/bootstrap.cpp
	src/error.cpp
	src/navbar.cpp
	src/main.cpp
	src/session.cpp
	src/template.cpp
	src/upload.cpp
	src/user.cpp
	src/utility.cpp
	src/venue.cpp
)

if(MSVC)
  target_compile_options(GigGizmo PRIVATE /W4 /WX)
else()
  target_compile_options(GigGizmo PRIVATE -Wall -pedantic)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

target_include_directories(
	GigGizmo
	PUBLIC
		"include"
)

target_link_libraries(
	GigGizmo 
	PRIVATE
		gold::web
		cryptopp
)

target_compile_features(
	GigGizmo
	PUBLIC
		cxx_variadic_templates
		cxx_nullptr
		cxx_generic_lambdas
		cxx_lambdas
		cxx_auto_type
		cxx_variable_templates
		cxx_variadic_macros
		cxx_template_template_parameters
		cxx_std_20
)