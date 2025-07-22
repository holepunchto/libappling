const updater = require('pear-updater-bootstrap')
const distributable = require('pear-distributable-bootstrap')

const onerror = (err) => {
  Appling.error(err.stack)

  Bare.exit(1) // Cleanly exit and return to C
}

Bare.on('uncaughtException', onerror).on('unhandledRejection', onerror)

async function bootstrap() {
  await updater(Buffer.from(Appling.key), Appling.directory, {
    lock: false
  })

  await distributable({
    pearKey: Buffer.from(Appling.key),
    pearDir: Appling.directory,
    appLink: 'pear://yx8yxsegdyow6kawrruhsx5k7scampx5kww1d7py5ebjf1gm5sjo'
  })
}

bootstrap()
