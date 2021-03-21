#!/bin/bash

cd ../..
git archive --prefix=wifiheatmap-0.1-alpha/ -o installer/rpm/wifiheatmap-0.1-alpha.3.tar.gz master .
cd installer/rpm
fedpkg --release f32 mockbuild
fedpkg --release f33 mockbuild
fedpkg --release f34 mockbuild
fedpkg --release rawhide mockbuild

