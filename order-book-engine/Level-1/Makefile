CXX = g++ #mentioning complier
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic #mentioning flags with std, warning all, and other, pedantic meaning 

TARGET = Orderbook #name of final executable

SOURCE = main.cpp Orderbook.cpp # source file

#Object file
OBJECTS = $(SOURCE:.cpp=.o) # list of files complier creates before linking

#build the executable
all: $(TARGET) 

# The below code say: To run target object file is needed and then it link the object file with name TARGET into file executable file
$(TARGET): $(OBJECTS) 
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) 
	@echo "Build Complete : ./$(TARGET)"


# till this point the executable file name (Orderbook ) is created 

# Now we will mention what 
# The meaning is that: object file a cpp file with same would be there so make it
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ 

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET)
	@echo "Build artificats cleaned"

# Rebuild everything
rebuild: clean all


.PHONY: all run clean rebuild