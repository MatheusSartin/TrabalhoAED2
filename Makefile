# Variaveis
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = programa
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
.PHONY: clear
clear:
	del /Q /F $(OBJS) $(TARGET).exe