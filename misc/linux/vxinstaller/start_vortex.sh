#!/bin/bash
export XDG_RUNTIME_DIR="/run/user/$(id -u)"
pkexec "$@"