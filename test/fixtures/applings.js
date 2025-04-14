const fs = require('fs')
const path = require('path')
const c = require('compact-encoding')

let executable

switch (process.platform) {
  case 'darwin':
    executable = 'Example.app/Contents/MacOS/Example'
    break
  case 'linux':
    executable = 'Example.AppDir/usr/bin/example'
    break
  case 'win32':
    executable = 'Example\\Example.exe'
    break
}

const appling = {
  preencode(state, m) {
    c.string.preencode(state, m.path)
    c.string.preencode(state, m.id)
  },
  encode(state, m) {
    c.string.encode(state, m.path)
    c.string.encode(state, m.id)
  }
}

const applings = c.array(appling)

fs.writeFileSync(
  path.resolve(__dirname, 'platform', 'applings'),
  c.encode(
    {
      preencode(state, m) {
        c.uint.preencode(state, 0) // Flags
        applings.preencode(state, m)
      },
      encode(state, m) {
        c.uint.encode(state, 0) // Flags
        applings.encode(state, m)
      }
    },
    [
      {
        path: `test/fixtures/app/${process.platform}-${process.arch}/${executable}`,
        id: 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'
      }
    ]
  )
)
