#!/bin/bash

VERSION=20210330-1

docker build -t ababoshin/monitoring-mutator:$VERSION .
docker push ababoshin/monitoring-mutator:$VERSION
