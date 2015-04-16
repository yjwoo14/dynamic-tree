CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

project (LINK_CUT_TREE)
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS} -g -ftest-coverage -fprofile-arcs")
set(SRC_DIR ${CMAKE_SOURCE_DIR})

set(SPLAY_TREE
	${SRC_DIR}/splay_tree.h
	${SRC_DIR}/statistics.h
	${SRC_DIR}/navigator.h
)

set(LINK_CUT_TREE
	${SPLAY_TREE}
	${SRC_DIR}/link_cut_tree.h
)

add_executable(splay_tree_test splay_tree_test_unit.cpp ${SPLAY_TREE})
add_executable(link_cut_tree_test link_cut_tree_test_unit.cpp ${LINK_CUT_TREE})
add_executable(lct_evert_test link_cut_tree_evert_test_unit.cpp ${LINK_CUT_TREE})
