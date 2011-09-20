#!/bin/zsh
alias nepomukcmd="sopranocmd --socket `kde4-config --path socket`nepomuk-socket --model main --nrl"
for res in `nepomukcmd --foo query 'select ?r { ?r a kext:Activity . }'`; nepomukcmd rm $res
