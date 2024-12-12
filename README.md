# omp-logger

A logging component for open.mp and open.mp components.

## If you're a normal open.mp developer
You just need to download the component on the releases section and put the `omp-logger.dll` (or `.so` for linux) to the `components` folder of your project.

Example usage:
```c
#include <open.mp>
// if you want to modify the default log level:
// #define #define DEFAULT_OMP_LOG_LEVEL       (Info | Warning | Error)
#include <omp-logger>

main()
{
    new Logger:logger = Logger_Create("my-logger", 0x00FF00);

    Logger_Log(logger, Info, "Hello World");
    Logger_Log(logger, Warning, "Hello World");
    Logger_Log(logger, Error, "Hello World");
    // these will not be printed as our default log level are Info, Warning, and Error
    Logger_Log(logger, Fatal, "Hello World");
    Logger_Log(logger, Debug, "Hello World");

    // You can also do this:
    Logger_Info(logger, "Hello World");
    Logger_Warning(logger, "Hello World");
    Logger_Error(logger, "Hello World");
    // these will not be printed as our default log level are Info, Warning, and Error
    Logger_Fatal(logger, "Hello World");
    Logger_Debug(logger, "Hello World");
}
```

## If you're a component developer
You can check this [repository](https://github.com/Tiaansu/greet-component) for the example on how to use this component to another component.

## Configurations
- `logger.log_level_capitalized`: whether print the log level name in uppercase or capitalize format. (default: `false`)
- `logger.display_source`: If set to `true`, the log printed in the file will have source. (default: `true`)
- `logger.enable_source_for_all`: If set to `true`, all log levels printed in the file will have source (file:line), otherwise only `Warning`, `Error`, and `Fatal` will have it. (default: `false`)
- `logger.timestamp_format`: The timestamp format for the log. (default: `%Y-%m-%dT%H:%M:%S%z`)
- `logger.log_directory`: The directory/path where the log files will be created. (default: `logs`)
- `logger.colors.debug`: The color for `debug` log level. (default: `0xADD8E6`)
- `logger.colors.info`: The color for `info` log level. (default: `0x90EE90`)
- `logger.colors.warning`: The color for `warning` log level. (default: `0xFFD700`)
- `logger.colors.error`: The color for `error` log level. (default: `0xFFB266`)
- `logger.colors.fatal`: The color for `fatal` log level. (default: `0xFF7F7F`)

## Thanks to
- [Amir's omp-node](https://github.com/AmyrAhmady/omp-node) (I copied it's `include` style so other components can use this component)
- [maddinatOr's samp-log-core](https://github.com/maddinat0r/samp-log-core) (most of the code were from this project)
