# libappling

Low-level plumbing for Holepunch application shells. Application shells, referred to as _applings_, are small, native applications that bundle the public key of their associated Holepunch application in addition to a copy of the Holepunch platform. At a glance, an appling goes through the following steps to launch its associated application:

1. **Resolve:** Determine the path to and version of the current platform installation, if available.
2. **Bootstrap:** If either no platform is available or the appling bundles a newer platform version, unpack and install the bundled platform.
3. **Launch:** With the most recent platform either already available or just installed, launch the platform with the application key.

Once created and distributed, an appling only needs to be recreated and redistributed if the application author wishes to bundle a more recent platform version with the appling as the Holepunch platform and application are themselves fully self updating.

## Usage

See [`example/`](example).

## API

See [`include/appling.h`](include/appling.h) for the public API.

## License

ISC
