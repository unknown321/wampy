# Wampy server

Provides data from Walkman application to end user.

## Build

```shell
make build
```

### Post-build dev cycle

```shell
make fast push
```

After pushing you must restart application on device:

```shell
adb shell hagodaemons.sh restart
```