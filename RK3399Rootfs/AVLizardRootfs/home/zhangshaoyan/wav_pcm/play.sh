#!/bin/bash
aplay -c 2 -t raw -r 48000 -f S16_LE test.pcm
aplay test.wav
