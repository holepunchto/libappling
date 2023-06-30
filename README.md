# libappling

Low-level plumbing for Pear application shells. Application shells, referred to as _applings_, are small, native applications that bundle the public key of their associated Pear application in addition to a copy of the Pear platform. At a glance, an appling goes through the following steps to launch its associated application:

1. **Resolve:** Determine the path to the current platform installation, if available.
2. **Bootstrap:** If no platform is available, unpack and install the bundled platform.
3. **Launch:** With the platform either already available or just installed, launch the platform with the application.

Once created and distributed, an appling never needs to be recreated and redistributed as the Pear platform and application are themselves fully self updating.

## Usage

See [`example/`](example).

## API

See [`include/appling.h`](include/appling.h) for the public API.

## License

Apache-2.0
