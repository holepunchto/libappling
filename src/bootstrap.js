const onerror = (err) => {
  Appling.error(err.stack)

  Bare.exit(1) // Cleanly exit and return to C
}

Bare.on('uncaughtException', onerror).on('unhandledRejection', onerror)

const opts = {
  pearKey: Buffer.from(Appling.key),
  pearDir: Appling.directory,
  appLink: `pear://${Appling.link}`,
  useLock: false
}
console.log(opts)

require('pear-distributable-bootstrap')(opts)
