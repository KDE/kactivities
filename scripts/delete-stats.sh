#!/bin/zsh
alias nepomukcmd="sopranocmd --socket `kde4-config --path socket`nepomuk-socket --model main --nrl"
for res in `nepomukcmd --foo query 'select ?r { ?r a kao:ResourceScoreCache . }'`; nepomukcmd rm $res
for res in `nepomukcmd --foo query 'select ?r { ?r a nuao:DesktopEvent . }'`; nepomukcmd rm $res
