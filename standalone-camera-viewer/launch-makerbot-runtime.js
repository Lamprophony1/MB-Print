'use strict';

const path = require('path');
const fs = require('fs');
const cp = require('child_process');

const bundledExe = path.resolve(__dirname, 'runtime', 'makerbot-print', 'makerbot-print.exe');
const installedExe = 'C:\\Program Files\\MakerBot\\MakerBotPrint\\makerbot-print.exe';

const makerbotExe = process.env.MAKERBOT_EXE
  || (fs.existsSync(bundledExe) ? bundledExe : installedExe);
const serverPath = path.resolve(__dirname, 'server.js');

if (!fs.existsSync(makerbotExe)) {
  console.error('[standalone-camera-viewer] No se encontró makerbot-print.exe.');
  console.error(`  buscado: ${makerbotExe}`);
  console.error(`  bundled esperado: ${bundledExe}`);
  console.error(`  instalación esperada: ${installedExe}`);
  console.error('Seteá MAKERBOT_EXE o ejecutá npm run vendor:runtime-win para empaquetar runtime local.');
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
