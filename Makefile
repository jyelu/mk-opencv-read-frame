CXX := g++ -std=c++11 
CXXFLAGS := -c -O2 `pkg-config --cflags opencv` 
OPENCV := `pkg-config opencv --libs --cflags` 
LIBS := -lpthread $(OPENCV) 
LDFLAGS := $(LIBS) 
DEPFLAGS := -MM

OBJDIR := obj
OBJS := $(addprefix $(OBJDIR)/,main.o )

$(OBJDIR)/%.o : %.cpp | $(OBJDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

all : main

main : $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) 

$(OBJDIR) :
	mkdir $(OBJDIR)

clean :
	rm -rf main $(OBJDIR)

dep:
	$(CXX) $(DEPFLAGS) $(CXXFLAGS) *.cpp

$(OBJDIR)/main.o: main.cpp
