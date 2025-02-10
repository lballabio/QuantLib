#!/bin/bash

SCRIPT_DIR=$(readlink -f "$0" || realpath "$0")

BASEDIR="$(dirname "$SCRIPT_DIR")"

echo "Basedir: $BASEDIR"