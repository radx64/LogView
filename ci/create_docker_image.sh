#!/bin/sh
set -ev
cd $(dirname $0)
sudo docker build -t logview:1 .
