#
# This is a project Makefile. It is assumed the directory this Makefile resides in is a
# project subdirectory.
#

#PROJECT_NAME := projeto_iot_mcc_univali
#include $(IDF_PATH)/make/project.mk

PROJECT_NAME := projeto_iot_mcc_univali
EXTRA_COMPONENT_DIRS := $(HOME)/junio/esp/RTOS_SDK/examples/projeto/esp-idf-lib/components
EXCLUDE_COMPONENTS := max7219 mcp23x17 led_strip max31865 ls7366r max31855
include $(IDF_PATH)/make/project.mk