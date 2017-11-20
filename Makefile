TARGET = lidarExample

SRC := src
INCLUDE = -Iinclude 
CC = g++ -std=c++11
CFLAGS = -Wno-deprecated-declarations -fpermissive
LIBS = -pthread -lrt -lsweep
AUX = $(SRC)/*.cpp

all: $(TARGET)

$(TARGET): $(TARGET).cpp $(AUX)
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp $(AUX) $(INCLUDE) $(LIBS) 
	
clean:
	$(RM) $(TARGET)
