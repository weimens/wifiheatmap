#!/bin/bash
set -e

[[ -d wifiheatmap-0.1-alpha ]] && rm -r wifiheatmap-0.1-alpha
git clone ../.. wifiheatmap-0.1-alpha
cd wifiheatmap-0.1-alpha
git submodule init
git submodule update --depth 1
rm -rf ./.git
rm -rf ./foreign/delaunator-cpp/gtest
rm -rf ./foreign/delaunator-cpp/.git
rm -rf ./foreign/delaunator-cpp/test-files
cd ..
tar -cavf wifiheatmap-0.1-alpha.3.tar.gz ./wifiheatmap-0.1-alpha/
fedpkg --release f33 mockbuild
fedpkg --release f34 mockbuild
#fedpkg --release rawhide mockbuild

