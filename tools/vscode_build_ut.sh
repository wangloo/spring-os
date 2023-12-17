#!/bin/bash

set -e

make clean-ukern
make kernel-ut
make run