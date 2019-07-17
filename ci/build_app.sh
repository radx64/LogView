#!/bin/sh
set -ev
cd $(dirname $0)/..

rm -rf ./build_workspace
mkdir ./build_workspace
cd ./build_workspace
qmake ../LogView.pro
make -l10

