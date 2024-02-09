/* global Bare */
const [key, directory] = Bare.argv

const onerror = (err) => {
  console.error(err)

  Bare.exit(1) // Cleanly exit and return to C
}

Bare
  .on('uncaughtException', onerror)
  .on('unhandledRejection', onerror)

require('pear-updater-bootstrap')(key, directory, { lock: false })
