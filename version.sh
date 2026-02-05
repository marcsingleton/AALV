#!/bin/sh

if [ -d .git ] && command -v git > /dev/null 2>&1; then
    VERSION=$(git describe --tags --always --match 'v[0-9]*.[0-9]*.[0-9]*')
    VERSION=${VERSION#v}  # Strip initial v
else
    VERSION=?.?.?
fi

echo $VERSION
