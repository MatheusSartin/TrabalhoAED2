#identificar sistema operacional
OS ?= $(shell echo %OS% 2>/dev/null)
ifeq ($(OS),Windows_NT)
	IS_WINDOWS := 1
else
	UNAME_S := $(shell uname -s 2>/dev/null)
	ifeq ($(UNAME_S),Linux)
		IS_UNIX := 1
	else ifeq ($(UNAME_S),Darwin)  # macOS usa "Darwin"
		IS_UNIX := 1
	# Detectar Windows subsystems (MINGW64, CYGWIN, etc.)
	else ifneq ($(filter MINGW%,$(UNAME_S)),)  # Git Bash/MinGW
		IS_WINDOWS := 1
	else ifneq ($(filter CYGWIN%,$(UNAME_S)),)  # Cygwin
		IS_WINDOWS := 1
	else
		# Assumir unix-like se não identificado
		IS_UNIX := 1
	endif
endif

ifeq ($(IS_WINDOWS),1)
	#comandos do windows
    RM      := del /f /q        # Deletar arquivos 
    RMDIR   := rd /s /q         # Deletar diretorios 
    MKDIR   := mkdir            # Criar diretorios
    CP      := copy /y          # Copiar arquivos 
    EXE_EXT := .exe
else
    # comandos Unix
    RM      := rm -f            # Deletar arquivos 
    RMDIR   := rm -rf           # Deletar diretorios
    MKDIR   := mkdir -p         # Criar diretorios
    CP      := cp               # Copiar arquivos 
    EXE_EXT :=                  
endif

# Variaveis
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = programa$(EXE_EXT)
OBJS = main.o parser.o dijkstra.o

# Target (Linkar os objetos no programa final)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Regras de compilacao
main.o: main.cpp projects.h
	$(CXX) $(CXXFLAGS) -c main.cpp

parser.o: parser.cpp projects.h
	$(CXX) $(CXXFLAGS) -c parser.cpp

dijkstra.o: dijkstra.cpp projects.h
	$(CXX) $(CXXFLAGS) -c dijkstra.cpp

# Limpeza
.PHONY: clean
clean:
	$(RM) $(OBJS) $(TARGET)
