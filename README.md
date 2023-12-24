# Memrescue
Memrescue keeps your Linux desktop fast and responsive by proactively killing programs using too much memory and clearing swap/caches as needed.

## Installation
```sh
$ make
$ sudo make install
```

## Set up with systemd
```sh
$ sudo make start
```

## Update systemd after recompile
```sh
$ sudo make update
```

## Stop systemd and uninstall
```sh
$ sudo make remove
```

## Command usage
```sh
$ sudo memrescue
```

## Configuring Memrescue
You can edit any of the options in `config.hpp`. However, you will need to compile Memrescue again.
