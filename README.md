## Shale-Services

Shale is a set of services for IRC networks designed for large IRC networks with high
scalability requirements, based on the proven codebase of Atheme 7. To this release
has been added a copy of `modules/protocol/aspircd.c` (based on `chatd.c` from ircd-chatd),
enabling compatibility with aspIRCd.

The source code is only available from Git. Unless you have done so already, this
means you should run `git submodule update --init --recursive`.

## Basic build instructions for the impatient

Whatever you do, make sure you do *not* install shale into the same location as the source.
Shale-Services will default to installing in `$HOME/shale`, so make sure you plan accordingly for this.

If this is not your desire, add a --prefix as appropriate to the `./configure` command line.

    $ git submodule update --init --recursive
    $ ./configure --enable-contrib             **[If you want to enable contrib]**
    $ make
    $ make install

## IRC Support

 * [IRC](irc://irc.aspircd.com/#aspIRCd) irc.aspircd.com #aspIRCd

## links / contact

 * [GitHub](http://www.github.com/aspircd/shale-services)
 * [Website](https://hyphovy.net)
 * [IRC](irc://irc.aspircd.com/#aspIRCd) irc.aspircd.com #aspIRCd
