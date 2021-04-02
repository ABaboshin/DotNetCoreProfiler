#!/bin/bash

VERSION=linux-x64-20210402

docker build -t ababoshin/monitoring-mutator:$VERSION .
docker push ababoshin/monitoring-mutator:$VERSION
