# CH55x Template

## Dependencies install (ArchLinux)
```sh
yay -S z88dk
```
```sh
pacman -S sdcc
```
```sh
cargo install wchisp
```


## Build
CH559はP4.6をLowにしながらPCに接続すると書き込みモードに入れます
```sh
bash ./scripts/build.sh
```

## Flash
```sh
bash ./scripts/flash.sh
```

## Build & Flash
```sh
bash ./scripts/run.sh
```