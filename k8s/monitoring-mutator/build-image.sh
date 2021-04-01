#!/bin/bash

VERSION=linux-x64-20210401

docker build -t ababoshin/monitoring-mutator:$VERSION .
docker push ababoshin/monitoring-mutator:$VERSION
