
# Rehome

Persuade badly-behaved programs not to write their config files in `$HOME`, and
instead obey the [XDG Base Directory Specification][xdg]. This is done by 
altering the value of `HOME` they see when executed.

I found this technique described by Tom Harrison [on his web site][credit].
This is the same idea but implemented in ANSI C.

Another approach is to use [rewritefs][rewritefs], a FUSE filesystem which intercepts file access in `$HOME` and redirects it. That seemed overly complicated to me.

Long term, perhaps `systemd-homed` can start enforcing XDG Base Directory Specification...

## Instructions

 1. Create `~/.local/share/rehome`. This is where the fake home directories 
   will be stored.
 2. Add ~/.local/rehome/bin to the front if your path. This is where you create symlinks to the rehome binary for each program you want to rehome.
 3. Make sure ~/.local/rehome/bin is at the front of your path. (Or at least, ahead of anything you want to rehome.)
 4. Compile `rehome` and put it somewhere; it doesn't need to be in your path.

To compile `rehome`, just

    cc -o rehome rehome.c

Then to rehome a program, for example `ansible`:

    ln -s /path/to/rehome ~/.local/rehome/bin/ansible

Now run `ansible`. Here's what happens:

 1. Your shell searches the (real) `PATH`, and finds `~/.local/rehome/bin/ansible`.
 2. That symlink is resolved to `rehome`, which the shell runs.
 3. The `rehome` executable looks up the target program name, by seeing how it was invoked. (In this example, as `ansible`.)
 4. Next, `rehome` looks up your `PATH` and removes `~/.local/rehome/bin` from
it, to make sure that the symlink to `rehome` won't be found when we try to run
the target program in the final step. (This change to the path only affects the
`rehome` process and child processes, it isn't exported to the parent shell.)
 5. Rehome works out a fake `HOME` directory location, by appending the target
program name to `~/.local/share/rehome`. It creates the directory if necessary,
in this case ~/.local/share/rehome/ansible`, and sets `HOME` to that value, again only for itself and child processes.
 6. Finally, `rehome` uses `execvp` to replace itself with the target program, as found in the adjusted `PATH`.

So when `ansible` runs, it finds `HOME=~/.local/share/rehome/ansible` and writes its data directory there.


[xdg]: https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
[credit]: https://superluserdo.xyz/blog/posts/2020/03/28/fake-home-prison/
[rewritefs]: https://github.com/sloonz/rewritefs
