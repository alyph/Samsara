SHELL=cmd
BIN_DIR := bin\win64
BUILD_DIR := build\win64
SRC_DIR := source

# ALL_SRC := $(wildcard $(SRC_DIR)/**/*.cpp) external/glew/src/glew.c
TEST_SRC := $(wildcard $(SRC_DIR)/test/*.cpp)
# ENGINE_SRC := $(filter-out $(TEST_SRC), $(ALL_SRC))
# ENGINE_OBJ := $(patsubst $(SRC_DIR)/*/%.cpp, $(BUILD_DIR)/%.obj, $(ENGINE_SRC))
ENGINE_SRC := $(wildcard $(SRC_DIR)/engine/*.cpp) external/glew/src/glew.c
ENGINE_OBJ := $(patsubst %, $(BUILD_DIR)/%.obj, $(basename $(notdir $(ENGINE_SRC))))
GAME_SRC := $(wildcard $(SRC_DIR)/game/*.cpp)

TESTS_EXE := $(TEST_SRC:$(SRC_DIR)/test/%.cpp=$(BIN_DIR)/%.exe)
TESTS := $(TESTS_EXE:$(BIN_DIR)/%.exe=%)
# TABLET_TEST := tablet_test
# TABLET_TEST_EXE := $(BIN_DIR)/$(TABLET_TEST).exe
GAME_EXE := $(BIN_DIR)/game.exe

INCLUDES := /I.\source /I.\external\glew\include /I.\external\stb /I.\external\easy_profiler\include /I.\external\freetype\include
COMPILER_FLAGS := /Od /W3 /WX /Zi /MD /GR- /MP /utf-8 /std:c++latest /D_HAS_EXCEPTIONS=0 /D_CRT_SECURE_NO_WARNINGS=1 /DGLEW_STATIC=1 /DBUILD_WITH_EASY_PROFILER=1 $(INCLUDES)
LINKER_FLAGS := /INCREMENTAL:NO /LIBPATH:external\glfw\bin\win64 /LIBPATH:external\easy_profiler\bin /LIBPATH:external\freetype\bin\win64
LIBS := opengl32.lib User32.lib Gdi32.lib easy_profiler.lib freetype.lib

.PHONY: all engine clean makedirs copyext game $(TESTS)


all: game $(TESTS)

game: $(GAME_EXE)

$(TESTS):%:$(BIN_DIR)/%.exe


$(TESTS_EXE):$(BIN_DIR)/%.exe:$(SRC_DIR)/test/%.cpp engine
	cl $(COMPILER_FLAGS) /Fo$(BUILD_DIR)\\ /Fd$(BUILD_DIR)\\ $< $(ENGINE_OBJ) /link $(LINKER_FLAGS) $(LIBS) /OUT:$@

$(GAME_EXE): $(GAME_SRC) engine
	cl $(COMPILER_FLAGS) /Fo$(BUILD_DIR)\\ /Fd$(BUILD_DIR)\\ $(GAME_SRC) $(ENGINE_OBJ) /link $(LINKER_FLAGS) $(LIBS) /OUT:$@

# $(TABLET_TEST_EXE): $(SRC_DIR)/test/$(TABLET_TEST).cpp engine
# 	cl $(COMPILER_FLAGS) /Fo$(BUILD_DIR)\\ /Fd$(BUILD_DIR)\\ $< $(ENGINE_OBJ) /link $(LINKER_FLAGS) $(LIBS) /OUT:$@

# $(ENGINE_OBJ): $(BUILD_DIR)/%.obj : $(SRC_DIR)/%.cpp
# $(ENGINE_OBJ): $(ENGINE_SRC)
engine: makedirs copyext $(ENGINE_OBJ) $(ENGINE_SRC)
	cl $(COMPILER_FLAGS) /c $(ENGINE_SRC) /Fo$(BUILD_DIR)\\ /Fd$(BUILD_DIR)\\

$(ENGINE_OBJ):	

clean:
	@del $(BIN_DIR)\*.exe >nul 2>&1
	@del $(BIN_DIR)\*.pdb >nul 2>&1
	@del $(BUILD_DIR)\*.obj >nul 2>&1
	@del $(BUILD_DIR)\*.pdb >nul 2>&1

makedirs:
	@if not exist $(BIN_DIR) (md "$(BIN_DIR)")
	@if not exist $(BUILD_DIR) (md "$(BUILD_DIR)")

copyext: makedirs
	@if not exist $(BIN_DIR)\freetype.dll (copy "external\freetype\bin\win64\freetype.dll" "$(BIN_DIR)\freetype.dll")
	@if not exist $(BIN_DIR)\freetype.pdb (copy "external\freetype\bin\win64\freetype.pdb" "$(BIN_DIR)\freetype.pdb")
	@if not exist $(BIN_DIR)\easy_profiler.dll (copy "external\easy_profiler\bin\easy_profiler.dll" "$(BIN_DIR)\easy_profiler.dll")


