CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
PROJECT (monstruosoft-game)
INCLUDE (FindPkgConfig)

OPTION (WANT_DEBUG "Build the project using debugging code" OFF)

SET (BASE_DIRECTORY .)
SET (SOURCE_DIR ${BASE_DIRECTORY}/src)
SET (CMAKE_C_FLAGS "-std=gnu99 -fgnu89-inline -g")
PKG_CHECK_MODULES (ALLEGRO5 allegro-5 allegro_image-5 allegro_font-5 allegro_primitives-5 allegro_color-5 allegro_ttf-5 allegro_memfile-5)

IF (WANT_DEBUG)
	ADD_DEFINITIONS(-DDEBUG)
ENDIF (WANT_DEBUG)

INCLUDE_DIRECTORIES (${ALLEGRO5_INCLUDE_DIRS} ${BASE_DIRECTORY}/include)
LINK_DIRECTORIES (${ALLEGRO5_LIBRARY_DIRS})

ADD_EXECUTABLE (main ${SOURCE_DIR}/main.c ${SOURCE_DIR}/monstrominas.c ${SOURCE_DIR}/support.c)
TARGET_LINK_LIBRARIES(main ${ALLEGRO5_LIBRARIES} -lm)
