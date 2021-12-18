Listerene is a simple ncurses application which sorts plaintext lists of passwords into unified lists according to various rulesets, strips strings with unusable characters, and removes duplicates.  This allows a user to create several different formats of password lists in order to use only passwords relevant to the tools/systems you are currently using, and can also format many improperly-formatted lists into lists which your brute-forcer can read.


Dependendencies:

libncursesw5-dev
libncurses5-dev



To install, type the following commands from the source dir:

make

sudo make install



There is also a clean option with:

make clean



To run the program after installing, type:

listerene
