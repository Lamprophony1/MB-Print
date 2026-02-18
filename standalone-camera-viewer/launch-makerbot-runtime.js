'use strict';

const path = require('path');
const fs = require('fs');
const cp = require('child_process');

const defaultExe = 'C:\\Program Files\\MakerBot\\MakerBotPrint\\makerbot-print.exe';
const makerbotExe = process.env.MAKERBOT_EXE || defaultExe;
const serverPath = path.resolve(__dirname, 'server.js');

if (!fs.existsSync(makerbotExe)) {
  console.error('[standalone-camera-viewer] No se encontró makerbot-print.exe en:');
  console.error(`  ${makerbotExe}`);
  console.error('Seteá MAKERBOT_EXE con la ruta correcta y reintentá.');
  process.exit(1);
}

const env = Object.assign({}, process.env, {
  ELECTRON_RUN_AS_NODE: '1'
});

const child = cp.spawn(makerbotExe, [serverPath], {
  env,
  stdio: 'inherit'
});

child.on('exit', code => process.exit(code || 0));
child.on('error', err => {
  console.error(err);
  process.exit(1);
});
