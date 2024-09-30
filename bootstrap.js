console.log('Bootstrapping')

require('pear-updater-bootstrap')('6b8374f1c0809ed23cfc371e87896c8d3bb593f2451d4d8de895d628941818dc', 'bootstrap').then(() => {
  console.log('Done!')
})
