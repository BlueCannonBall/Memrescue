# Memrescue
Memrescue keeps your Linux desktop fast and responsive by proactively killing programs using too much memory and clearing swap/caches as needed.

## Installation
```sh
$ make
$ sudo make install
```

## Set up with systemctl
```sh
$ sudo make start
```

## Update systemctl after recompile
```sh
$ sudo make update
```

## Stop systemctl and uninstall
```sh
$ sudo make remove
```

## Command usage
```sh
$ sudo memrescue
```

## Configuring Memrescue
You can edit any of the options in `config.hpp`. However, you will need to compile Memrescue again.
