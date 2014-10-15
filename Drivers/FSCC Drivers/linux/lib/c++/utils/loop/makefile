TARGET = loop

SOURCES = loop.cpp
CFC_DIR = ..\..
LIBS = $(CFC_DIR)\cppfscc.lib

all: $(TARGET).exe

$(TARGET).exe:$(SOURCES) $(LIBS)
  cl /EHsc /D_WIN32_WINNT=0x0502 /W4 /MT $(SOURCES) $(LIBS) /I $(CFC_DIR) /I $(CFC_DIR)/src/ /Fe$@
  
clean:
  del *.exe