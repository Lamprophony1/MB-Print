'use strict';

const fs = require('fs');
const path = require('path');

const sourceDir = process.env.MAKERBOT_INSTALL_DIR || 'C:\\Program Files\\MakerBot\\MakerBotPrint';
const targetDir = path.resolve(__dirname, '..', 'runtime', 'makerbot-print');

const EXCLUDE_DIRS = new Set([
  'Cache',
  'GPUCache',
  'Code Cache',
  'Crashpad',
  'logs',
  'Dumps',
  'Temp'
]);

function ensureDir(dirPath) {
  fs.mkdirSync(dirPath, { recursive: true });
}

function copyRecursive(src, dest) {
  const stat = fs.statSync(src);

  if (stat.isDirectory()) {
    const base = path.basename(src);
    if (EXCLUDE_DIRS.has(base)) {
      return;
    }

    ensureDir(dest);
    const entries = fs.readdirSync(src);

    entries.forEach(entry => {
      copyRecursive(path.join(src, entry), path.join(dest, entry));
    });
    return;
  }

  ensureDir(path.dirname(dest));
  fs.copyFileSync(src, dest);
}

function main() {
  if (!fs.existsSync(sourceDir)) {
    throw new Error(`MakerBot install dir not found: ${sourceDir}`);
  }

  console.log(`[vendor-runtime] Source: ${sourceDir}`);
  console.log(`[vendor-runtime] Target: ${targetDir}`);

  if (fs.existsSync(targetDir)) {
    console.log('[vendor-runtime] Cleaning previous bundled runtime...');
    fs.rmSync(targetDir, { recursive: true, force: true });
  }

  copyRecursive(sourceDir, targetDir);

  const exePath = path.join(targetDir, 'makerbot-print.exe');
  if (!fs.existsSync(exePath)) {
    throw new Error(`Bundled runtime missing makerbot-print.exe at: ${exePath}`);
  }

  console.log('[vendor-runtime] Done. Runtime bundled successfully.');
  console.log('[vendor-runtime] You can now run: npm run start:bundle-win');
}

try {
  main();
} catch (err) {
  console.error(err.message || err);
  process.exit(1);
}
