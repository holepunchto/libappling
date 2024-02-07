/* global Bare */
const [key, directory] = Bare.argv

const onerror = (err) => {
  console.error(err)

  Bare.exit(1) // Cleanly exit and return to C
}

Bare
  .on('uncaughtException', onerror)
  .on('unhandledRejection', onerror)

const process = require('bare-process')
const path = require('bare-path')

global.process = process

const Updater = require('pear-updater')
const Hyperdrive = require('hyperdrive')
const Hyperswarm = require('hyperswarm')
const Corestore = require('corestore')

const swarm = new Hyperswarm()

const drive = new Hyperdrive(new Corestore(path.join(directory, 'corestores/platform')), key)

const updater = new Updater(drive, {
  directory,
  checkout: { key, length: 0, fork: 0 },
  additionalBuiltins: [
    'buffer',
    'child_process',
    'constants',
    'crypto',
    'dns',
    'electron',
    'events',
    'fs',
    'fs/promises',
    'http',
    'https',
    'module',
    'net',
    'os',
    'path',
    'readline',
    'repl',
    'stream',
    'timers',
    'tls',
    'url',
    'util',
    'zlib'
  ]
})

updater.on('update', async () => {
  await swarm.destroy()
})

drive.ready().then(() => {
  swarm
    .on('connection', (stream) => drive.replicate(stream))
    .join(drive.discoveryKey, {
      server: false,
      client: true
    })
})
