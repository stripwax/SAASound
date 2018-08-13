AUTOMAKE_OPTIONS = foreign

GENERAL_SRCS = SAAAmp.cpp SAAEnv.cpp SAAFreq.cpp SAANoise.cpp \
		SAASoundImplementation.cpp SAASndC.cpp

lib_LTLIBRARIES = libSAASound.la

libSAASound_la_SOURCES = $(GENERAL_SRCS)
libSAASound_la_LDFLAGS = 	\
        -release $(LT_RELEASE)	\
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
