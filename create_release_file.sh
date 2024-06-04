#!/bin/bash
rev=$(./version.sh | sed -n 2p)

zip -9 q2pro_jump_r$rev.zip q2pro_jump.exe

