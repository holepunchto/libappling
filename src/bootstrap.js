/* global Bare, Appling */

const onerror = (err) => {
  Appling.error(err.stack)

  Bare.exit(1) // Cleanly exit and return to C
}

Bare.on('uncaughtException', onerror).on('unhandledRejection', onerror)

require('pear-updater-bootstrap')(
  Buffer.from(Appling.dkey),
  Appling.directory,
  { lock: false }
)
