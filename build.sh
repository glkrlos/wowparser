#!/bin/bash

./staticdata.sh

cargo build

rm -rf src/staticdata.rs
