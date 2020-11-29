#!/bin/bash

dotnet app.dll > result.txt
diff --strip-trailing-cr result.txt expected.txt || exit -1
