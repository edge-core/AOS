# SPDX-License-Identifier: GPL-2.0
#
# Makefile for multiplexer I2C chip drivers.

obj-$(CONFIG_I2C_ARB_GPIO_CHALLENGE)	+= i2c-arb-gpio-challenge.o

obj-$(CONFIG_I2C_DEMUX_PINCTRL)		+= i2c-demux-pinctrl.o

obj-$(CONFIG_I2C_MUX_GPIO)	+= i2c-mux-gpio.o
obj-$(CONFIG_I2C_MUX_GPMUX)	+= i2c-mux-gpmux.o
obj-$(CONFIG_I2C_MUX_LTC4306)	+= i2c-mux-ltc4306.o
obj-$(CONFIG_I2C_MUX_MLXCPLD)	+= i2c-mux-mlxcpld.o
obj-$(CONFIG_I2C_MUX_PCA9541)	+= i2c-mux-pca9541.o
obj-$(CONFIG_I2C_MUX_PCA954x)	+= i2c-mux-pca954x.o
obj-$(CONFIG_I2C_MUX_PINCTRL)	+= i2c-mux-pinctrl.o
obj-$(CONFIG_I2C_MUX_REG)	+= i2c-mux-reg.o

obj-$(CONFIG_I2C_MUX_ACCTON_AS5712_54x_CPLD) += i2c-mux-accton_as5712_54x_cpld.o
obj-$(CONFIG_I2C_MUX_ACCTON_AS6712_32x_CPLD) += i2c-mux-accton_as6712_32x_cpld.o
obj-$(CONFIG_I2C_MUX_ACCTON_AS5812_54x_CPLD) += i2c-mux-accton_as5812_54x_cpld.o
obj-$(CONFIG_I2C_MUX_ACCTON_AS6812_32x_CPLD) += i2c-mux-accton_as6812_32x_cpld.o

ccflags-$(CONFIG_I2C_DEBUG_BUS) := -DDEBUG
