Listerene is a simple ncurses application which sorts plaintext lists of passwords into unified lists according to various rulesets, strips strings with unusable characters, and removes duplicates.  This allows a user to create several different formats of password lists in order to use only passwords relevant to the tools/systems you are currently using, and can also format many improperly-formatted lists into lists which your brute-forcer can read.

This code is in a working alpha state and is functional, but needs major overhauls and polishing of the sorting method and a few small bugs before it is ready for a stable release.  It works amazingly for lists of a million passwords or less, but it becomes very slow after the database list grows too large.


Build Dependendencies:

libncursesw5-dev
libncurses5-dev



To install, type the following commands from the source dir:

make

sudo make install



There is also a clean option with:

make clean



To run the program after installing, type:

listerene
