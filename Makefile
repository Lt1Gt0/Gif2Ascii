#File Directory things (might be overkill idk yet)
INCLUDE = -I$(SRC_DIR)/headers
BUILD_DIR = bin
SRC_DIR = src
OBJ_DIR = obj
REDIRECT_DIR = redirects

#Compiler and linker things
CC = g++
CCFLAGS = -g -Wall -Wextra
CCFLAGS += -Wno-write-strings # figure out how to make a make recipe for suppressed warnings rather than forcing it
LD = ld
LDFLAGS = 

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

#Essential files and groups
OBJNAME = Gif2Ascii 
SRCS = $(call rwildcard, $(SRC_DIR), *.cpp)
OBJ = $(addprefix $(BUILD_DIR)/, $(OBJNAME))
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

all: $(OBJ) 
	@mkdir -p $(@D)
	@mkdir -p $(REDIRECT_DIR)
	@echo ---- Generating $^ ---

$(OBJ): $(OBJS)
	@echo ---- Linking $^ ----
	@mkdir -p $(@D)
	$(CC) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo ---- Compiling $^ ----
	@mkdir -p $(@D)
	$(CC) $(CCFLAGS) $(INCLUDE) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/
	rm -rf $(OBJ_DIR)/
	rm -rf $(REDIRECT_DIR)/