#!/bin/bash

cd app && dotnet build && cd ..
cd interceptor && dotnet publish -c Debug -o release

cd release
~/.dotnet/tools/interception-generator -a interceptor.dll,Interception.Core.dll -p /Users/ababoshin/src/DotNetCoreProfiler/demo/interceptor/release/ -o profiler.json -s ../skip.txt

cd ../..