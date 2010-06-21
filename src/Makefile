# Compiler and compiler options
# -------------------------------------------------------------
CXX=g++
CXXFLAGS=-Wall -O2
RM=rm

TARGET=fen2eps

# Makefile options
# -------------------------------------------------------------
all: $(TARGET)
	

$(TARGET): $(TARGET).cpp
	$(CXX) $(CXXFLAGS) $(TARGET).cpp -o $(TARGET)

clean:
	$(RM) -f $(TARGET)

