const onerror = (err) => {
  Appling.error(err.stack)

  Bare.exit(1) // Cleanly exit and return to C
}

Bare.on('uncaughtException', onerror).on('unhandledRejection', onerror)

require('pear-distributable-bootstrap')({
  pearKey: Buffer.from(Appling.key),
  pearDir: Appling.directory,
  appLink: 'pear://yx8yxsegdyow6kawrruhsx5k7scampx5kww1d7py5ebjf1gm5sjo' // TEMP: hardcoded keet-next
})
