function _list_commands
    set -l CMD_LIST (string replace -r 'aliasme[\s]+[\w]+\s+' '' (commandline))
    set -l CMD_LIST (string replace -a -r '[\s]+' '/' "$CMD_LIST")

    if test -d "$HOME/.local/share/aliasme/$CMD_LIST" 
        find "$HOME/.local/share/aliasme/$CMD_LIST" -maxdepth 1 -type d | sed 's:.*/::g'
    end
end

complete -f -c aliasme \
    -n "not __fish_seen_subcommand_from add edit rm run" \
    -a "add" \
    -d "add new command"

complete -f -c aliasme \
    -n "not __fish_seen_subcommand_from add edit rm run" \
    -a "edit" \
    -d "edit command"

complete -f -c aliasme \
    -n "not __fish_seen_subcommand_from add edit rm run" \
    -a "rm" \
    -d "rm command"

complete -f -c aliasme \
    -n "not __fish_seen_subcommand_from add edit rm run" \
    -a "run" \
    -d "run command"

complete -f -c aliasme \
    -n "__fish_seen_subcommand_from add edit rm run" \
    -a "(_list_commands)"
