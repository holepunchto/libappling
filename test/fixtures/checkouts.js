const fs = require('fs')
const path = require('path')
const c = require('compact-encoding')

const checkout = {
  preencode(state, m) {
    c.fixed32.preencode(state, m.key)
    c.uint.preencode(state, m.length)
    c.uint.preencode(state, m.fork)
    c.string.preencode(state, process.platform)
    c.string.preencode(state, process.arch)
  },
  encode(state, m) {
    c.fixed32.encode(state, m.key)
    c.uint.encode(state, m.length)
    c.uint.encode(state, m.fork)
    c.string.encode(state, process.platform)
    c.string.encode(state, process.arch)
  }
}

const key = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'

fs.writeFileSync(
  path.resolve(__dirname, 'platform', 'by-dkey', key, '0', 'checkout'),
  c.encode(checkout, {
    key: Buffer.from(key, 'hex'),
    length: 123,
    fork: 0
  })
)

fs.writeFileSync(
  path.resolve(__dirname, 'platform', 'by-dkey', key, '1', 'checkout'),
  c.encode(checkout, {
    key: Buffer.from(key, 'hex'),
    length: 124,
    fork: 0
  })
)
