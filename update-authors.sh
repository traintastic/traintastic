#!/bin/sh
git authors -l --no-email | grep -v "\\[bot\\]" > AUTHORS
