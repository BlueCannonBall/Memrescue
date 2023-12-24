# Memrescue
Memrescue keeps your Linux desktop fast and responsive by proactively killing programs using too much memory and clearing swap/caches as needed.

## Installation
You can install Memrescue using make:
```sh
$ make
$ sudo make install
```
Then, you can make it start on boot with systemd:
```sh
$ systemctl enable memrescue # Add --now to also start it immediately
```

## Uninstallation
You can uninstall Memrescue using make:
```sh
$ sudo make uninstall
```
This will also disable and stop `memrescue.service`.

## Updating
If you enabled `memrescue.service`, update with `sudo make update` to recompile Memrescue and restart the service. Otherwise, use `sudo make install`.

## Usage (without systemd)
```sh
$ sudo memrescue
```

## Configuring Memrescue
You can edit any of the options in `config.hpp`. However, you will need to compile Memrescue again, as if you are updating it.
