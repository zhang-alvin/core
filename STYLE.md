# Unofficial SCOREC style guide #

The really important rules are:

1. Be clean. If you don't know what clean is, read code until you do.
2. Be consistent. Match the style of surrounding code, unless its not clean.
3. Be concise. Reduce characters and lines unless thats not consistent or clean.

## Files ##

Header files use extension `.h`, C++ source has extension `.cc`,
and C source has extension `.c`.

Header files are protected by an include guard, which should be
the name of the file converted to preprocessor symbol style
(see Symbols).
Remember, no leading or trailing underscores on the include guard
symbol.

Files should not exceed 1000 lines,
and lines should not exceed 80 characters.

## Symbols ##

Global symbols are a concatenation of English words.
Ideally, spell the words out fully.
If the word is too long, find a short synonym.
If that doesn't work, find a good abbreviation with some vowels.
For example, `ctor` is better for `constructor` than `cnstrctr`.

Words are concatenated either with nothing between them, in which
case we use camel case to distinguish between words: `createBetterMesh`,
or using underscores, in which case all letters are upper or lower case:
`create_better_mesh`.

All preprocessor symbols and enums should be all upper case,
which forces them to use
underscores to distinguish words: `CREATE_BETTER_MESH`.
Leading or trailing underscores on preprocessor symbols
are reserved for system libraries,
and mortals are forbidden from using these in their symbols:
`__TURN_BACK_HUMAN__`.

Some top-level APIs use a mix of camel case and underscores which really
distinguishes the words: `Create_Better_Mesh`.
Thats fine for high-level APIs, but its a bit too verbose for internal symbols.

Choose one of the above styles, do not mix: `thisIs_Reallynot_good`.

Symbols which are local to a function or structure
don't have to be fully spelled-out words,
since functions and structures should be small,
then there are few symbols to think about,
so things like `n` and `i` are perfectly understandable.

## Braces ##

Either

	for (i = 0; i < 3; ++i) {
	  b[i] = a[i];
	}

or

	for (i = 0; i < 3; ++i)
	{
	  b[i] = a[i];
	}

Anything else, like the GNU style, is too weird.

## Whitespace ##

Currently, most of the code uses 2 spaces for indentation.
Consistency requires that this is followed in existing code,
but new projects may choose a different self-consistent style.

Setup your editor to automate indentation with the style of surrounding code.

Do not mix tabs and spaces, be consistent.

Do not leave trailing whitespace.

### Controlling Whitespace ###
 
If you would like to render the leading whitespaces differently in your text editor or IDE without Git seeing a diff on every line when a file is committed you can setup up Git configuration files to automatically convert between your local settings and the Git repository you are commiting your code to. Commonly, if you use tabs for indentation rather than spaces you would add [the following lines](https://stackoverflow.com/a/14759913/27639153) to your Github configuration files,

```
[filter "tabspace"]
	smudge = gunexpand --tabs=2 --first-only
	clean = gexpand --tabs=2 --initial
```

Here the the two spaces turn to a tab when fetching the code to your local repository and a tab is turned into two spaces when sending it back to Github repository. This can be done globally in your `.gitconfig` file or locally per repository. Since it is hardcoded and Git repositories may differ in style, it is recommended to set it per particular repository rather than globally. Note that this converts a tab as a single special character, meaning that in your own text editor/IDE you can set any number of spaces per tab.

If the above doesn't work for some reason you can convert tabs to spaces in some automated way using your text editor/IDE. In case of `vim`  [the conversion](http://vim.wikia.com/wiki/Converting_tabs_to_spaces) is done by first setting to expanding tabs to spaces,
```
:set expandtab 
```
and then turning all existing tabs to the preset amount of spaces
```
:retab
```
Setting the number of spaces per tab can be done in the `.vimrc` file usually found in ones home directory (or created if not found) or by typing it manually in a specific file. In both cases the command is 
```
set tabstop=4
```
except in the editor one adds a `:` in front. You can view tabs and other special characters by typing `:se list`.

If you need to search for files that have tabs and need manual conversion, on Linux and OsX systems you can do so very efficiently using [`grep`](https://askubuntu.com/questions/53071/how-to-grep-for-tabs-without-using-literal-tabs-and-why-does-t-not-work). A working combination on Linux is
```
grep -P "\t" -r --include="*.cc" .
```
which means recursively (`-r`) find the ocurence of special character tab (`-P “\t”`) in all files ending with `.cc` (`—include=“*.cc”`) starting from this directory (`.`). 

Finally, github diff files will by default show whitespace changes including all these manual tab to space conversions if they happen. While it is necessary to view them, in order to filter them out and see the actual code changes one can add `?w=1` [at the end of the current github *diff url*](https://github.com/tiimgreen/github-cheat-sheet#ignore-whitespace). Literally, if the url is for example https://github.com/diff-1 change the url in the url tab to https://github.com/diff-1?w=1. This seems to be also available in Diff settings menu as part of the web GUI at least on some web browsers.

## Comments ##

Reserve the use of comments for describing functionality, why some code exists, etc. .

Do not leave commented code in the develop or master branches.  

## Projects ##

Each project should have a small abbreviation of its name that
is useful as a prefix in several cases.
This prefix is used to distinguish
the files, functions, and types of one project from another.
Each project also gets a top-level subdirectory named after this
prefix.

All files belonging to a project should be named starting with
the project prefix.
This is useful for debugging reports which give source file names
and line numbers as well as for include files which usually get
mixed into the same directory.
A name like `util.h` could have come from any number of local
or third-party libraries, and causes havoc in large applications.
If its called `gmiUtil.h`, then at least it must be from GMI.

## C++ ##

Types, including classes, are camel case starting with a capital letter.
Functions and variables should be camel case starting with a lower case letter.
Functions preferrably start with a verb.

A project should be contained within a namespace, that is, all its public
symbols should be in a namespace that is the project prefix.

	namespace apf {

	class SomeType;

	void doSomethingGreat(int a, int b);

	}

## C ##

public symbols should be lowercase with underscores,
starting with the project prefix.

	struct gmi_model;

	double gmi_compute(struct gmi_model* m);

## Assertions ##

Four macros have been defined:
```
PCU_ALWAYS_ASSERT(cond)
PCU_ALWAYS_ASSERT_VERBOSE(cond, msg)
PCU_DEBUG_ASSERT(cond)
PCU_DEBUG_ASSERT_VERBOSE(cond, msg)
```
All `assert`'s have been replaced with `PCU_ALWAYS_ASSERT()`.  As the name suggests, this assert is never disabled. Developers can now implement or revise asserts that should get disabled with `-DNDEBUG=ON` using the `PCU_DEBUG*` methods. The rationale for this is to minimize the impact of making "fast code" choices in the code base, so that the onus of making these decisions is placed on the developers who must carefully choose in which instances they should occur. This is in contrast to going through the entire code base and removing warnings=errors that are associated with unused variables when `-DNDEBUG=ON.`

tldr; asserts in the code do some pretty important stuff. you now have to choose where you want "fast code" to get executed with `-DNDEBUG=ON`

Additionally, the `*ASSERT_VERBOSE` options now provide developers the ability to print a user-friendly error message along with the assertion failure so that user's don't have to decipher cryptic messages like:
`r == n failed`
