#! /bin/bash

set -eux

# ================ CLONE QT SOURCES ================
git clone git@github.com:qt/qtbase.git

# ================ INSTALLING QT DEPENDENCIES ================
sudo apt-get install -y qt6-base-dev
