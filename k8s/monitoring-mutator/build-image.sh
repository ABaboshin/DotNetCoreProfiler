#!/bin/bash

VERSION=20210331-1

docker build -t ababoshin/monitoring-mutator:$VERSION .
docker push ababoshin/monitoring-mutator:$VERSION
