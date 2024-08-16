function _list_commands
    set -l CMD_LIST (string replace -r 'aliasme[\s]+[\w]+\s+' '' (commandline))
    set -l CMD_LIST (string replace -a -r '[\s]+' '/' "$CMD_LIST")

    if test -d "$HOME/.local/share/aliasme/$CMD_LIST" 
        find "$HOME/.local/share/aliasme/$CMD_LIST" -maxdepth 1 -type d | sed 's:.*/::g'
    end
end

set -l _SUBCOMMANDS "add edit rm run show"

complete -f -c aliasme \
    -n "not __fish_seen_subcommand_from $_SUBCOMMANDS" \
    -a "add" \
    -d "add new command"

complete -f -c aliasme \
    -n "not __fish_seen_subcommand_from $_SUBCOMMANDS" \
    -a "edit" \
    -d "edit command"

complete -f -c aliasme \
    -n "not __fish_seen_subcommand_from $_SUBCOMMANDS" \
    -a "rm" \
    -d "rm command"

complete -f -c aliasme \
    -n "not __fish_seen_subcommand_from $_SUBCOMMANDS" \
    -a "run" \
    -d "run command"

complete -f -c aliasme \
    -n "not __fish_seen_subcommand_from $_SUBCOMMANDS" \
    -a "generate" \
    -d "generate completions"

complete -f -c aliasme \
    -n "not __fish_seen_subcommand_from $_SUBCOMMANDS" \
    -a "show" \
    -d "show command content"

complete -f -c aliasme \
    -n "__fish_seen_subcommand_from $_SUBCOMMANDS" \
    -a "(_list_commands)"
