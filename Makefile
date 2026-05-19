#Variaveis
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = programa
OBJS = main.o parser.o

#Target (Linkar os objetos no programa final)
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

#Regras de compilacao
main.o: main.cpp parser.h
	$(CXX) $(CXXFLAGS) -c main.cpp

parser.o: parser.cpp parser.h
	$(CXX) $(CXXFLAGS) -c parser.cpp

#Limpeza
.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)
