#!/usr/bin/env bash

####
#### This script will install python 2.7.8 and scons on current folder
####

TOP_DIR=$(cd $(dirname "${BASH_SOURCE:-$0}") && pwd)
SCRIPTS_DIR=$TOP_DIR/Scripts

source $SCRIPTS_DIR/10-build_python.sh && \
source $SCRIPTS_DIR/20-install_setuptools_pip.sh && \
source $SCRIPTS_DIR/30-install_scons.sh
