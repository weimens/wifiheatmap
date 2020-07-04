#!/bin/bash

cd ../..
git archive --prefix=wifiheatmap-0.1-alpha/ -o installer/rpm/wifiheatmap-0.1-alpha.2.tar.gz rpm .
cd installer/rpm
fedpkg --release f32 mockbuild

