# Advent of Code 2023

My solutions for AOC2023!

Folder structure: `(?'day'\d+)(\.(?'part'\d+))?`

# TOC
- [Advent of Code 2023](#advent-of-code-2023)
- [TOC](#toc)
- [Running](#running)
- [Takeaways](#takeaways)
- [Dependencies](#dependencies)
	- [Arch Linux](#arch-linux)
	- [Other linux/unix distros:](#other-linuxunix-distros)
	- [Windows](#windows)
	- [Other OS's](#other-oss)

# Running

Each folder has a makefile, just run `make` and then pass the input to the executable like so:

```
$ ./main input.txt
```

# Takeaways



# Dependencies

**Some** solutions have interactive/visual helpers that use [raylib](https://github.com/raysan5/raylib) to work. 

Here is what you need:

## Arch Linux

> We use yay here to install all dependencies of raylib and raylib from the aur.

```console
$ sudo yay -S alsa-lib mesa libx11 libxrandr libxi libxcursor libxinerama raylib
```

## Other linux/unix distros:

Needs some figuring it out!

[Raylib - Working on Gnu/Linux](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux)

## Windows

Good luck!

[Raylib - Working on Windows](https://github.com/raysan5/raylib/wiki/Working-on-Windows)

## Other OS's

MacOs is terrible for the environment!

Checkout [raylib repo](https://github.com/raysan5/raylib)!