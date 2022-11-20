### AliasMe

To ease the management for code snippets add aliases for them to the `$HOME/bin` directory. Instead of launching them directly, create a simple mnemonic, for example: "publish on github" with some snippet:

```bash
$ git push origin main
$ echo done
```

#### Installation

```bash
make install
```

#### Usage

```bash
$ aliasme add builder run go    # create snippets for each of subcommands
$ aliasme rm builder run        # works recursively
$ aliasme edit builder run go
```
