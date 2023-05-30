#!/usr/bin/env python3
"""Runs a built binary."""

import subprocess
import sys

sys.exit(subprocess.call([sys.argv[1]] + sys.argv[2:]))
