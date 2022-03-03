#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

PROJECT_NAME := dsp
EXTRA_COMPONENT_DIRS += \
  $(abspath ./components/dsp-playground/)

include $(IDF_PATH)/make/project.mk
